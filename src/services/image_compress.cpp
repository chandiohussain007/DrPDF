#include "image_compress.h"

#include "core/assembly.h"
#include "core/content_util.h"
#include "core/qpdf_io.h"

#include <qpdf/Buffer.hh>
#include <qpdf/Constants.h>
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>


#include <QBuffer>
#include <QImage>

#include <algorithm>
#include <cstring>


namespace drpdf {
namespace {

std::string filterName(QPDFObjectHandle dict) {
    auto f = dict.getKey("/Filter");
    if (f.isName()) {
        return f.getName();
    }
    if (f.isArray() && f.getArrayNItems() > 0 && f.getArrayItem(0).isName()) {
        return f.getArrayItem(0).getName();
    }
    return {};
}

QImage loadPdfImage(QPDFObjectHandle image) {
    auto dict = image.getDict();
    if (!dict.hasKey("/Width") || !dict.hasKey("/Height")) {
        return {};
    }
    const int w = static_cast<int>(dict.getKey("/Width").getIntValue());
    const int h = static_cast<int>(dict.getKey("/Height").getIntValue());
    int bpc = 8;
    if (dict.hasKey("/BitsPerComponent") && dict.getKey("/BitsPerComponent").isInteger()) {
        bpc = static_cast<int>(dict.getKey("/BitsPerComponent").getIntValue());
    }
    if (w <= 0 || h <= 0 || bpc != 8) {
        return {};
    }
    const std::string filter = filterName(dict);
    if (filter == "/DCTDecode" || filter == "/JPXDecode") {
        auto raw = image.getStreamData(qpdf_dl_none);

        if (!raw || raw->getSize() == 0) {
            return {};
        }
        QByteArray bytes(reinterpret_cast<char const*>(raw->getBuffer()),
                         static_cast<int>(raw->getSize()));
        QImage img;
        img.loadFromData(bytes);
        return img;
    }
    std::string cs = "/DeviceRGB";
    if (dict.hasKey("/ColorSpace") && dict.getKey("/ColorSpace").isName()) {
        cs = dict.getKey("/ColorSpace").getName();
    }
    auto buf = image.getStreamData();
    if (!buf || buf->getSize() == 0) {
        return {};
    }
    const unsigned char* p = buf->getBuffer();
    const size_t n = buf->getSize();
    if (cs == "/DeviceGray") {
        const size_t need = static_cast<size_t>(w) * static_cast<size_t>(h);
        if (n < need) {
            return {};
        }
        QImage img(w, h, QImage::Format_Grayscale8);
        for (int y = 0; y < h; ++y) {
            memcpy(img.scanLine(y), p + static_cast<size_t>(y) * w, static_cast<size_t>(w));
        }
        return img;
    }
    if (cs == "/DeviceRGB") {
        const size_t need = static_cast<size_t>(w) * static_cast<size_t>(h) * 3;
        if (n < need) {
            return {};
        }
        QImage img(w, h, QImage::Format_RGB888);
        for (int y = 0; y < h; ++y) {
            memcpy(img.scanLine(y), p + static_cast<size_t>(y) * w * 3, static_cast<size_t>(w) * 3);
        }
        return img;
    }
    return {};
}

bool replaceWithJpeg(QPDF& pdf, QPDFObjectHandle image, const QImage& src, int quality) {
    QImage rgb = src.convertToFormat(QImage::Format_RGB888);
    QByteArray encoded;
    QBuffer buffer(&encoded);
    buffer.open(QIODevice::WriteOnly);
    if (!rgb.save(&buffer, "JPEG", quality)) {
        return false;
    }
    std::string bytes(encoded.constData(), static_cast<size_t>(encoded.size()));
    image.replaceStreamData(bytes, QPDFObjectHandle::newName("/DCTDecode"),
                            QPDFObjectHandle::newNull());
    auto dict = image.getDict();
    dict.replaceKey("/Width", QPDFObjectHandle::newInteger(rgb.width()));
    dict.replaceKey("/Height", QPDFObjectHandle::newInteger(rgb.height()));
    dict.replaceKey("/BitsPerComponent", QPDFObjectHandle::newInteger(8));
    dict.replaceKey("/ColorSpace", QPDFObjectHandle::newName("/DeviceRGB"));
    dict.replaceKey("/Filter", QPDFObjectHandle::newName("/DCTDecode"));
    dict.removeKey("/DecodeParms");
    dict.removeKey("/SMask");
    dict.removeKey("/Mask");
    dict.removeKey("/ColorTransform");
    (void)pdf;
    return true;
}

} // namespace

core::Result<void> compressPdf(const QString& input, const QString& output, CompressTier tier) {
    try {
        if (tier == CompressTier::Lossless) {
            return core::optimizePdf(std::filesystem::path(input.toStdString()),
                                     std::filesystem::path(output.toStdString()));
        }
        const int maxEdge = (tier == CompressTier::Low) ? 900 : 1400;
        const int quality = (tier == CompressTier::Low) ? 42 : 70;
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        core::qpdfProcessFile(pdf, std::filesystem::path(input.toStdString()));
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        for (auto& page : pages) {
            auto res = core::pageResources(page);
            auto xo = res.getKey("/XObject");
            if (!xo.isDictionary()) {
                continue;
            }
            for (const auto& key : xo.getKeys()) {
                auto obj = xo.getKey(key);
                if (!obj.isStream()) {
                    continue;
                }
                auto dict = obj.getDict();
                if (!dict.hasKey("/Subtype") || !dict.getKey("/Subtype").isName() ||
                    dict.getKey("/Subtype").getName() != "/Image") {
                    continue;
                }
                QImage img = loadPdfImage(obj);
                if (img.isNull()) {
                    continue;
                }
                const int edge = std::max(img.width(), img.height());
                QImage outImg = img;
                if (edge > maxEdge) {
                    outImg = img.scaled(maxEdge, maxEdge, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                } else if (filterName(dict) == "/DCTDecode" && edge <= maxEdge) {
                    continue;
                }
                replaceWithJpeg(pdf, obj, outImg, quality);
            }
        }
        QPDFWriter writer(pdf);
        writer.setOutputFilename(output.toStdString().c_str());
        writer.setCompressStreams(true);
        writer.setRecompressFlate(true);
        writer.write();
        return {};
    } catch (const std::exception& e) {
        return core::Error{e.what()};
    }
}

} // namespace drpdf

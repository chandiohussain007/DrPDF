#include "overlay.h"

#include "content_util.h"
#include "qpdf_io.h"

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>

#include <algorithm>
#include <cmath>
#include <sstream>

namespace drpdf::core {
namespace {

void writePdf(QPDF& pdf, const std::filesystem::path& output) {
    QPDFWriter writer(pdf);
    writer.setOutputFilename(pathToUtf8(output).c_str());
    writer.setCompressStreams(true);
    writer.write();
}

int lastPageIndex(int pageTo, int count) {
    if (pageTo < 0 || pageTo >= count) {
        return count - 1;
    }
    return pageTo;
}

std::string expandPageFormat(const std::string& fmt, int n, int total) {
    std::string s = fmt;
    const std::string ns = std::to_string(n);
    const std::string ts = std::to_string(total);
    auto replaceAll = [](std::string& hay, const std::string& from, const std::string& to) {
        size_t pos = 0;
        while ((pos = hay.find(from, pos)) != std::string::npos) {
            hay.replace(pos, from.size(), to);
            pos += to.size();
        }
    };
    replaceAll(s, "{n}", ns);
    replaceAll(s, "{total}", ts);
    return s;
}

void placeText(std::ostringstream& c, const std::string& text, double x, double y, double size,
               double r, double g, double b) {
    c << std::fixed;
    c << r << ' ' << g << ' ' << b << " rg BT /DrSans " << size << " Tf 1 0 0 1 " << x << ' ' << y
      << " Tm " << pdfEscapeLiteral(text) << " Tj ET\n";
}

} // namespace

Result<void> applyWatermark(const std::filesystem::path& input, const std::filesystem::path& output,
                            const WatermarkOptions& options, const std::string& password) {
    try {
        if (options.text.empty()) {
            return Error{"watermark text is empty"};
        }
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        const int last = lastPageIndex(options.pageTo, static_cast<int>(pages.size()));
        const double rad = options.rotationDeg * 3.14159265358979323846 / 180.0;
        const double cs = std::cos(rad);
        const double sn = std::sin(rad);
        for (int i = std::max(0, options.pageFrom); i <= last; ++i) {
            auto& ph = pages[static_cast<size_t>(i)];
            ensureStandardFont(pdf, ph, "DrSans", "Helvetica-Bold");
            ensureExtGState(ph, "DrWM", std::clamp(options.opacity, 0.02, 1.0));
            const auto box = pageMediaBox(ph);
            std::ostringstream c;
            c << std::fixed;
            c << "/DrWM gs " << options.r << ' ' << options.g << ' ' << options.b << " rg\n";
            auto stampAt = [&](double cx, double cy) {
                c << "q " << cs << ' ' << sn << ' ' << -sn << ' ' << cs << ' ' << cx << ' ' << cy
                  << " cm BT /DrSans " << options.fontSize << " Tf 1 0 0 1 " << (-options.fontSize * 2.2)
                  << " 0 Tm " << pdfEscapeLiteral(options.text) << " Tj ET Q\n";
            };
            if (options.tiled) {
                for (double y = box.lly + 80; y < box.ury; y += 160) {
                    for (double x = box.llx + 80; x < box.urx; x += 220) {
                        stampAt(x, y);
                    }
                }
            } else {
                stampAt((box.llx + box.urx) / 2.0, (box.lly + box.ury) / 2.0);
            }
            appendPageContents(pdf, ph, c.str());
        }
        writePdf(pdf, output);
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<void> applyHeaderFooter(const std::filesystem::path& input,
                               const std::filesystem::path& output,
                               const HeaderFooterOptions& options, const std::string& password) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        const int count = static_cast<int>(pages.size());
        const int last = lastPageIndex(options.pageTo, count);
        for (int i = std::max(0, options.pageFrom); i <= last; ++i) {
            auto& ph = pages[static_cast<size_t>(i)];
            ensureStandardFont(pdf, ph, "DrSans", "Helvetica");
            const auto box = pageMediaBox(ph);
            const double m = options.margin;
            std::ostringstream c;
            if (!options.header.empty()) {
                placeText(c, options.header, box.llx + m, box.ury - m, options.fontSize, options.r,
                          options.g, options.b);
            }
            if (!options.footer.empty()) {
                placeText(c, options.footer, box.llx + m, box.lly + m * 0.45, options.fontSize,
                          options.r, options.g, options.b);
            }
            if (!options.pageNumberFormat.empty()) {
                const std::string label = expandPageFormat(options.pageNumberFormat, i + 1, count);
                double x = box.llx + m;
                double y = box.lly + m * 0.45;
                switch (options.pageNumberPos) {
                case PageMarkPos::TopLeft:
                    x = box.llx + m;
                    y = box.ury - m;
                    break;
                case PageMarkPos::TopCenter:
                    x = (box.llx + box.urx) / 2.0 - 40;
                    y = box.ury - m;
                    break;
                case PageMarkPos::TopRight:
                    x = box.urx - m - 80;
                    y = box.ury - m;
                    break;
                case PageMarkPos::BottomLeft:
                    x = box.llx + m;
                    y = box.lly + m * 0.45;
                    break;
                case PageMarkPos::BottomCenter:
                    x = (box.llx + box.urx) / 2.0 - 40;
                    y = box.lly + m * 0.45;
                    break;
                case PageMarkPos::BottomRight:
                    x = box.urx - m - 80;
                    y = box.lly + m * 0.45;
                    break;
                }
                placeText(c, label, x, y, options.fontSize, options.r, options.g, options.b);
            }
            appendPageContents(pdf, ph, c.str());
        }
        writePdf(pdf, output);
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<void> stampImage(const std::filesystem::path& input, const std::filesystem::path& output,
                        const ImageStamp& stamp, const std::string& password) {
    try {
        if (stamp.image.width <= 0 || stamp.image.height <= 0 || stamp.image.samples.empty()) {
            return Error{"invalid image"};
        }
        const int comp = stamp.image.components == 1 ? 1 : 3;
        const size_t expected =
            static_cast<size_t>(stamp.image.width) * static_cast<size_t>(stamp.image.height) *
            static_cast<size_t>(comp);
        if (stamp.image.samples.size() < expected) {
            return Error{"image buffer too small"};
        }
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        if (stamp.page < 0 || stamp.page >= static_cast<int>(pages.size())) {
            return Error{"page out of range"};
        }
        auto& ph = pages[static_cast<size_t>(stamp.page)];
        auto res = pageResources(ph);
        auto xos = res.getKey("/XObject");
        if (!xos.isDictionary()) {
            xos = QPDFObjectHandle::newDictionary();
            res.replaceKey("/XObject", xos);
        }
        const std::string name = uniqueResourceName(xos, "DrIm");
        std::string bytes(reinterpret_cast<char const*>(stamp.image.samples.data()), expected);
        auto image = QPDFObjectHandle::newStream(&pdf, bytes);
        auto dict = image.getDict();
        dict.replaceKey("/Type", QPDFObjectHandle::newName("/XObject"));
        dict.replaceKey("/Subtype", QPDFObjectHandle::newName("/Image"));
        dict.replaceKey("/Width", QPDFObjectHandle::newInteger(stamp.image.width));
        dict.replaceKey("/Height", QPDFObjectHandle::newInteger(stamp.image.height));
        dict.replaceKey("/BitsPerComponent", QPDFObjectHandle::newInteger(8));
        dict.replaceKey("/ColorSpace",
                        QPDFObjectHandle::newName(comp == 1 ? "/DeviceGray" : "/DeviceRGB"));
        xos.replaceKey(name, image);
        if (stamp.image.alpha.size() >=
            static_cast<size_t>(stamp.image.width) * static_cast<size_t>(stamp.image.height)) {
            std::string aBytes(reinterpret_cast<char const*>(stamp.image.alpha.data()),
                               static_cast<size_t>(stamp.image.width) *
                                   static_cast<size_t>(stamp.image.height));
            auto smask = QPDFObjectHandle::newStream(&pdf, aBytes);
            auto sd = smask.getDict();
            sd.replaceKey("/Type", QPDFObjectHandle::newName("/XObject"));
            sd.replaceKey("/Subtype", QPDFObjectHandle::newName("/Image"));
            sd.replaceKey("/Width", QPDFObjectHandle::newInteger(stamp.image.width));
            sd.replaceKey("/Height", QPDFObjectHandle::newInteger(stamp.image.height));
            sd.replaceKey("/ColorSpace", QPDFObjectHandle::newName("/DeviceGray"));
            sd.replaceKey("/BitsPerComponent", QPDFObjectHandle::newInteger(8));
            dict.replaceKey("/SMask", smask);
        }

        std::ostringstream c;

        c << std::fixed;
        c << "q " << stamp.width << " 0 0 " << stamp.height << ' ' << stamp.x << ' ' << stamp.y
          << " cm " << name << " Do Q\n";
        appendPageContents(pdf, ph, c.str());
        writePdf(pdf, output);
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

} // namespace drpdf::core

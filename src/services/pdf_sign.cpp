#include "pdf_sign.h"

#include "core/qpdf_io.h"

#include <qpdf/Constants.h>
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>

#include <QFile>
#include <QImage>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <memory>
#include <sstream>
#include <vector>


#ifdef DRPDF_HAS_OPENSSL
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pkcs12.h>
#include <openssl/pkcs7.h>
#if defined(OPENSSL_VERSION_MAJOR) && OPENSSL_VERSION_MAJOR >= 3
#include <openssl/provider.h>
#endif
#endif

namespace drpdf {
namespace {

core::RasterImage fromArgb(const QImage& src) {
    QImage img = src.convertToFormat(QImage::Format_ARGB32);
    core::RasterImage r;
    r.width = img.width();
    r.height = img.height();
    r.components = 3;
    r.samples.resize(static_cast<size_t>(r.width) * static_cast<size_t>(r.height) * 3);
    r.alpha.resize(static_cast<size_t>(r.width) * static_cast<size_t>(r.height));
    for (int y = 0; y < img.height(); ++y) {
        const auto* line = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            const size_t i = static_cast<size_t>(y) * img.width() + x;
            r.samples[i * 3 + 0] = static_cast<unsigned char>(qRed(line[x]));
            r.samples[i * 3 + 1] = static_cast<unsigned char>(qGreen(line[x]));
            r.samples[i * 3 + 2] = static_cast<unsigned char>(qBlue(line[x]));
            r.alpha[i] = static_cast<unsigned char>(qAlpha(line[x]));
        }
    }
    return r;
}

std::string readAll(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        return {};
    }
    const QByteArray b = f.readAll();
    return std::string(b.constData(), static_cast<size_t>(b.size()));
}

bool writeAll(const QString& path, const std::string& data) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return f.write(data.data(), qint64(data.size())) == qint64(data.size());
}

void patch10(std::string& data, size_t from, long long value) {
    const auto pos = data.find("9999999999", from);
    if (pos == std::string::npos) {
        return;
    }
    char buf[11];
    std::snprintf(buf, sizeof(buf), "%010lld", value);
    data.replace(pos, 10, buf, 10);
}

#ifdef DRPDF_HAS_OPENSSL

std::string opensslErr() {
    char buf[256];
    ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
    return buf;
}

struct P12Bits {
    EVP_PKEY* key = nullptr;
    X509* cert = nullptr;
    STACK_OF(X509)* ca = nullptr;
    ~P12Bits() {
        EVP_PKEY_free(key);
        X509_free(cert);
        sk_X509_pop_free(ca, X509_free);
    }
};

core::Result<P12Bits*> loadP12(const QString& path, const QString& password) {
    OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS, nullptr);
#if defined(OPENSSL_VERSION_MAJOR) && OPENSSL_VERSION_MAJOR >= 3
    OSSL_PROVIDER_load(nullptr, "default");
    OSSL_PROVIDER_load(nullptr, "legacy");
#endif
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        return core::Error{"could not open PKCS#12 file"};
    }
    const QByteArray blob = f.readAll();
    const unsigned char* p = reinterpret_cast<const unsigned char*>(blob.constData());
    PKCS12* p12 = d2i_PKCS12(nullptr, &p, static_cast<long>(blob.size()));
    if (!p12) {
        return core::Error{"invalid PKCS#12: " + opensslErr()};
    }
    auto* bits = new P12Bits();
    const int ok =
        PKCS12_parse(p12, password.toUtf8().constData(), &bits->key, &bits->cert, &bits->ca);
    PKCS12_free(p12);
    if (!ok || !bits->key || !bits->cert) {
        delete bits;
        return core::Error{"PKCS#12 parse failed (wrong password?): " + opensslErr()};
    }
    return bits;
}


core::Result<std::string> pkcs7Detached(P12Bits* bits, const std::string& payload) {
    BIO* bio = BIO_new_mem_buf(payload.data(), static_cast<int>(payload.size()));
    if (!bio) {
        return core::Error{"BIO alloc failed"};
    }
    PKCS7* p7 = PKCS7_sign(bits->cert, bits->key, bits->ca, bio,
                           PKCS7_DETACHED | PKCS7_BINARY);
    BIO_free(bio);
    if (!p7) {
        return core::Error{"PKCS#7 sign failed: " + opensslErr()};
    }
    unsigned char* der = nullptr;
    const int len = i2d_PKCS7(p7, &der);
    PKCS7_free(p7);
    if (len <= 0 || !der) {
        return core::Error{"could not encode PKCS#7"};
    }
    std::string out(reinterpret_cast<char*>(der), static_cast<size_t>(len));
    OPENSSL_free(der);
    return out;
}

#endif

} // namespace

core::RasterImage rasterFromImage(const QImage& image) { return fromArgb(image); }

bool opensslSigningAvailable() {
#ifdef DRPDF_HAS_OPENSSL
    return true;
#else
    return false;
#endif
}

core::Result<void> digitallySignPdf(const QString& input, const QString& output,
                                    const QString& p12Path, const QString& p12Password, int page,
                                    double llx, double lly, double urx, double ury) {
#ifndef DRPDF_HAS_OPENSSL
    (void)input;
    (void)output;
    (void)p12Path;
    (void)p12Password;
    (void)page;
    (void)llx;
    (void)lly;
    (void)urx;
    (void)ury;
    return core::Error{
        "Cryptographic signing is disabled in this build. Install OpenSSL dev libraries and rebuild. "
        "Visual signatures still work."};
#else
    try {
        auto loaded = loadP12(p12Path, p12Password);
        if (!loaded) {
            return core::Error{loaded.error()};
        }
        std::unique_ptr<P12Bits> bits(loaded.value());

        QPDF pdf;
        pdf.setSuppressWarnings(true);
        core::qpdfProcessFile(pdf, std::filesystem::path(input.toStdString()));
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        if (page < 0 || page >= static_cast<int>(pages.size())) {
            return core::Error{"page out of range"};
        }
        auto pageOh = pages[static_cast<size_t>(page)].getObjectHandle();

        std::string placeholder(8192, '\0');
        auto sig = pdf.makeIndirectObject(QPDFObjectHandle::newDictionary());
        sig.replaceKey("/Type", QPDFObjectHandle::newName("/Sig"));
        sig.replaceKey("/Filter", QPDFObjectHandle::newName("/Adobe.PPKLite"));
        sig.replaceKey("/SubFilter", QPDFObjectHandle::newName("/adbe.pkcs7.detached"));
        auto br = QPDFObjectHandle::newArray();
        br.appendItem(QPDFObjectHandle::newInteger(0));
        br.appendItem(QPDFObjectHandle::newInteger(9999999999LL));
        br.appendItem(QPDFObjectHandle::newInteger(9999999999LL));
        br.appendItem(QPDFObjectHandle::newInteger(9999999999LL));
        sig.replaceKey("/ByteRange", br);
        sig.replaceKey("/Contents", QPDFObjectHandle::newString(placeholder));
        sig.replaceKey("/M", QPDFObjectHandle::newString("D:20260101000000Z"));

        auto widget = pdf.makeIndirectObject(QPDFObjectHandle::newDictionary());
        widget.replaceKey("/Type", QPDFObjectHandle::newName("/Annot"));
        widget.replaceKey("/Subtype", QPDFObjectHandle::newName("/Widget"));
        widget.replaceKey("/FT", QPDFObjectHandle::newName("/Sig"));
        widget.replaceKey("/F", QPDFObjectHandle::newInteger(4));
        widget.replaceKey("/T", QPDFObjectHandle::newString("DrPdfSignature"));
        widget.replaceKey("/V", sig);
        widget.replaceKey("/P", pageOh);
        std::ostringstream rect;
        rect << std::fixed << "[ " << std::min(llx, urx) << ' ' << std::min(lly, ury) << ' '
             << std::max(llx, urx) << ' ' << std::max(lly, ury) << " ]";
        widget.replaceKey("/Rect", QPDFObjectHandle::parse(rect.str()));

        auto annots = pageOh.getKey("/Annots");
        if (!annots.isArray()) {
            annots = QPDFObjectHandle::newArray();
            pageOh.replaceKey("/Annots", annots);
        }
        annots.appendItem(widget);

        auto root = pdf.getRoot();
        QPDFObjectHandle form;
        if (root.hasKey("/AcroForm") && root.getKey("/AcroForm").isDictionary()) {
            form = root.getKey("/AcroForm");
        } else {
            form = QPDFObjectHandle::newDictionary();
            root.replaceKey("/AcroForm", form);
        }
        auto fields = form.getKey("/Fields");
        if (!fields.isArray()) {
            fields = QPDFObjectHandle::newArray();
            form.replaceKey("/Fields", fields);
        }
        fields.appendItem(widget);
        form.replaceKey("/SigFlags", QPDFObjectHandle::newInteger(3));

        QPDFWriter writer(pdf);
        writer.setOutputFilename(output.toStdString().c_str());
        writer.setObjectStreamMode(qpdf_o_disable);
        writer.setCompressStreams(false);
        writer.write();

        std::string data = readAll(output);
        if (data.empty()) {
            return core::Error{"signed PDF was empty after write"};
        }
        auto marker = data.find("/SubFilter /adbe.pkcs7.detached");
        if (marker == std::string::npos) {
            marker = data.find("/Adobe.PPKLite");
        }
        auto contentsKey = data.find("/Contents", marker == std::string::npos ? 0 : marker);
        if (contentsKey == std::string::npos) {
            return core::Error{"could not locate signature Contents"};
        }
        auto lt = data.find('<', contentsKey);
        auto gt = data.find('>', lt);
        if (lt == std::string::npos || gt == std::string::npos || gt <= lt + 1) {
            return core::Error{"signature Contents is not a hex string"};
        }
        const long long a = static_cast<long long>(lt);
        const long long b = static_cast<long long>(gt + 1);
        const long long rest = static_cast<long long>(data.size()) - b;
        auto brPos = data.find("/ByteRange", marker == std::string::npos ? 0 : marker);
        if (brPos == std::string::npos) {
            return core::Error{"could not locate ByteRange"};
        }
        patch10(data, brPos, a);
        patch10(data, brPos, b);
        patch10(data, brPos, rest);

        std::string payload;
        payload.append(data.data(), static_cast<size_t>(a));
        payload.append(data.data() + static_cast<size_t>(b), static_cast<size_t>(rest));
        auto der = pkcs7Detached(bits.get(), payload);
        if (!der) {
            return core::Error{der.error()};
        }
        const size_t innerUnused = gt - lt - 1;
        (void)innerUnused;
        size_t hexSlots = 0;

        for (size_t i = lt + 1; i < gt; ++i) {
            const unsigned char c = static_cast<unsigned char>(data[i]);
            if (std::isxdigit(c)) {
                ++hexSlots;
            }
        }
        if (der.value().size() * 2 > hexSlots) {
            return core::Error{"signature is larger than the placeholder; use a smaller certificate chain"};
        }
        std::string padded = der.value();
        padded.resize(hexSlots / 2, '\0');
        std::string hex;
        hex.resize(hexSlots);
        static const char* kHex = "0123456789ABCDEF";
        size_t hi = 0;
        for (unsigned char c : padded) {
            hex[hi++] = kHex[c >> 4];
            hex[hi++] = kHex[c & 0xF];
        }
        size_t h = 0;
        for (size_t i = lt + 1; i < gt; ++i) {
            if (std::isxdigit(static_cast<unsigned char>(data[i]))) {
                data[i] = hex[h++];
            }
        }
        if (!writeAll(output, data)) {
            return core::Error{"could not write signed PDF"};
        }
        return {};
    } catch (const std::exception& e) {
        return core::Error{e.what()};
    }
#endif
}

} // namespace drpdf

#include "content_util.h"

#include <qpdf/Buffer.hh>
#include <qpdf/QPDFObjectHandle.hh>


#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace drpdf::core {
namespace {

std::string streamBytes(QPDFObjectHandle stream) {
    if (!stream.isStream()) {
        return {};
    }
    auto buf = stream.getStreamData();
    if (!buf || buf->getSize() == 0) {
        return {};
    }
    return std::string(reinterpret_cast<char const*>(buf->getBuffer()), buf->getSize());
}

} // namespace

PageRect pageMediaBox(QPDFPageObjectHelper& page) {
    PageRect r;
    try {
        auto box = page.getAttribute("/MediaBox", true);
        if (box.isArray() && box.getArrayNItems() >= 4) {
            r.llx = box.getArrayItem(0).getNumericValue();
            r.lly = box.getArrayItem(1).getNumericValue();
            r.urx = box.getArrayItem(2).getNumericValue();
            r.ury = box.getArrayItem(3).getNumericValue();
        }
    } catch (...) {
    }
    return r;
}


QPDFObjectHandle pageResources(QPDFPageObjectHelper& page) {
    auto res = page.getAttribute("/Resources", true);
    if (!res.isDictionary()) {
        res = QPDFObjectHandle::newDictionary();
        page.getObjectHandle().replaceKey("/Resources", res);
    }
    return res;
}

void ensureStandardFont(QPDF& pdf, QPDFPageObjectHelper& page, const std::string& resName,
                        const std::string& baseFont) {
    (void)pdf;
    auto res = pageResources(page);
    auto fonts = res.getKey("/Font");
    if (!fonts.isDictionary()) {
        fonts = QPDFObjectHandle::newDictionary();
        res.replaceKey("/Font", fonts);
    }
    const std::string key = resName[0] == '/' ? resName : ("/" + resName);
    if (fonts.hasKey(key)) {
        return;
    }
    auto font = QPDFObjectHandle::parse("<< /Type /Font /Subtype /Type1 /BaseFont /" + baseFont +
                                        " /Encoding /WinAnsiEncoding >>");
    fonts.replaceKey(key, font);
}

void ensureExtGState(QPDFPageObjectHelper& page, const std::string& resName, double opacity) {
    auto res = pageResources(page);
    auto gs = res.getKey("/ExtGState");
    if (!gs.isDictionary()) {
        gs = QPDFObjectHandle::newDictionary();
        res.replaceKey("/ExtGState", gs);
    }
    const std::string key = resName[0] == '/' ? resName : ("/" + resName);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) << opacity;
    auto dict = QPDFObjectHandle::parse("<< /CA " + oss.str() + " /ca " + oss.str() + " >>");
    gs.replaceKey(key, dict);
}

void appendPageContents(QPDF& pdf, QPDFPageObjectHelper& page, const std::string& content) {
    auto stream = QPDFObjectHandle::newStream(&pdf, "q\n" + content + "\nQ\n");
    page.addPageContents(stream, false);
}

void prependPageContents(QPDF& pdf, QPDFPageObjectHelper& page, const std::string& content) {
    auto stream = QPDFObjectHandle::newStream(&pdf, "q\n" + content + "\nQ\n");
    page.addPageContents(stream, true);
}

std::string getPageContent(QPDFPageObjectHelper& page) {
    auto c = page.getObjectHandle().getKey("/Contents");
    if (c.isStream()) {
        return streamBytes(c);
    }
    if (c.isArray()) {
        std::string all;
        const int n = c.getArrayNItems();
        for (int i = 0; i < n; ++i) {
            auto item = c.getArrayItem(i);
            if (item.isStream()) {
                all += streamBytes(item);
                all.push_back('\n');
            }
        }
        return all;
    }

    return {};
}

void setPageContent(QPDF& pdf, QPDFPageObjectHelper& page, const std::string& content) {
    auto stream = QPDFObjectHandle::newStream(&pdf, content);
    page.getObjectHandle().replaceKey("/Contents", stream);
}

std::string utf8ToWinAnsi(const std::string& utf8) {
    std::string out;
    out.reserve(utf8.size());
    size_t i = 0;
    while (i < utf8.size()) {
        unsigned char c = static_cast<unsigned char>(utf8[i]);
        if (c < 0x80) {
            out.push_back(static_cast<char>(c));
            ++i;
            continue;
        }
        unsigned int cp = 0;
        int need = 0;
        if ((c & 0xE0) == 0xC0) {
            cp = c & 0x1F;
            need = 1;
        } else if ((c & 0xF0) == 0xE0) {
            cp = c & 0x0F;
            need = 2;
        } else if ((c & 0xF8) == 0xF0) {
            cp = c & 0x07;
            need = 3;
        } else {
            out.push_back('?');
            ++i;
            continue;
        }
        if (i + static_cast<size_t>(need) >= utf8.size()) {
            out.push_back('?');
            break;
        }
        bool ok = true;
        for (int n = 1; n <= need; ++n) {
            unsigned char cc = static_cast<unsigned char>(utf8[i + static_cast<size_t>(n)]);
            if ((cc & 0xC0) != 0x80) {
                ok = false;
                break;
            }
            cp = (cp << 6) | (cc & 0x3F);
        }
        i += static_cast<size_t>(need) + 1;
        if (!ok || cp > 255) {
            out.push_back('?');
        } else {
            out.push_back(static_cast<char>(cp));
        }
    }
    return out;
}

std::string winAnsiToUtf8(const std::string& bytes) {
    std::string out;
    out.reserve(bytes.size());
    for (unsigned char c : bytes) {
        if (c < 0x80) {
            out.push_back(static_cast<char>(c));
        } else {
            out.push_back(static_cast<char>(0xC0 | (c >> 6)));
            out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        }
    }
    return out;
}

std::string pdfEscapeLiteral(const std::string& utf8) {
    const std::string raw = utf8ToWinAnsi(utf8);
    std::string out;
    out.push_back('(');
    for (unsigned char c : raw) {
        if (c == '(' || c == ')' || c == '\\') {
            out.push_back('\\');
            out.push_back(static_cast<char>(c));
        } else if (c == '\n') {
            out += "\\n";
        } else if (c == '\r') {
            out += "\\r";
        } else if (c == '\t') {
            out += "\\t";
        } else {
            out.push_back(static_cast<char>(c));
        }
    }
    out.push_back(')');
    return out;
}

std::string pdfUnescapeLiteral(const std::string& raw) {
    std::string bytes;
    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '\\' && i + 1 < raw.size()) {
            char n = raw[i + 1];
            if (n == 'n') {
                bytes.push_back('\n');
                ++i;
            } else if (n == 'r') {
                bytes.push_back('\r');
                ++i;
            } else if (n == 't') {
                bytes.push_back('\t');
                ++i;
            } else if (n == 'b') {
                bytes.push_back('\b');
                ++i;
            } else if (n == 'f') {
                bytes.push_back('\f');
                ++i;
            } else if (n == '(' || n == ')' || n == '\\') {
                bytes.push_back(n);
                ++i;
            } else if (std::isdigit(static_cast<unsigned char>(n))) {
                int val = 0;
                int count = 0;
                while (count < 3 && i + 1 < raw.size() &&
                       std::isdigit(static_cast<unsigned char>(raw[i + 1]))) {
                    val = val * 8 + (raw[i + 1] - '0');
                    ++i;
                    ++count;
                }
                bytes.push_back(static_cast<char>(val));
            } else {
                bytes.push_back(n);
                ++i;
            }
        } else {
            bytes.push_back(raw[i]);
        }
    }
    return winAnsiToUtf8(bytes);
}

std::string uniqueResourceName(QPDFObjectHandle dict, const std::string& prefix) {
    if (!dict.isDictionary()) {
        return "/" + prefix + "1";
    }
    for (int i = 1; i < 10000; ++i) {
        const std::string key = "/" + prefix + std::to_string(i);
        if (!dict.hasKey(key)) {
            return key;
        }
    }
    return "/" + prefix + "X";
}

} // namespace drpdf::core

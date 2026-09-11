#include "ocr_text.h"

#include "content_util.h"
#include "qpdf_io.h"

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>

#include <map>
#include <sstream>
#include <algorithm>
#include <vector>


namespace drpdf::core {
namespace {

std::vector<std::string> splitTab(const std::string& line) {
    std::vector<std::string> cols;
    std::string cur;
    for (char c : line) {
        if (c == '\t') {
            cols.push_back(cur);
            cur.clear();
        } else if (c != '\r') {
            cur.push_back(c);
        }
    }
    cols.push_back(cur);
    return cols;
}

int toInt(const std::string& s, int fallback = 0) {
    try {
        return std::stoi(s);
    } catch (...) {
        return fallback;
    }
}

std::string utf8ToUtf16BeHex(const std::string& utf8) {
    std::string hex = "FEFF";
    auto append16 = [&](unsigned int cp) {
        static const char* d = "0123456789ABCDEF";
        hex.push_back(d[(cp >> 12) & 0xF]);
        hex.push_back(d[(cp >> 8) & 0xF]);
        hex.push_back(d[(cp >> 4) & 0xF]);
        hex.push_back(d[cp & 0xF]);
    };
    size_t i = 0;
    while (i < utf8.size()) {
        unsigned char c = static_cast<unsigned char>(utf8[i]);
        unsigned int cp = 0;
        int need = 0;
        if (c < 0x80) {
            cp = c;
            need = 0;
        } else if ((c & 0xE0) == 0xC0) {
            cp = c & 0x1F;
            need = 1;
        } else if ((c & 0xF0) == 0xE0) {
            cp = c & 0x0F;
            need = 2;
        } else if ((c & 0xF8) == 0xF0) {
            cp = c & 0x07;
            need = 3;
        } else {
            ++i;
            continue;
        }
        if (i + static_cast<size_t>(need) >= utf8.size()) {
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
        if (!ok) {
            continue;
        }
        if (cp > 0xFFFF) {
            cp -= 0x10000;
            append16(0xD800 + ((cp >> 10) & 0x3FF));
            append16(0xDC00 + (cp & 0x3FF));
        } else {
            append16(cp);
        }
    }
    return "<" + hex + ">";
}

} // namespace


std::vector<TsvWord> parseTesseractTsv(const std::string& tsv, int page, int minConf) {
    std::vector<TsvWord> out;
    std::istringstream in(tsv);
    std::string line;
    bool header = true;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        auto cols = splitTab(line);
        if (header) {
            header = false;
            if (!cols.empty() && cols[0] == "level") {
                continue;
            }
        }
        if (cols.size() < 12) {
            continue;
        }
        if (toInt(cols[0]) != 5) {
            continue;
        }
        TsvWord w;
        w.page = page;
        w.left = toInt(cols[6]);
        w.top = toInt(cols[7]);
        w.width = toInt(cols[8]);
        w.height = toInt(cols[9]);
        w.conf = toInt(cols[10], -1);
        w.text = cols[11];
        for (size_t i = 12; i < cols.size(); ++i) {
            w.text.push_back('\t');
            w.text += cols[i];
        }
        if (w.conf < minConf || w.text.empty() || w.width <= 0 || w.height <= 0) {
            continue;
        }
        out.push_back(std::move(w));
    }
    return out;
}

Result<void> overlaySearchableText(const std::filesystem::path& input,
                                   const std::filesystem::path& output,
                                   const std::vector<SearchableWord>& words,
                                   const std::string& password) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        std::map<int, std::ostringstream> perPage;
        for (const auto& w : words) {
            if (w.page < 0 || w.page >= static_cast<int>(pages.size()) || w.text.empty()) {
                continue;
            }
            const double size = std::max(4.0, std::min(w.height * 0.9, 48.0));
            auto& c = perPage[w.page];
            c << std::fixed;
            c << "/Span << /ActualText " << utf8ToUtf16BeHex(w.text) << " >> BDC\n";
            c << "3 Tr BT /DrSans " << size << " Tf 1 0 0 1 " << w.x << ' ' << w.y << " Tm "
              << pdfEscapeLiteral(w.text) << " Tj ET EMC\n";

        }
        for (auto& [page, stream] : perPage) {
            auto& ph = pages[static_cast<size_t>(page)];
            ensureStandardFont(pdf, ph, "DrSans", "Helvetica");
            appendPageContents(pdf, ph, stream.str());
        }
        QPDFWriter writer(pdf);
        writer.setOutputFilename(pathToUtf8(output).c_str());
        writer.setCompressStreams(true);
        writer.write();
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

} // namespace drpdf::core

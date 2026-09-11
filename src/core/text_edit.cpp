#include "text_edit.h"

#include "content_util.h"
#include "qpdf_io.h"

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>

#include <cctype>
#include <cmath>
#include <sstream>
#include <vector>

namespace drpdf::core {
namespace {

struct Token {
    enum Kind { Number, Name, String, Operator, LBrack, RBrack, Other } kind = Other;
    std::string text;
    double number = 0;
};

struct Matrix {
    double a = 1, b = 0, c = 0, d = 1, e = 0, f = 0;
};

Matrix multiply(const Matrix& m, const Matrix& n) {
    Matrix r;
    r.a = m.a * n.a + m.b * n.c;
    r.b = m.a * n.b + m.b * n.d;
    r.c = m.c * n.a + m.d * n.c;
    r.d = m.c * n.b + m.d * n.d;
    r.e = m.e * n.a + m.f * n.c + n.e;
    r.f = m.e * n.b + m.f * n.d + n.f;
    return r;
}

bool isOpChar(unsigned char c) {
    return std::isalpha(c) || c == '*' || c == '\'' || c == '"';
}

std::vector<Token> tokenize(const std::string& s) {
    std::vector<Token> out;
    const size_t n = s.size();
    size_t i = 0;
    auto skipWs = [&] {
        while (i < n && std::isspace(static_cast<unsigned char>(s[i]))) {
            ++i;
        }
    };
    while (i < n) {
        skipWs();
        if (i >= n) {
            break;
        }
        if (s[i] == '%') {
            while (i < n && s[i] != '\n' && s[i] != '\r') {
                ++i;
            }
            continue;
        }
        Token t;
        if (s[i] == '(') {
            t.kind = Token::String;
            ++i;
            std::string raw;
            int depth = 1;
            while (i < n && depth > 0) {
                if (s[i] == '\\' && i + 1 < n) {
                    raw.push_back('\\');
                    raw.push_back(s[i + 1]);
                    i += 2;
                    continue;
                }
                if (s[i] == '(') {
                    ++depth;
                } else if (s[i] == ')') {
                    --depth;
                    if (depth == 0) {
                        ++i;
                        break;
                    }
                }
                raw.push_back(s[i]);
                ++i;
            }
            t.text = pdfUnescapeLiteral(raw);
            out.push_back(std::move(t));
            continue;
        }
        if (s[i] == '<' && i + 1 < n && s[i + 1] != '<') {
            t.kind = Token::String;
            ++i;
            std::string hex;
            while (i < n && s[i] != '>') {
                if (!std::isspace(static_cast<unsigned char>(s[i]))) {
                    hex.push_back(s[i]);
                }
                ++i;
            }
            if (i < n && s[i] == '>') {
                ++i;
            }
            std::string bytes;
            if (hex.size() % 2 == 1) {
                hex.push_back('0');
            }
            for (size_t h = 0; h + 1 < hex.size(); h += 2) {
                int v = 0;
                std::istringstream is(hex.substr(h, 2));
                is >> std::hex >> v;
                bytes.push_back(static_cast<char>(v));
            }
            if (bytes.size() >= 2 && static_cast<unsigned char>(bytes[0]) == 0xFE &&
                static_cast<unsigned char>(bytes[1]) == 0xFF) {
                std::string utf8;
                for (size_t k = 2; k + 1 < bytes.size(); k += 2) {
                    unsigned int cp = (static_cast<unsigned char>(bytes[k]) << 8) |
                                      static_cast<unsigned char>(bytes[k + 1]);
                    if (cp < 0x80) {
                        utf8.push_back(static_cast<char>(cp));
                    } else if (cp < 0x800) {
                        utf8.push_back(static_cast<char>(0xC0 | (cp >> 6)));
                        utf8.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                    } else {
                        utf8.push_back(static_cast<char>(0xE0 | (cp >> 12)));
                        utf8.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                        utf8.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                    }
                }
                t.text = utf8;
            } else {
                t.text = winAnsiToUtf8(bytes);
            }
            out.push_back(std::move(t));
            continue;
        }
        if (s[i] == '[' ) {
            t.kind = Token::LBrack;
            t.text = "[";
            ++i;
            out.push_back(t);
            continue;
        }
        if (s[i] == ']') {
            t.kind = Token::RBrack;
            t.text = "]";
            ++i;
            out.push_back(t);
            continue;
        }
        if (s[i] == '/' ) {
            t.kind = Token::Name;
            size_t start = i;
            ++i;
            while (i < n && !std::isspace(static_cast<unsigned char>(s[i])) &&
                   s[i] != '[' && s[i] != ']' && s[i] != '(' && s[i] != '<' && s[i] != '/' &&
                   s[i] != '%') {
                ++i;
            }
            t.text = s.substr(start, i - start);
            out.push_back(t);
            continue;
        }
        if (s[i] == '-' || s[i] == '+' || s[i] == '.' || std::isdigit(static_cast<unsigned char>(s[i]))) {
            size_t start = i;
            if (s[i] == '-' || s[i] == '+') {
                ++i;
            }
            while (i < n && (std::isdigit(static_cast<unsigned char>(s[i])) || s[i] == '.' ||
                             s[i] == 'e' || s[i] == 'E' || s[i] == '-' || s[i] == '+')) {
                ++i;
            }
            t.kind = Token::Number;
            t.text = s.substr(start, i - start);
            try {
                t.number = std::stod(t.text);
            } catch (...) {
                t.number = 0;
            }
            out.push_back(t);
            continue;
        }
        if (i + 1 < n && s[i] == 'B' && s[i + 1] == 'I' &&
            (i + 2 >= n || !isOpChar(static_cast<unsigned char>(s[i + 2])))) {
            size_t start = i;
            auto pos = s.find("EI", i + 2);
            if (pos == std::string::npos) {
                i = n;
            } else {
                i = pos + 2;
            }
            t.kind = Token::Other;
            t.text = s.substr(start, i - start);
            out.push_back(t);
            continue;
        }
        if (isOpChar(static_cast<unsigned char>(s[i]))) {
            size_t start = i;
            while (i < n && isOpChar(static_cast<unsigned char>(s[i]))) {
                ++i;
            }
            t.kind = Token::Operator;
            t.text = s.substr(start, i - start);
            out.push_back(t);
            continue;
        }
        t.kind = Token::Other;
        t.text = s.substr(i, 1);
        ++i;
        out.push_back(t);
    }
    return out;
}

std::string serialize(const std::vector<Token>& tokens) {
    std::ostringstream o;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& t = tokens[i];
        if (t.kind == Token::String) {
            o << pdfEscapeLiteral(t.text);
        } else {
            o << t.text;
        }
        o << ' ';
        if (t.kind == Token::Operator) {
            o << '\n';
        }
    }
    return o.str();
}

struct RunLoc {
    TextRun run;
    size_t tokenIndex = 0;
};

std::vector<RunLoc> findRuns(const std::vector<Token>& tokens, int page) {
    std::vector<RunLoc> runs;
    bool inText = false;
    Matrix tm;
    Matrix lineMatrix;
    double leading = 0;
    std::string font = "/F1";
    double size = 12;
    int index = 0;

    auto emit = [&](const std::string& text, size_t tok) {
        if (text.empty()) {
            return;
        }
        RunLoc loc;
        loc.run.page = page;
        loc.run.index = index++;
        loc.run.text = text;
        loc.run.font = font;
        loc.run.size = size;
        loc.run.x = tm.e;
        loc.run.y = tm.f;
        loc.run.simpleEncoding = true;
        loc.tokenIndex = tok;
        runs.push_back(std::move(loc));
    };

    auto takeNums = [&](size_t i, int count) -> bool {
        if (i < static_cast<size_t>(count)) {
            return false;
        }
        for (int k = 1; k <= count; ++k) {
            if (tokens[i - static_cast<size_t>(k)].kind != Token::Number) {
                return false;
            }
        }
        return true;
    };

    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& t = tokens[i];
        if (t.kind != Token::Operator) {
            continue;
        }
        if (t.text == "BT") {
            inText = true;
            tm = Matrix{};
            lineMatrix = tm;
            continue;
        }
        if (t.text == "ET") {
            inText = false;
            continue;
        }
        if (!inText) {
            continue;
        }
        if (t.text == "Tm" && takeNums(i, 6)) {
            tm.a = tokens[i - 6].number;
            tm.b = tokens[i - 5].number;
            tm.c = tokens[i - 4].number;
            tm.d = tokens[i - 3].number;
            tm.e = tokens[i - 2].number;
            tm.f = tokens[i - 1].number;
            lineMatrix = tm;
        } else if ((t.text == "Td" || t.text == "TD") && takeNums(i, 2)) {
            Matrix tr;
            tr.e = tokens[i - 2].number;
            tr.f = tokens[i - 1].number;
            tm = multiply(lineMatrix, tr);
            lineMatrix = tm;
            if (t.text == "TD") {
                leading = -tokens[i - 1].number;
            }
        } else if (t.text == "T*") {
            Matrix tr;
            tr.f = -leading;
            tm = multiply(lineMatrix, tr);
            lineMatrix = tm;
        } else if (t.text == "Tf" && i >= 2 && tokens[i - 1].kind == Token::Number &&
                   tokens[i - 2].kind == Token::Name) {
            font = tokens[i - 2].text;
            size = tokens[i - 1].number;
        } else if (t.text == "Tj" && i >= 1 && tokens[i - 1].kind == Token::String) {
            emit(tokens[i - 1].text, i - 1);
        } else if (t.text == "'" && i >= 1 && tokens[i - 1].kind == Token::String) {
            Matrix tr;
            tr.f = -leading;
            tm = multiply(lineMatrix, tr);
            lineMatrix = tm;
            emit(tokens[i - 1].text, i - 1);
        } else if (t.text == "TJ" && i >= 1) {
            std::string acc;
            size_t firstStr = i;
            int depth = 0;
            size_t start = i;
            while (start > 0) {
                --start;
                if (tokens[start].kind == Token::RBrack) {
                    ++depth;
                } else if (tokens[start].kind == Token::LBrack) {
                    if (depth == 0) {
                        break;
                    }
                    --depth;
                }
            }
            for (size_t k = start; k < i; ++k) {
                if (tokens[k].kind == Token::String) {
                    if (acc.empty()) {
                        firstStr = k;
                    }
                    acc += tokens[k].text;
                }
            }
            emit(acc, firstStr);
        }
    }
    return runs;
}

void writePdf(QPDF& pdf, const std::filesystem::path& output) {
    QPDFWriter writer(pdf);
    writer.setOutputFilename(pathToUtf8(output).c_str());
    writer.setCompressStreams(true);
    writer.write();
}

} // namespace

Result<std::vector<TextRun>> extractTextRuns(const std::filesystem::path& file,
                                             const std::string& password) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, file, password);
        std::vector<TextRun> all;
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        for (int p = 0; p < static_cast<int>(pages.size()); ++p) {
            auto tokens = tokenize(getPageContent(pages[static_cast<size_t>(p)]));
            for (auto& loc : findRuns(tokens, p)) {
                all.push_back(std::move(loc.run));
            }
        }
        return all;
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<void> replaceTextRun(const std::filesystem::path& input, const std::filesystem::path& output,
                            int page, int runIndex, const std::string& newText,
                            const std::string& password) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        if (page < 0 || page >= static_cast<int>(pages.size())) {
            return Error{"page out of range"};
        }
        auto& ph = pages[static_cast<size_t>(page)];
        auto tokens = tokenize(getPageContent(ph));
        auto runs = findRuns(tokens, page);
        if (runIndex < 0 || runIndex >= static_cast<int>(runs.size())) {
            return Error{"text run out of range"};
        }
        auto& tok = tokens[runs[static_cast<size_t>(runIndex)].tokenIndex];
        if (tok.kind != Token::String) {
            return Error{"selected run is not a simple string (CID/TJ cluster)"};
        }
        tok.text = newText;
        setPageContent(pdf, ph, serialize(tokens));
        writePdf(pdf, output);
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<void> addTextBox(const std::filesystem::path& input, const std::filesystem::path& output,
                        const NewTextBox& box, const std::string& password) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        if (box.page < 0 || box.page >= static_cast<int>(pages.size())) {
            return Error{"page out of range"};
        }
        auto& ph = pages[static_cast<size_t>(box.page)];
        ensureStandardFont(pdf, ph, "DrSans", box.baseFont.empty() ? "Helvetica" : box.baseFont);
        std::ostringstream c;
        c << std::fixed;
        c << box.r << ' ' << box.g << ' ' << box.b << " rg\n";
        c << "BT /DrSans " << box.fontSize << " Tf 1 0 0 1 " << box.x << ' ' << box.y << " Tm "
          << pdfEscapeLiteral(box.text) << " Tj ET\n";
        appendPageContents(pdf, ph, c.str());
        writePdf(pdf, output);
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<void> redactBoxes(const std::filesystem::path& input, const std::filesystem::path& output,
                         const std::vector<RedactBox>& boxes, const std::string& password) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        for (const auto& box : boxes) {
            if (box.page < 0 || box.page >= static_cast<int>(pages.size())) {
                return Error{"page out of range"};
            }
            auto& ph = pages[static_cast<size_t>(box.page)];
            auto tokens = tokenize(getPageContent(ph));
            auto runs = findRuns(tokens, box.page);
            for (auto& loc : runs) {
                if (loc.run.x >= box.llx && loc.run.x <= box.urx && loc.run.y >= box.lly &&
                    loc.run.y <= box.ury) {
                    if (tokens[loc.tokenIndex].kind == Token::String) {
                        tokens[loc.tokenIndex].text.clear();
                    }
                }
            }
            setPageContent(pdf, ph, serialize(tokens));
            std::ostringstream c;
            c << std::fixed;
            c << "0 0 0 rg " << box.llx << ' ' << box.lly << ' ' << (box.urx - box.llx) << ' '
              << (box.ury - box.lly) << " re f\n";
            appendPageContents(pdf, ph, c.str());
        }
        writePdf(pdf, output);
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

} // namespace drpdf::core

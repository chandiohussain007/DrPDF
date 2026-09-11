#pragma once

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QPDFPageObjectHelper.hh>

#include <string>
#include <utility>

namespace drpdf::core {

struct PageRect {
    double llx = 0;
    double lly = 0;
    double urx = 612;
    double ury = 792;
    double width() const { return urx - llx; }
    double height() const { return ury - lly; }
};

PageRect pageMediaBox(QPDFPageObjectHelper& page);

QPDFObjectHandle pageResources(QPDFPageObjectHelper& page);

void ensureStandardFont(QPDF& pdf, QPDFPageObjectHelper& page, const std::string& resName,
                        const std::string& baseFont = "Helvetica");

void ensureExtGState(QPDFPageObjectHelper& page, const std::string& resName, double opacity);

void appendPageContents(QPDF& pdf, QPDFPageObjectHelper& page, const std::string& content);
void prependPageContents(QPDF& pdf, QPDFPageObjectHelper& page, const std::string& content);

std::string getPageContent(QPDFPageObjectHelper& page);
void setPageContent(QPDF& pdf, QPDFPageObjectHelper& page, const std::string& content);

std::string pdfEscapeLiteral(const std::string& utf8);
std::string pdfUnescapeLiteral(const std::string& raw);
std::string utf8ToWinAnsi(const std::string& utf8);
std::string winAnsiToUtf8(const std::string& bytes);

std::string uniqueResourceName(QPDFObjectHandle dict, const std::string& prefix);

} // namespace drpdf::core

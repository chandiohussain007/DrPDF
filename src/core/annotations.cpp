#include "annotations.h"

#include "content_util.h"
#include "qpdf_io.h"

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>

#include <algorithm>
#include <sstream>


namespace drpdf::core {
namespace {

void writePdf(QPDF& pdf, const std::filesystem::path& output) {
    QPDFWriter writer(pdf);
    writer.setOutputFilename(pathToUtf8(output).c_str());
    writer.setCompressStreams(true);
    writer.write();
}

QPDFObjectHandle annotsArray(QPDFPageObjectHelper& page) {
    auto oh = page.getObjectHandle();
    auto annots = oh.getKey("/Annots");
    if (!annots.isArray()) {
        annots = QPDFObjectHandle::newArray();
        oh.replaceKey("/Annots", annots);
    }
    return annots;
}

void appendAnnot(QPDF& pdf, QPDFPageObjectHelper& page, QPDFObjectHandle dict) {
    auto annot = pdf.makeIndirectObject(dict);
    annotsArray(page).appendItem(annot);
}

std::string pdfString(const std::string& utf8) { return pdfEscapeLiteral(utf8); }

AnnotKind kindFromSubtype(const std::string& sub) {
    if (sub == "/Highlight") {
        return AnnotKind::Highlight;
    }
    if (sub == "/Ink") {
        return AnnotKind::Ink;
    }
    if (sub == "/FreeText") {
        return AnnotKind::FreeText;
    }
    return AnnotKind::Comment;
}

} // namespace

Result<std::vector<Annotation>> listAnnotations(const std::filesystem::path& file,
                                                const std::string& password) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, file, password);
        std::vector<Annotation> out;
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        for (int p = 0; p < static_cast<int>(pages.size()); ++p) {
            auto annots = pages[static_cast<size_t>(p)].getObjectHandle().getKey("/Annots");
            if (!annots.isArray()) {
                continue;
            }
            for (int i = 0; i < annots.getArrayNItems(); ++i) {
                auto a = annots.getArrayItem(i);
                if (!a.isDictionary()) {
                    continue;
                }

                Annotation an;
                an.page = p;
                an.index = i;
                if (a.hasKey("/Subtype") && a.getKey("/Subtype").isName()) {
                    an.subtype = a.getKey("/Subtype").getName();
                    an.kind = kindFromSubtype(an.subtype);
                }
                if (a.hasKey("/Contents") && a.getKey("/Contents").isString()) {
                    an.contents = a.getKey("/Contents").getUTF8Value();
                }
                if (a.hasKey("/Rect") && a.getKey("/Rect").isArray() &&
                    a.getKey("/Rect").getArrayNItems() >= 4) {
                    auto r = a.getKey("/Rect");
                    an.llx = r.getArrayItem(0).getNumericValue();
                    an.lly = r.getArrayItem(1).getNumericValue();
                    an.urx = r.getArrayItem(2).getNumericValue();
                    an.ury = r.getArrayItem(3).getNumericValue();
                }
                out.push_back(std::move(an));
            }
        }
        return out;
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<void> addHighlight(const std::filesystem::path& input, const std::filesystem::path& output,
                          const HighlightSpec& spec, const std::string& password) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        if (spec.page < 0 || spec.page >= static_cast<int>(pages.size())) {
            return Error{"page out of range"};
        }
        const double llx = std::min(spec.llx, spec.urx);
        const double urx = std::max(spec.llx, spec.urx);
        const double lly = std::min(spec.lly, spec.ury);
        const double ury = std::max(spec.lly, spec.ury);
        std::ostringstream d;
        d << std::fixed;
        d << "<< /Type /Annot /Subtype /Highlight /F 4 /C [" << spec.r << ' ' << spec.g << ' '
          << spec.b << "] /CA 0.45 /Rect [" << llx << ' ' << lly << ' ' << urx << ' ' << ury
          << "] /QuadPoints [" << llx << ' ' << ury << ' ' << urx << ' ' << ury << ' ' << llx << ' '
          << lly << ' ' << urx << ' ' << lly << "] >>";
        appendAnnot(pdf, pages[static_cast<size_t>(spec.page)], QPDFObjectHandle::parse(d.str()));
        writePdf(pdf, output);
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<void> addComment(const std::filesystem::path& input, const std::filesystem::path& output,
                        const CommentSpec& spec, const std::string& password) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        if (spec.page < 0 || spec.page >= static_cast<int>(pages.size())) {
            return Error{"page out of range"};
        }
        std::ostringstream d;
        d << std::fixed;
        d << "<< /Type /Annot /Subtype /Text /Name /Comment /F 4 /C [1 0.82 0.2] /Rect ["
          << spec.x << ' ' << spec.y << ' ' << (spec.x + 20) << ' ' << (spec.y + 20) << "] /Contents "
          << pdfString(spec.text) << " >>";
        appendAnnot(pdf, pages[static_cast<size_t>(spec.page)], QPDFObjectHandle::parse(d.str()));
        writePdf(pdf, output);
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<void> addInk(const std::filesystem::path& input, const std::filesystem::path& output,
                    const InkSpec& spec, const std::string& password) {
    try {
        if (spec.points.size() < 2) {
            return Error{"stroke needs at least two points"};
        }
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        if (spec.page < 0 || spec.page >= static_cast<int>(pages.size())) {
            return Error{"page out of range"};
        }
        double minx = spec.points[0].first, maxx = spec.points[0].first;
        double miny = spec.points[0].second, maxy = spec.points[0].second;
        std::ostringstream ink;
        ink << std::fixed << "[ ";
        for (const auto& pt : spec.points) {
            ink << pt.first << ' ' << pt.second << ' ';
            minx = std::min(minx, pt.first);
            maxx = std::max(maxx, pt.first);
            miny = std::min(miny, pt.second);
            maxy = std::max(maxy, pt.second);
        }
        ink << "]";
        std::ostringstream d;
        d << std::fixed;
        d << "<< /Type /Annot /Subtype /Ink /F 4 /C [" << spec.r << ' ' << spec.g << ' ' << spec.b
          << "] /BS << /W " << spec.width << " /S /S >> /Rect [" << (minx - 4) << ' ' << (miny - 4)
          << ' ' << (maxx + 4) << ' ' << (maxy + 4) << "] /InkList [ " << ink.str() << " ] >>";
        appendAnnot(pdf, pages[static_cast<size_t>(spec.page)], QPDFObjectHandle::parse(d.str()));
        writePdf(pdf, output);
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<void> addFreeText(const std::filesystem::path& input, const std::filesystem::path& output,
                         const FreeTextSpec& spec, const std::string& password) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        if (spec.page < 0 || spec.page >= static_cast<int>(pages.size())) {
            return Error{"page out of range"};
        }
        const double llx = std::min(spec.llx, spec.urx);
        const double urx = std::max(spec.llx, spec.urx);
        const double lly = std::min(spec.lly, spec.ury);
        const double ury = std::max(spec.lly, spec.ury);
        std::ostringstream d;
        d << std::fixed;
        d << "<< /Type /Annot /Subtype /FreeText /F 4 /C [1 1 1] /DA (/Helv " << spec.fontSize
          << " Tf 0 0 0 rg) /Rect [" << llx << ' ' << lly << ' ' << urx << ' ' << ury
          << "] /Contents " << pdfString(spec.text) << " >>";
        appendAnnot(pdf, pages[static_cast<size_t>(spec.page)], QPDFObjectHandle::parse(d.str()));
        writePdf(pdf, output);
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

Result<void> removeAnnotation(const std::filesystem::path& input,
                              const std::filesystem::path& output, int page, int index,
                              const std::string& password) {
    try {
        QPDF pdf;
        pdf.setSuppressWarnings(true);
        qpdfProcessFile(pdf, input, password);
        auto pages = QPDFPageDocumentHelper(pdf).getAllPages();
        if (page < 0 || page >= static_cast<int>(pages.size())) {
            return Error{"page out of range"};
        }
        auto annots = pages[static_cast<size_t>(page)].getObjectHandle().getKey("/Annots");
        if (!annots.isArray() || index < 0 || index >= annots.getArrayNItems()) {
            return Error{"annotation out of range"};
        }
        auto next = QPDFObjectHandle::newArray();
        for (int i = 0; i < annots.getArrayNItems(); ++i) {
            if (i != index) {
                next.appendItem(annots.getArrayItem(i));
            }
        }
        pages[static_cast<size_t>(page)].getObjectHandle().replaceKey("/Annots", next);
        writePdf(pdf, output);
        return {};
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

} // namespace drpdf::core

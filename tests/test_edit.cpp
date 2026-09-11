#include "core/annotations.h"
#include "core/overlay.h"
#include "core/text_edit.h"

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFPageObjectHelper.hh>
#include <qpdf/QPDFWriter.hh>

#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

static int g_failed = 0;

#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";           \
            ++g_failed;                                                                            \
        }                                                                                          \
    } while (0)

static void writeHelloPdf(const fs::path& path) {
    QPDF pdf;
    pdf.emptyPDF();
    auto page = pdf.makeIndirectObject(QPDFObjectHandle::parse(
        "<< /Type /Page /MediaBox [0 0 612 792] /Resources << /Font << /F1 << /Type /Font "
        "/Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >> >> >> >>"));
    auto contents = QPDFObjectHandle::newStream(&pdf, "BT /F1 24 Tf 1 0 0 1 72 700 Tm (Hello World) Tj ET\n");
    page.replaceKey("/Contents", contents);
    QPDFPageDocumentHelper(pdf).addPage(QPDFPageObjectHelper(page), false);
    QPDFWriter writer(pdf, path.string().c_str());
    writer.write();
}

int run_edit_tests() {
    auto tmp = fs::temp_directory_path() / "drpdf-edit-tests";
    fs::create_directories(tmp);
    auto src = tmp / "hello.pdf";
    auto replaced = tmp / "replaced.pdf";
    auto boxed = tmp / "boxed.pdf";
    auto marked = tmp / "marked.pdf";
    auto numbered = tmp / "numbered.pdf";
    auto noted = tmp / "noted.pdf";

    writeHelloPdf(src);

    auto runs = drpdf::core::extractTextRuns(src);
    if (!runs) {
        std::cerr << "extract error: " << runs.error() << "\n";
    }
    CHECK(runs.ok());
    CHECK(runs.value().size() >= 1);
    if (runs.ok() && !runs.value().empty()) {
        CHECK(runs.value()[0].text.find("Hello") != std::string::npos);
    }

    auto rr = drpdf::core::replaceTextRun(src, replaced, 0, 0, "Hello Dr PDF");
    if (!rr) {
        std::cerr << "replace error: " << rr.error() << "\n";
    }
    CHECK(rr.ok());
    auto runs2 = drpdf::core::extractTextRuns(replaced);
    CHECK(runs2.ok());
    if (runs2.ok() && !runs2.value().empty()) {
        CHECK(runs2.value()[0].text.find("Dr PDF") != std::string::npos);
    }

    drpdf::core::NewTextBox box;
    box.page = 0;
    box.text = "Added line";
    box.x = 72;
    box.y = 640;
    box.fontSize = 14;
    CHECK(drpdf::core::addTextBox(src, boxed, box).ok());

    drpdf::core::WatermarkOptions wm;
    wm.text = "DRAFT";
    CHECK(drpdf::core::applyWatermark(src, marked, wm).ok());

    drpdf::core::HeaderFooterOptions hf;
    hf.pageNumberFormat = "Page {n} of {total}";
    CHECK(drpdf::core::applyHeaderFooter(src, numbered, hf).ok());

    drpdf::core::CommentSpec cmt;
    cmt.page = 0;
    cmt.x = 100;
    cmt.y = 500;
    cmt.text = "A note";
    CHECK(drpdf::core::addComment(src, noted, cmt).ok());
    auto annots = drpdf::core::listAnnotations(noted);
    CHECK(annots.ok());
    CHECK(annots.value().size() >= 1);

    return g_failed;
}

#include "core/assembly.h"
#include "core/ranges.h"

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFPageObjectHelper.hh>
#include <qpdf/QPDFWriter.hh>

#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;
using drpdf::core::Assembly;
using drpdf::core::inspectPdf;

static int g_failed = 0;

#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";           \
            ++g_failed;                                                                            \
        }                                                                                          \
    } while (0)

int run_range_tests();
int run_edit_tests();
int run_ocr_tests();



static void writeBlankPdf(const fs::path& path, int pages) {
    QPDF pdf;
    pdf.emptyPDF();
    QPDFPageDocumentHelper dh(pdf);
    for (int i = 0; i < pages; ++i) {
        auto pageDict = pdf.makeIndirectObject(QPDFObjectHandle::parse(
            "<< /Type /Page /MediaBox [0 0 612 792] /Resources << >> >>"));
        dh.addPage(QPDFPageObjectHelper(pageDict), false);
    }
    QPDFWriter writer(pdf, path.string().c_str());
    writer.write();
}

int main() {
    auto tmp = fs::temp_directory_path() / "drpdf-tests";
    fs::create_directories(tmp);
    auto a = tmp / "a.pdf";
    auto b = tmp / "b.pdf";
    auto out = tmp / "merged.pdf";
    auto splitOut = tmp / "split.pdf";

    writeBlankPdf(a, 2);
    writeBlankPdf(b, 3);

    auto ia = inspectPdf(a);
    CHECK(ia.ok());
    CHECK(ia.value().pageCount == 2);

    auto ib = inspectPdf(b);
    CHECK(ib.ok());
    CHECK(ib.value().pageCount == 3);

    Assembly assembly;
    CHECK(assembly.addDocument(a).ok());
    CHECK(assembly.addDocument(b).ok());
    CHECK(assembly.count() == 5);

    assembly.rotate(0, 90);
    assembly.remove(4);
    CHECK(assembly.count() == 4);

    auto wr = assembly.write(out);
    if (!wr) {
        std::cerr << "write error: " << wr.error() << "\n";
    }
    CHECK(wr.ok());

    auto im = inspectPdf(out);
    CHECK(im.ok());
    CHECK(im.value().pageCount == 4);

    Assembly extract;
    CHECK(extract.addDocument(out).ok());
    extract.setIncluded(0, false);
    extract.setIncluded(1, false);
    CHECK(extract.write(splitOut).ok());
    auto is = inspectPdf(splitOut);
    CHECK(is.ok());
    CHECK(is.value().pageCount == 2);

    g_failed += run_range_tests();
    g_failed += run_edit_tests();
    g_failed += run_ocr_tests();



    if (g_failed == 0) {
        std::cout << "all tests passed\n";
        return 0;
    }
    std::cerr << g_failed << " checks failed\n";
    return 1;
}

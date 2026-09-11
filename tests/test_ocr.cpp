#include "core/ocr_text.h"

#include <iostream>
#include <string>

static int g_ocr_failed = 0;

#define CHECK_OCR(cond)                                                                            \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";           \
            ++g_ocr_failed;                                                                        \
        }                                                                                          \
    } while (0)

int run_ocr_tests() {
    const std::string tsv =
        "level\tpage_num\tblock_num\tpar_num\tline_num\tword_num\tleft\ttop\twidth\theight\tconf\ttext\n"
        "1\t1\t0\t0\t0\t0\t0\t0\t100\t100\t-1\t\n"
        "5\t1\t1\t1\t1\t1\t10\t20\t40\t12\t95\tHello\n"
        "5\t1\t1\t1\t1\t2\t55\t20\t50\t12\t12\tlowconf\n"
        "5\t1\t1\t1\t1\t3\t10\t40\t60\t14\t88\tWorld\n";
    auto words = drpdf::core::parseTesseractTsv(tsv, 0, 35);
    CHECK_OCR(words.size() == 2);
    if (words.size() >= 2) {
        CHECK_OCR(words[0].text == "Hello");
        CHECK_OCR(words[0].left == 10);
        CHECK_OCR(words[1].text == "World");
        CHECK_OCR(words[1].page == 0);
    }
    return g_ocr_failed;
}

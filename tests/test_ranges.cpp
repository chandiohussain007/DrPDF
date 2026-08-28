#include "core/ranges.h"

#include <iostream>

using drpdf::core::parsePageRanges;

static int g_failed = 0;

#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";           \
            ++g_failed;                                                                            \
        }                                                                                          \
    } while (0)

int run_range_tests() {
    auto a = parsePageRanges("1-3,5", 10);
    CHECK(a.ok());
    CHECK(a.value().size() == 4);
    CHECK(a.value()[0] == 0 && a.value()[1] == 1 && a.value()[2] == 2 && a.value()[3] == 4);

    auto b = parsePageRanges("z", 7);
    CHECK(b.ok());
    CHECK(b.value().size() == 1 && b.value()[0] == 6);

    auto c = parsePageRanges("1-z", 3);
    CHECK(c.ok());
    CHECK(c.value().size() == 3);

    auto d = parsePageRanges("99", 3);
    CHECK(!d.ok());

    auto e = parsePageRanges("", 3);
    CHECK(!e.ok());

    return g_failed;
}

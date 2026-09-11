#pragma once

#include <qpdf/Constants.h>

// QPDF 12 renamed qpdf_r3_print_full → qpdf_r3p_full (and flipped numeric values).
#if defined(DRPDF_QPDF_R3P) || (defined(QPDF_MAJOR_VERSION) && QPDF_MAJOR_VERSION >= 12)
#define DRPDF_R3_PRINT_FULL qpdf_r3p_full
#else
#define DRPDF_R3_PRINT_FULL qpdf_r3_print_full
#endif

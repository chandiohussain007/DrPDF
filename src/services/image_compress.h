#pragma once

#include "core/result.h"

#include <QString>

namespace drpdf {

enum class CompressTier { Lossless, Medium, Low };

core::Result<void> compressPdf(const QString& input, const QString& output, CompressTier tier);

} // namespace drpdf

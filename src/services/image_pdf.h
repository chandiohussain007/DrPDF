#pragma once

#include "core/result.h"

#include <QPageSize>
#include <QStringList>

#include <functional>

namespace drpdf {

struct ImagePdfOptions {
    QPageSize::PageSizeId pageSize = QPageSize::A4;
    bool landscape = false;
    bool fitToImage = false;
    int marginMm = 8;
    int dpi = 150;
};

core::Result<void> writeImagesToPdf(const QStringList& imagePaths, const QString& outputPath,
                                    const ImagePdfOptions& options,
                                    std::function<void(int, int)> progress = {});

} // namespace drpdf

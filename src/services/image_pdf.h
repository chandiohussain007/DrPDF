#pragma once

#include "core/result.h"

#include <QPageSize>
#include <QStringList>
#include <QVector>

#include <functional>

namespace drpdf {

struct ImagePdfOptions {
    QPageSize::PageSizeId pageSize = QPageSize::A4;
    bool landscape = false;
    bool fitToImage = false;
    int marginMm = 8;
    int dpi = 150;
};

struct ImageInput {
    QString path;
    int width = 0;  // 0 = keep native
    int height = 0;
};

core::Result<void> writeImagesToPdf(const QVector<ImageInput>& images, const QString& outputPath,
                                    const ImagePdfOptions& options,
                                    std::function<void(int, int)> progress = {});

core::Result<void> writeImagesToPdf(const QStringList& imagePaths, const QString& outputPath,
                                    const ImagePdfOptions& options,
                                    std::function<void(int, int)> progress = {});

} // namespace drpdf

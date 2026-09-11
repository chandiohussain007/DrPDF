#pragma once

#include "core/ocr_text.h"
#include "core/result.h"

#include <QString>
#include <QStringList>

#include <functional>

namespace drpdf {

struct OcrOptions {
    QString language = QStringLiteral("eng");
    int dpi = 300;
    bool sidecarTxt = false;
};

QString findTesseract();
QStringList tesseractLanguages();

core::Result<void> ocrPdf(const QString& input, const QString& output, const OcrOptions& options,
                          const std::function<void(int page, int total)>& progress = {});

} // namespace drpdf

#pragma once

#include "core/overlay.h"
#include "core/result.h"

#include <QImage>
#include <QString>

namespace drpdf {

core::RasterImage rasterFromImage(const QImage& image);

bool opensslSigningAvailable();

core::Result<void> digitallySignPdf(const QString& input, const QString& output,
                                    const QString& p12Path, const QString& p12Password, int page,
                                    double llx, double lly, double urx, double ury);

} // namespace drpdf

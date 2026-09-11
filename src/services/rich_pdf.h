#pragma once

#include "core/overlay.h"
#include "core/result.h"

#include <QPageSize>
#include <QString>

class QTextDocument;

namespace drpdf {

struct RichPdfOptions {
    QPageSize::PageSizeId pageSize = QPageSize::A4;
    bool landscape = false;
    double marginMm = 18;
    QString title = QStringLiteral("Dr PDF");
    QString header;
    QString footer;
    QString pageNumberFormat; // empty = off; {n} {total}
    core::PageMarkPos pageNumberPos = core::PageMarkPos::BottomCenter;
};

core::Result<void> writeTextDocumentToPdf(QTextDocument* document, const QString& outputPath,
                                          const RichPdfOptions& options = {});

} // namespace drpdf

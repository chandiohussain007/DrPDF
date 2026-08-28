#pragma once

#include "core/result.h"

#include <QString>

class QTextDocument;

namespace drpdf {

core::Result<void> writeTextDocumentToPdf(QTextDocument* document, const QString& outputPath);

} // namespace drpdf

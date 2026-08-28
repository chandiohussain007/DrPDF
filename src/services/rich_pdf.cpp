#include "rich_pdf.h"

#include <QPdfWriter>
#include <QTextDocument>
#include <QPageSize>
#include <QPageLayout>

namespace drpdf {

core::Result<void> writeTextDocumentToPdf(QTextDocument* document, const QString& outputPath) {
    if (!document) {
        return core::Error{"no document"};
    }
    QPdfWriter writer(outputPath);
    writer.setTitle(QStringLiteral("Dr PDF"));
    writer.setCreator(QStringLiteral("Dr PDF"));
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(18, 18, 18, 18), QPageLayout::Millimeter);
    writer.setResolution(120);
    document->print(&writer);
    return {};
}

} // namespace drpdf

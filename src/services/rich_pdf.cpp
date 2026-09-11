#include "rich_pdf.h"

#include <QFile>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QTextDocument>
#include <QUuid>

#include <algorithm>


namespace drpdf {

core::Result<void> writeTextDocumentToPdf(QTextDocument* document, const QString& outputPath,
                                          const RichPdfOptions& options) {
    if (!document) {
        return core::Error{"no document"};
    }

    const bool stamps = !options.header.isEmpty() || !options.footer.isEmpty() ||
                        !options.pageNumberFormat.isEmpty();
    const QString bodyPath = stamps ? (outputPath + QStringLiteral(".body-") +
                                       QUuid::createUuid().toString(QUuid::WithoutBraces) +
                                       QStringLiteral(".pdf"))
                                    : outputPath;

    QPdfWriter writer(bodyPath);
    writer.setTitle(options.title.isEmpty() ? QStringLiteral("Dr PDF") : options.title);
    writer.setCreator(QStringLiteral("Dr PDF"));
    writer.setResolution(144);
    const QPageSize pageSize(options.pageSize);
    QPageLayout layout(pageSize,
                       options.landscape ? QPageLayout::Landscape : QPageLayout::Portrait,
                       QMarginsF(options.marginMm, options.marginMm, options.marginMm,
                                 options.marginMm),
                       QPageLayout::Millimeter);
    writer.setPageLayout(layout);
    document->print(&writer);

    if (!stamps) {
        return {};
    }

    core::HeaderFooterOptions hf;
    hf.header = options.header.toStdString();
    hf.footer = options.footer.toStdString();
    hf.pageNumberFormat = options.pageNumberFormat.toStdString();
    hf.pageNumberPos = options.pageNumberPos;
    hf.fontSize = 9;
    hf.margin = std::max(18.0, options.marginMm * 2.0);
    auto r = core::applyHeaderFooter(std::filesystem::path(bodyPath.toStdString()),
                                     std::filesystem::path(outputPath.toStdString()), hf);
    QFile::remove(bodyPath);
    return r;
}

} // namespace drpdf

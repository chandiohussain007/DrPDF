#include "image_pdf.h"

#include <QImageReader>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>

#include <functional>

namespace drpdf {

core::Result<void> writeImagesToPdf(const QVector<ImageInput>& images, const QString& outputPath,
                                    const ImagePdfOptions& options,
                                    std::function<void(int, int)> progress) {
    if (images.isEmpty()) {
        return core::Error{"no images selected"};
    }

    QPdfWriter writer(outputPath);
    writer.setTitle(QStringLiteral("Dr PDF"));
    writer.setCreator(QStringLiteral("Dr PDF"));
    writer.setResolution(options.dpi);

    QPageSize pageSize(options.pageSize);
    QPageLayout layout(pageSize,
                       options.landscape ? QPageLayout::Landscape : QPageLayout::Portrait,
                       QMarginsF(options.marginMm, options.marginMm, options.marginMm,
                                 options.marginMm),
                       QPageLayout::Millimeter);
    writer.setPageLayout(layout);

    QPainter painter;
    if (!painter.begin(&writer)) {
        return core::Error{"could not start PDF writer"};
    }

    for (int i = 0; i < images.size(); ++i) {
        if (progress) {
            progress(i, images.size());
        }
        QImageReader reader(images[i].path);
        reader.setAutoTransform(true);
        QImage img = reader.read();
        if (img.isNull()) {
            painter.end();
            return core::Error{"failed to read image: " + images[i].path.toStdString() + " (" +
                               reader.errorString().toStdString() + ")"};
        }
        if (images[i].width > 0 && images[i].height > 0 &&
            (images[i].width != img.width() || images[i].height != img.height())) {
            img = img.scaled(images[i].width, images[i].height, Qt::IgnoreAspectRatio,
                             Qt::SmoothTransformation);
        }
        if (options.fitToImage) {
            const QSizeF mm(img.width() * 25.4 / options.dpi, img.height() * 25.4 / options.dpi);
            QPageLayout fit(QPageSize(mm, QPageSize::Millimeter, QString(), QPageSize::ExactMatch),
                            QPageLayout::Portrait, QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter);
            writer.setPageLayout(fit);
        }
        if (i > 0) {
            writer.newPage();
        }
        const QRect page = painter.viewport();
        QImage scaled = img.scaled(page.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        const QPoint topLeft((page.width() - scaled.width()) / 2,
                             (page.height() - scaled.height()) / 2);
        painter.drawImage(topLeft, scaled);
    }
    painter.end();
    if (progress) {
        progress(images.size(), images.size());
    }
    return {};
}

core::Result<void> writeImagesToPdf(const QStringList& imagePaths, const QString& outputPath,
                                    const ImagePdfOptions& options,
                                    std::function<void(int, int)> progress) {
    QVector<ImageInput> items;
    items.reserve(imagePaths.size());
    for (const auto& p : imagePaths) {
        items.push_back(ImageInput{p, 0, 0});
    }
    return writeImagesToPdf(items, outputPath, options, std::move(progress));
}

} // namespace drpdf

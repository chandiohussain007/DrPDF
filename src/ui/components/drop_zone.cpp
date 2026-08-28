#include "drop_zone.h"

#include "ui/theme/theme.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace drpdf {
namespace {

bool isPdf(const QString& p) { return p.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive); }

bool isImage(const QString& p) {
    static const QStringList ext{".png", ".jpg", ".jpeg", ".bmp", ".tif", ".tiff", ".webp", ".heic",
                                 ".heif"};
    for (const auto& e : ext) {
        if (p.endsWith(e, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

} // namespace

DropZone::DropZone(QWidget* parent) : QWidget(parent) {
    setAcceptDrops(true);
    setMinimumHeight(168);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void DropZone::setTitle(const QString& title) {
    title_ = title;
    update();
}

void DropZone::setSubtitle(const QString& subtitle) {
    subtitle_ = subtitle;
    update();
}

QStringList DropZone::filter(const QStringList& in) const {
    QStringList out;
    for (const auto& p : in) {
        if ((acceptPdf_ && isPdf(p)) || (acceptImages_ && isImage(p))) {
            out << p;
        }
    }
    return out;
}

void DropZone::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        hover_ = true;
        update();
    }
}

void DropZone::dragLeaveEvent(QDragLeaveEvent*) {
    hover_ = false;
    update();
}

void DropZone::dropEvent(QDropEvent* event) {
    hover_ = false;
    update();
    QStringList paths;
    for (const auto& url : event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            paths << url.toLocalFile();
        }
    }
    paths = filter(paths);
    if (!paths.isEmpty()) {
        emit filesDropped(paths);
    }
}

void DropZone::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        return;
    }
    QString filterStr;
    if (acceptPdf_ && acceptImages_) {
        filterStr = QStringLiteral("Documents (*.pdf *.png *.jpg *.jpeg *.bmp *.tif *.tiff *.webp);;All files (*)");
    } else if (acceptImages_) {
        filterStr = QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.webp *.heic);;All files (*)");
    } else {
        filterStr = QStringLiteral("PDF files (*.pdf);;All files (*)");
    }
    const auto files = QFileDialog::getOpenFileNames(this, QStringLiteral("Open"), QString(), filterStr);
    const auto ok = filter(files);
    if (!ok.isEmpty()) {
        emit filesDropped(ok);
    }
}

void DropZone::paintEvent(QPaintEvent*) {
    const auto& t = Theme::instance().tokens();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(1, 1, -1, -1), 16, 16);
    p.fillPath(path, hover_ ? t.surfaceHover : t.surface);
    QPen pen(hover_ ? t.accent : t.border, hover_ ? 2 : 1, Qt::DashLine);
    p.setPen(pen);
    p.drawPath(path);
    p.setPen(t.text);
    QFont f = font();
    f.setPixelSize(16);
    f.setWeight(QFont::DemiBold);
    p.setFont(f);
    p.drawText(rect().adjusted(0, -12, 0, 0), Qt::AlignCenter, title_);
    p.setPen(t.muted);
    f.setPixelSize(13);
    f.setWeight(QFont::Normal);
    p.setFont(f);
    p.drawText(rect().adjusted(0, 22, 0, 0), Qt::AlignCenter, subtitle_);
}

} // namespace drpdf

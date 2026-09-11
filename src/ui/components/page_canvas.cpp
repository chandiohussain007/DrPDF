#include "page_canvas.h"

#include "ui/theme/theme.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace drpdf {

PageCanvas::PageCanvas(QWidget* parent) : QWidget(parent) {
    setMinimumSize(280, 360);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

void PageCanvas::setPage(const QImage& image, QSizeF pointSize) {
    image_ = image;
    if (pointSize.width() > 1 && pointSize.height() > 1) {
        points_ = pointSize;
    }
    stroke_.clear();
    dragging_ = false;
    update();
}

void PageCanvas::setMode(Mode mode) {
    mode_ = mode;
    setCursor(mode == Mode::None ? Qt::ArrowCursor : Qt::CrossCursor);
}

void PageCanvas::clearPreviewStroke() {
    stroke_.clear();
    dragging_ = false;
    update();
}

QRectF PageCanvas::imageRect() const {
    if (image_.isNull()) {
        return {};
    }
    const QSize avail = size().shrunkBy(QMargins(12, 12, 12, 12));
    QSize scaled = image_.size().scaled(avail, Qt::KeepAspectRatio);
    const int x = (width() - scaled.width()) / 2;
    const int y = (height() - scaled.height()) / 2;
    return QRectF(x, y, scaled.width(), scaled.height());
}

QPointF PageCanvas::toPdf(const QPointF& widget) const {
    const QRectF r = imageRect();
    if (r.isEmpty()) {
        return {};
    }
    const qreal nx = (widget.x() - r.left()) / r.width();
    const qreal ny = (widget.y() - r.top()) / r.height();
    return QPointF(nx * points_.width(), (1.0 - ny) * points_.height());
}

QPointF PageCanvas::toWidget(const QPointF& pdf) const {
    const QRectF r = imageRect();
    const qreal nx = pdf.x() / points_.width();
    const qreal ny = 1.0 - (pdf.y() / points_.height());
    return QPointF(r.left() + nx * r.width(), r.top() + ny * r.height());
}

void PageCanvas::paintEvent(QPaintEvent*) {
    const auto& t = Theme::instance().tokens();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), t.bg);
    const QRectF r = imageRect();
    if (image_.isNull()) {
        p.setPen(t.muted);
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("Drop a PDF to preview a page"));
        return;
    }
    p.setPen(QPen(t.border, 1));
    p.setBrush(t.surface);
    p.drawRoundedRect(r.adjusted(-4, -4, 4, 4), 8, 8);
    p.drawImage(r, image_);

    if (dragging_ && mode_ == Mode::Rect) {
        QRectF box = QRectF(start_, current_).normalized();
        QColor fill = t.accent;
        fill.setAlpha(60);
        p.setBrush(fill);
        p.setPen(QPen(t.accent, 1.5, Qt::DashLine));
        p.drawRect(box);
    }
    if ((dragging_ || !stroke_.isEmpty()) && mode_ == Mode::Ink) {
        QPainterPath path;
        bool first = true;
        for (const auto& pt : stroke_) {
            if (first) {
                path.moveTo(pt);
                first = false;
            } else {
                path.lineTo(pt);
            }
        }
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(t.accent, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawPath(path);
    }
}

void PageCanvas::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton || mode_ == Mode::None || image_.isNull()) {
        return;
    }
    start_ = event->position();
    current_ = start_;
    dragging_ = true;
    if (mode_ == Mode::Ink) {
        stroke_.clear();
        stroke_.push_back(start_);
    }
    if (mode_ == Mode::Place) {
        emit placed(toPdf(start_));
        dragging_ = false;
    }
    update();
}

void PageCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (!dragging_) {
        return;
    }
    current_ = event->position();
    if (mode_ == Mode::Ink) {
        stroke_.push_back(current_);
    }
    update();
}

void PageCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (!dragging_ || event->button() != Qt::LeftButton) {
        return;
    }
    dragging_ = false;
    current_ = event->position();
    if (mode_ == Mode::Rect) {
        const QPointF a = toPdf(start_);
        const QPointF b = toPdf(current_);
        emit rectDrawn(QRectF(a, b).normalized());
    } else if (mode_ == Mode::Ink) {
        QVector<QPointF> pdf;
        pdf.reserve(stroke_.size());
        for (const auto& pt : stroke_) {
            pdf.push_back(toPdf(pt));
        }
        emit strokeDrawn(pdf);
        stroke_.clear();
    }
    update();
}

} // namespace drpdf

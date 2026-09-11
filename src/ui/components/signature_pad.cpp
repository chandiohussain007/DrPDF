#include "signature_pad.h"

#include "ui/theme/theme.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>


namespace drpdf {

SignaturePad::SignaturePad(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(140);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void SignaturePad::clear() {
    strokes_.clear();
    current_.clear();
    drawing_ = false;
    update();
}

QImage SignaturePad::toImage() const {
    QImage img(std::max(400, width() * 2), std::max(200, height() * 2), QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(img.width() / double(std::max(1, width())), img.height() / double(std::max(1, height())));
    QPen pen(Qt::black, 2.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    auto drawStroke = [&](const QVector<QPointF>& s) {
        if (s.size() < 2) {
            return;
        }
        QPainterPath path;
        path.moveTo(s.first());
        for (int i = 1; i < s.size(); ++i) {
            path.lineTo(s[i]);
        }
        p.drawPath(path);
    };
    for (const auto& s : strokes_) {
        drawStroke(s);
    }
    drawStroke(current_);
    return img;
}

void SignaturePad::paintEvent(QPaintEvent*) {
    const auto& t = Theme::instance().tokens();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), t.surface);
    p.setPen(QPen(t.border, 1));
    p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 8, 8);
    p.setPen(t.muted);
    if (strokes_.isEmpty() && current_.isEmpty()) {
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("Draw your signature"));
    }
    QPen pen(t.text, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    auto drawStroke = [&](const QVector<QPointF>& s) {
        if (s.size() < 2) {
            return;
        }
        QPainterPath path;
        path.moveTo(s.first());
        for (int i = 1; i < s.size(); ++i) {
            path.lineTo(s[i]);
        }
        p.drawPath(path);
    };
    for (const auto& s : strokes_) {
        drawStroke(s);
    }
    drawStroke(current_);
}

void SignaturePad::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        return;
    }
    drawing_ = true;
    current_.clear();
    current_.push_back(event->position());
    update();
}

void SignaturePad::mouseMoveEvent(QMouseEvent* event) {
    if (!drawing_) {
        return;
    }
    current_.push_back(event->position());
    update();
}

void SignaturePad::mouseReleaseEvent(QMouseEvent* event) {
    if (!drawing_ || event->button() != Qt::LeftButton) {
        return;
    }
    drawing_ = false;
    if (current_.size() >= 2) {
        strokes_.push_back(current_);
    }
    current_.clear();
    update();
}

} // namespace drpdf

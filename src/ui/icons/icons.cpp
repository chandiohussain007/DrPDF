#include "icons.h"

#include <QPainter>
#include <QPainterPath>

namespace drpdf {
namespace {

void strokePen(QPainter& p, const QColor& color, int size) {
    QPen pen(color, std::max(1.6, size / 14.0));
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
}

void drawDoc(QPainter& p, const QRectF& r) {
    QPainterPath path;
    const qreal fold = r.width() * 0.28;
    path.moveTo(r.left(), r.top());
    path.lineTo(r.right() - fold, r.top());
    path.lineTo(r.right(), r.top() + fold);
    path.lineTo(r.right(), r.bottom());
    path.lineTo(r.left(), r.bottom());
    path.closeSubpath();
    p.drawPath(path);
    p.drawLine(QPointF(r.right() - fold, r.top()),
               QPointF(r.right() - fold, r.top() + fold));
    p.drawLine(QPointF(r.right() - fold, r.top() + fold), QPointF(r.right(), r.top() + fold));
}

void paintIcon(QPainter& p, const QString& name, const QColor& color, int size) {
    p.setRenderHint(QPainter::Antialiasing, true);
    strokePen(p, color, size);
    const QRectF r(size * 0.18, size * 0.16, size * 0.64, size * 0.68);

    if (name == "app") {
        QPainterPath bg;
        bg.addRoundedRect(QRectF(1, 1, size - 2, size - 2), size * 0.22, size * 0.22);
        p.fillPath(bg, color);
        QPen pen(QColor("#042F2E"), std::max(1.4, size / 16.0));
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        const QRectF d(size * 0.28, size * 0.22, size * 0.40, size * 0.50);
        drawDoc(p, d);
        p.drawLine(QPointF(size * 0.36, size * 0.48), QPointF(size * 0.60, size * 0.48));
        p.drawLine(QPointF(size * 0.36, size * 0.58), QPointF(size * 0.54, size * 0.58));
        return;
    }
    if (name == "merge") {
        drawDoc(p, r.adjusted(size * 0.08, 0, 0, 0));
        drawDoc(p, r.adjusted(-size * 0.08, size * 0.08, -size * 0.08, size * 0.08));
        return;
    }
    if (name == "split") {
        drawDoc(p, r.adjusted(0, 0, -size * 0.18, 0));
        p.drawLine(QPointF(r.center().x() + size * 0.08, r.top()),
                   QPointF(r.center().x() + size * 0.08, r.bottom()));
        p.drawLine(QPointF(r.center().x() + size * 0.02, r.center().y() - 4),
                   QPointF(r.center().x() + size * 0.18, r.center().y()));
        p.drawLine(QPointF(r.center().x() + size * 0.02, r.center().y() + 4),
                   QPointF(r.center().x() + size * 0.18, r.center().y()));
        return;
    }
    if (name == "compress") {
        drawDoc(p, r);
        p.drawLine(QPointF(r.center().x(), r.top() + 6), QPointF(r.center().x(), r.bottom() - 6));
        p.drawLine(QPointF(r.center().x() - 5, r.top() + 12), QPointF(r.center().x(), r.top() + 6));
        p.drawLine(QPointF(r.center().x() + 5, r.top() + 12), QPointF(r.center().x(), r.top() + 6));
        p.drawLine(QPointF(r.center().x() - 5, r.bottom() - 12),
                   QPointF(r.center().x(), r.bottom() - 6));
        p.drawLine(QPointF(r.center().x() + 5, r.bottom() - 12),
                   QPointF(r.center().x(), r.bottom() - 6));
        return;
    }
    if (name == "sign") {
        drawDoc(p, r);
        QPainterPath sig;
        sig.moveTo(r.left() + 6, r.bottom() - 10);
        sig.cubicTo(r.left() + 10, r.bottom() - 22, r.center().x(), r.bottom() - 4,
                    r.right() - 8, r.bottom() - 14);
        p.drawPath(sig);
        return;
    }
    if (name == "ocr") {
        drawDoc(p, r);
        p.drawText(r, Qt::AlignCenter, QStringLiteral("A"));
        return;
    }
    if (name == "edit") {
        drawDoc(p, r);
        p.drawLine(QPointF(r.left() + 8, r.bottom() - 8), QPointF(r.right() - 6, r.top() + 10));
        return;
    }
    if (name == "create") {
        drawDoc(p, r);
        const QPointF c = r.center();
        p.drawLine(QPointF(c.x() - 6, c.y()), QPointF(c.x() + 6, c.y()));
        p.drawLine(QPointF(c.x(), c.y() - 6), QPointF(c.x(), c.y() + 6));
        return;
    }
    if (name == "images") {
        p.drawRoundedRect(r, 4, 4);
        p.drawLine(QPointF(r.left() + 4, r.bottom() - 8), QPointF(r.center().x(), r.center().y()));
        p.drawLine(QPointF(r.center().x(), r.center().y()),
                   QPointF(r.right() - 4, r.bottom() - 6));
        p.setBrush(color);
        p.drawEllipse(QPointF(r.left() + r.width() * 0.32, r.top() + r.height() * 0.32), 2.4, 2.4);
        p.setBrush(Qt::NoBrush);
        return;
    }
    if (name == "organize") {
        p.drawRoundedRect(QRectF(r.left(), r.top(), r.width() * 0.42, r.height() * 0.42), 3, 3);
        p.drawRoundedRect(
            QRectF(r.right() - r.width() * 0.42, r.top(), r.width() * 0.42, r.height() * 0.42), 3,
            3);
        p.drawRoundedRect(
            QRectF(r.left(), r.bottom() - r.height() * 0.42, r.width() * 0.42, r.height() * 0.42),
            3, 3);
        p.drawRoundedRect(QRectF(r.right() - r.width() * 0.42, r.bottom() - r.height() * 0.42,
                                 r.width() * 0.42, r.height() * 0.42),
                          3, 3);
        return;
    }
    if (name == "protect") {
        QPainterPath path;
        path.moveTo(r.center().x(), r.top());
        path.lineTo(r.right(), r.top() + r.height() * 0.22);
        path.lineTo(r.right(), r.top() + r.height() * 0.52);
        path.cubicTo(r.right(), r.bottom(), r.left(), r.bottom(), r.left(),
                     r.top() + r.height() * 0.52);
        path.lineTo(r.left(), r.top() + r.height() * 0.22);
        path.closeSubpath();
        p.drawPath(path);
        return;
    }
    if (name == "viewer") {
        drawDoc(p, r);
        p.drawEllipse(r.center(), r.width() * 0.18, r.height() * 0.14);
        p.drawPoint(r.center());
        return;
    }
    if (name == "watermark") {
        drawDoc(p, r);
        p.drawLine(QPointF(r.left() + 6, r.bottom() - 8), QPointF(r.right() - 6, r.top() + 10));
        return;
    }
    if (name == "annotate") {
        p.drawRoundedRect(r, 4, 4);
        p.drawLine(QPointF(r.left() + 6, r.top() + 10), QPointF(r.right() - 6, r.top() + 10));
        p.drawLine(QPointF(r.left() + 6, r.top() + 16), QPointF(r.right() - 10, r.top() + 16));
        return;
    }
    if (name == "back") {
        p.drawLine(QPointF(size * 0.58, size * 0.28), QPointF(size * 0.36, size * 0.50));
        p.drawLine(QPointF(size * 0.36, size * 0.50), QPointF(size * 0.58, size * 0.72));
        return;
    }
    if (name == "sun") {
        p.drawEllipse(r.center(), size * 0.12, size * 0.12);
        for (int i = 0; i < 8; ++i) {
            QTransform xf;
            xf.translate(r.center().x(), r.center().y());
            xf.rotate(i * 45);
            p.save();
            p.setTransform(xf, true);
            p.drawLine(QPointF(0, -size * 0.22), QPointF(0, -size * 0.30));
            p.restore();
        }
        return;
    }
    if (name == "moon") {
        QPainterPath path;
        path.addEllipse(r);
        QPainterPath cut;
        cut.addEllipse(r.adjusted(size * 0.16, -size * 0.04, size * 0.16, -size * 0.04));
        p.drawPath(path.subtracted(cut));
        return;
    }
    if (name == "open") {
        p.drawLine(QPointF(r.left(), r.bottom()), QPointF(r.left(), r.top() + 8));
        p.drawLine(QPointF(r.left(), r.top() + 8), QPointF(r.left() + 8, r.top() + 8));
        p.drawLine(QPointF(r.left() + 8, r.top() + 8), QPointF(r.left() + 12, r.top()));
        p.drawLine(QPointF(r.left() + 12, r.top()), QPointF(r.right(), r.top()));
        p.drawLine(QPointF(r.right(), r.top()), QPointF(r.right(), r.bottom()));
        p.drawLine(QPointF(r.right(), r.bottom()), QPointF(r.left(), r.bottom()));
        return;
    }
    if (name == "search") {
        p.drawEllipse(QPointF(r.center().x() - 2, r.center().y() - 2), size * 0.16, size * 0.16);
        p.drawLine(QPointF(r.center().x() + 6, r.center().y() + 6),
                   QPointF(r.right() - 2, r.bottom() - 2));
        return;
    }
    drawDoc(p, r);
}

} // namespace

QPixmap Icons::pixmap(const QString& name, const QColor& color, int size) {
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    paintIcon(p, name, color, size);
    return pm;
}

QIcon Icons::named(const QString& name, const QColor& color, int size) {
    return QIcon(pixmap(name, color, size));
}

QIcon Icons::app(int size) {
    return named(QStringLiteral("app"), QColor("#12B5A0"), size);
}

} // namespace drpdf

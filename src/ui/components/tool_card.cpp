#include "tool_card.h"

#include "ui/icons/icons.h"
#include "ui/theme/theme.h"

#include <QEnterEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>



namespace drpdf {

ToolCard::ToolCard(Tool tool, const QString& iconName, const QString& title, const QString& subtitle,
                   bool ready, QWidget* parent)
    : QWidget(parent), tool_(tool), iconName_(iconName), title_(title), subtitle_(subtitle),
      ready_(ready) {
    setCursor(Qt::PointingHandCursor);
    setMinimumSize(220, 132);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAccessibleName(title);
    setAccessibleDescription(subtitle);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_Hover, true);
}


void ToolCard::enterEvent(QEnterEvent*) {
    hover_ = true;
    update();
}

void ToolCard::leaveEvent(QEvent*) {
    hover_ = false;
    update();
}

void ToolCard::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Space) {
        if (ready_) {
            emit activated(tool_);
        }
        return;
    }
    QWidget::keyPressEvent(event);
}


void ToolCard::paintEvent(QPaintEvent*) {
    const auto& t = Theme::instance().tokens();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(1, 1, -1, -1), 12, 12);

    p.fillPath(path, hover_ ? t.surfaceHover : t.surface);
    p.setPen(QPen(hover_ ? t.accent : t.border, hover_ ? 1.6 : 1));
    p.drawPath(path);

    const QPixmap ic = Icons::pixmap(iconName_, t.accent, 36);
    p.drawPixmap(20, 22, ic);

    if (!ready_) {
        QFont badge = font();
        badge.setPixelSize(10);
        badge.setWeight(QFont::DemiBold);
        p.setFont(badge);
        const QString label = QStringLiteral("COMING SOON");
        const QRect br = QFontMetrics(badge).boundingRect(label).adjusted(-8, -3, 8, 3);
        QRect box(width() - br.width() - 16, 16, br.width(), br.height());
        QPainterPath bpath;
        bpath.addRoundedRect(box, 8, 8);
        p.fillPath(bpath, t.surface2);
        p.setPen(t.muted);
        p.drawText(box, Qt::AlignCenter, label);
    }

    QFont title = font();
    title.setPixelSize(16);
    title.setWeight(QFont::DemiBold);
    p.setFont(title);
    p.setPen(t.text);
    p.drawText(QRect(20, 68, width() - 36, 24), Qt::AlignLeft | Qt::AlignVCenter, title_);

    QFont sub = font();
    sub.setPixelSize(12);
    p.setFont(sub);
    p.setPen(t.muted);
    p.drawText(QRect(20, 92, width() - 36, 24), Qt::AlignLeft | Qt::AlignVCenter, subtitle_);
}

void ToolCard::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (ready_) {
            emit activated(tool_);
        }
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

} // namespace drpdf


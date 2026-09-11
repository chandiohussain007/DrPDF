#include "icon_rail.h"

#include "ui/icons/icons.h"
#include "ui/theme/theme.h"

#include <QToolButton>
#include <QVBoxLayout>

namespace drpdf {

IconRail::IconRail(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("IconRail"));
    setFixedWidth(48);

    lay_ = new QVBoxLayout(this);
    lay_->setContentsMargins(6, 12, 6, 12);
    lay_->setSpacing(4);

    addBtn(QStringLiteral("organize"), QStringLiteral("Page Thumbnails"), 0);
    addBtn(QStringLiteral("viewer"), QStringLiteral("Document Outline"), 1);
    addBtn(QStringLiteral("annotate"), QStringLiteral("Annotations"), 2);
    addBtn(QStringLiteral("images"), QStringLiteral("Attachments"), 3);
    addBtn(QStringLiteral("protect"), QStringLiteral("Digital Signatures"), 4);

    lay_->addStretch();

    auto* settings = addBtn(QStringLiteral("settings"), QStringLiteral("App Settings"), 5);
    settings->setProperty("rail-settings", true);

    setCurrent(0);
}

QToolButton* IconRail::addBtn(const QString& iconName, const QString& tip, int index) {
    auto* btn = new QToolButton(this);
    btn->setIcon(Icons::named(iconName, Theme::instance().tokens().muted, 20));
    btn->setToolTip(tip);
    btn->setAccessibleName(tip);
    btn->setObjectName(QStringLiteral("IconBtn"));
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setProperty("railIndex", index);
    connect(btn, &QToolButton::clicked, this, [this, index]() {
        setCurrent(index);
        emit panelChosen(index);
    });
    lay_->addWidget(btn);
    buttons_.append(btn);
    return btn;
}

void IconRail::setCurrent(int index) {
    for (auto* btn : buttons_) {
        btn->setChecked(btn->property("railIndex").toInt() == index);
    }
}

} // namespace drpdf
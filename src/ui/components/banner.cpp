#include "banner.h"

#include "ui/theme/theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>

namespace drpdf {

Banner::Banner(QWidget* parent) : QWidget(parent) {
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(14, 8, 14, 8);
    lay->setSpacing(12);
    label_ = new QLabel(this);
    bar_ = new QProgressBar(this);
    bar_->setFixedWidth(180);
    bar_->setTextVisible(false);
    bar_->setFixedHeight(8);
    lay->addWidget(label_, 1);
    lay->addWidget(bar_);
    hide();
}

void Banner::showInfo(const QString& text) {
    const auto& t = Theme::instance().tokens();
    setStyleSheet(QStringLiteral("background:%1; border-radius:10px;").arg(t.surface2.name()));
    label_->setText(text);
    label_->setStyleSheet(QStringLiteral("color:%1;").arg(t.text.name()));
    bar_->hide();
    show();
}

void Banner::showError(const QString& text) {
    const auto& t = Theme::instance().tokens();
    setStyleSheet(QStringLiteral("background:%1; border-radius:10px;")
                      .arg(t.danger.darker(180).name()));
    label_->setText(text);
    label_->setStyleSheet(QStringLiteral("color:%1;").arg(t.danger.name()));
    bar_->hide();
    show();
}

void Banner::showProgress(const QString& text, int current, int total) {
    const auto& t = Theme::instance().tokens();
    setStyleSheet(QStringLiteral("background:%1; border-radius:10px;").arg(t.surface2.name()));
    label_->setText(text);
    label_->setStyleSheet(QStringLiteral("color:%1;").arg(t.text.name()));
    bar_->setRange(0, std::max(1, total));
    bar_->setValue(current);
    bar_->show();
    show();
}

void Banner::hideBanner() { hide(); }

} // namespace drpdf

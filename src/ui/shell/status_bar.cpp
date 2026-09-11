#include "status_bar.h"

#include "ui/icons/icons.h"
#include "ui/theme/theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QToolButton>

namespace drpdf {

StatusBar::StatusBar(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("StatusBar"));
    setFixedHeight(28);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(12, 2, 12, 2);
    lay->setSpacing(6);

    auto* firstBtn = new QToolButton(this);
    firstBtn->setIcon(Icons::named(QStringLiteral("first"), Theme::instance().tokens().muted, 14));
    firstBtn->setToolTip(QStringLiteral("First page"));
    firstBtn->setObjectName(QStringLiteral("IconBtn"));
    connect(firstBtn, &QToolButton::clicked, this, &StatusBar::firstPage);

    auto* prevBtn = new QToolButton(this);
    prevBtn->setIcon(Icons::named(QStringLiteral("prev"), Theme::instance().tokens().muted, 14));
    prevBtn->setToolTip(QStringLiteral("Previous page"));
    prevBtn->setObjectName(QStringLiteral("IconBtn"));
    connect(prevBtn, &QToolButton::clicked, this, &StatusBar::prevPage);

    pageLabel_ = new QLabel(QStringLiteral("Page 1 of 1"), this);
    pageLabel_->setProperty("muted", true);
    QFont pf = pageLabel_->font();
    pf.setPixelSize(11);
    pageLabel_->setFont(pf);

    auto* nextBtn = new QToolButton(this);
    nextBtn->setIcon(Icons::named(QStringLiteral("next"), Theme::instance().tokens().muted, 14));
    nextBtn->setToolTip(QStringLiteral("Next page"));
    nextBtn->setObjectName(QStringLiteral("IconBtn"));
    connect(nextBtn, &QToolButton::clicked, this, &StatusBar::nextPage);

    auto* lastBtn = new QToolButton(this);
    lastBtn->setIcon(Icons::named(QStringLiteral("last"), Theme::instance().tokens().muted, 14));
    lastBtn->setToolTip(QStringLiteral("Last page"));
    lastBtn->setObjectName(QStringLiteral("IconBtn"));
    connect(lastBtn, &QToolButton::clicked, this, &StatusBar::lastPage);

    auto* singleView = new QToolButton(this);
    singleView->setIcon(Icons::named(QStringLiteral("viewer"), Theme::instance().tokens().accent, 14));
    singleView->setToolTip(QStringLiteral("Single Page"));
    singleView->setObjectName(QStringLiteral("IconBtn"));
    singleView->setCheckable(true);
    singleView->setChecked(true);

    auto* continuousView = new QToolButton(this);
    continuousView->setIcon(Icons::named(QStringLiteral("organize"), Theme::instance().tokens().muted, 14));
    continuousView->setToolTip(QStringLiteral("Continuous Scroll"));
    continuousView->setObjectName(QStringLiteral("IconBtn"));
    continuousView->setCheckable(true);

    auto* facingView = new QToolButton(this);
    facingView->setIcon(Icons::named(QStringLiteral("images"), Theme::instance().tokens().muted, 14));
    facingView->setToolTip(QStringLiteral("Facing Pages"));
    facingView->setObjectName(QStringLiteral("IconBtn"));
    facingView->setCheckable(true);

    lay->addWidget(firstBtn);
    lay->addWidget(prevBtn);
    lay->addWidget(pageLabel_);
    lay->addWidget(nextBtn);
    lay->addWidget(lastBtn);
    lay->addSpacing(12);
    lay->addWidget(singleView);
    lay->addWidget(continuousView);
    lay->addWidget(facingView);

    lay->addStretch();

    privacy_ = new QLabel(QStringLiteral("● Zero-Telemetry Local Sandbox"), this);
    privacy_->setStyleSheet(
        QStringLiteral("color: #10B981; font-size: 10px; font-family: 'JetBrains Mono','Consolas',monospace;"));

    auto* minusBtn = new QToolButton(this);
    minusBtn->setIcon(Icons::named(QStringLiteral("minus"), Theme::instance().tokens().muted, 14));
    minusBtn->setToolTip(QStringLiteral("Zoom out"));
    minusBtn->setObjectName(QStringLiteral("IconBtn"));
    connect(minusBtn, &QToolButton::clicked, this, [this] {
        const int v = zoomSlider_->value() - 10;
        zoomSlider_->setValue(std::max(25, v));
    });

    zoomSlider_ = new QSlider(Qt::Horizontal, this);
    zoomSlider_->setRange(25, 400);
    zoomSlider_->setValue(100);
    zoomSlider_->setFixedWidth(120);
    zoomSlider_->setToolTip(QStringLiteral("Zoom"));
    connect(zoomSlider_, &QSlider::valueChanged, this, [this](int v) {
        zoomLabel_->setText(QStringLiteral("%1%").arg(v));
        emit zoomChanged(v);
    });

    auto* plusBtn = new QToolButton(this);
    plusBtn->setIcon(Icons::named(QStringLiteral("plus"), Theme::instance().tokens().muted, 14));
    plusBtn->setToolTip(QStringLiteral("Zoom in"));
    plusBtn->setObjectName(QStringLiteral("IconBtn"));
    connect(plusBtn, &QToolButton::clicked, this, [this] {
        const int v = zoomSlider_->value() + 10;
        zoomSlider_->setValue(std::min(400, v));
    });

    zoomLabel_ = new QLabel(QStringLiteral("100%"), this);
    zoomLabel_->setProperty("muted", true);
    QFont zf = zoomLabel_->font();
    zf.setPixelSize(10);
    zf.setFamily(QStringLiteral("JetBrains Mono"));
    zoomLabel_->setFont(zf);

    auto* fitWidthBtn = new QToolButton(this);
    fitWidthBtn->setIcon(Icons::named(QStringLiteral("fit_width"), Theme::instance().tokens().muted, 14));
    fitWidthBtn->setToolTip(QStringLiteral("Fit Width"));
    fitWidthBtn->setObjectName(QStringLiteral("IconBtn"));
    connect(fitWidthBtn, &QToolButton::clicked, this, &StatusBar::fitWidth);

    auto* fitPageBtn = new QToolButton(this);
    fitPageBtn->setIcon(Icons::named(QStringLiteral("fit_page"), Theme::instance().tokens().muted, 14));
    fitPageBtn->setToolTip(QStringLiteral("Fit Page"));
    fitPageBtn->setObjectName(QStringLiteral("IconBtn"));
    connect(fitPageBtn, &QToolButton::clicked, this, &StatusBar::fitPage);

    lay->addWidget(privacy_);
    lay->addSpacing(12);
    lay->addWidget(minusBtn);
    lay->addWidget(zoomSlider_);
    lay->addWidget(plusBtn);
    lay->addWidget(zoomLabel_);
    lay->addWidget(fitWidthBtn);
    lay->addWidget(fitPageBtn);
}

void StatusBar::setPageInfo(int page, int total) {
    pageLabel_->setText(QStringLiteral("Page %1 of %2").arg(page).arg(total));
}

void StatusBar::setZoom(int percent) {
    zoomSlider_->setValue(percent);
    zoomLabel_->setText(QStringLiteral("%1%").arg(percent));
}

void StatusBar::setPrivacyText(const QString& text) {
    privacy_->setText(text);
}

} // namespace drpdf
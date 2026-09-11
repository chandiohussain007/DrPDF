#include "title_bar.h"

#include "ui/icons/icons.h"
#include "ui/theme/theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>

namespace drpdf {

TitleBar::TitleBar(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("TitleBar"));
    setFixedHeight(44);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(12, 6, 12, 6);
    lay->setSpacing(6);

    // Brand: provided logo + name
    auto* logo = new QLabel(this);
    QPixmap lp = Icons::logo(28);
    if (!lp.isNull()) {
        logo->setPixmap(lp);
    } else {
        logo->setPixmap(Icons::pixmap(QStringLiteral("pdf"), Theme::instance().tokens().accent, 28));
    }
    logo->setAccessibleName(QStringLiteral("Dr PDF logo"));

    auto* brand = new QLabel(QStringLiteral("Dr PDF"), this);
    QFont bf = brand->font();
    bf.setPixelSize(14);
    bf.setWeight(QFont::DemiBold);
    brand->setFont(bf);
    brand->setProperty("strong", true);

    auto* sep = new QLabel(QStringLiteral("|"), this);
    sep->setProperty("muted", true);

    auto* openBtn = new QToolButton(this);
    openBtn->setText(QStringLiteral("  Open  "));
    openBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    openBtn->setIcon(Icons::named(QStringLiteral("open"), Theme::instance().tokens().muted, 18));
    openBtn->setObjectName(QStringLiteral("IconBtn"));
    openBtn->setAccessibleName(QStringLiteral("Open files"));
    connect(openBtn, &QToolButton::clicked, this, &TitleBar::openRequested);

    docName_ = new QLabel(QStringLiteral("No document"), this);
    docName_->setProperty("muted", true);
    QFont df = docName_->font();
    df.setPixelSize(12);
    df.setFamily(QStringLiteral("JetBrains Mono"));
    docName_->setFont(df);

    badge_ = new QLabel(QStringLiteral("Local & Offline"), this);
    badge_->setStyleSheet(QStringLiteral("background: #111827; color: #38BDF8; border-radius: 10px; padding: 2px 10px; font-size: 10px;"));
    badge_->setVisible(false);

    // Search pill (opens command palette)
    auto* searchBtn = new QPushButton(this);
    searchBtn->setText(QStringLiteral("  Tools & Commands   Ctrl+K  "));
    searchBtn->setIcon(Icons::pixmap(QStringLiteral("search"), Theme::instance().tokens().muted, 16));
    searchBtn->setObjectName(QStringLiteral("CyanBtn"));
    searchBtn->setAccessibleName(QStringLiteral("Command palette"));
    searchBtn->setCursor(Qt::PointingHandCursor);
    connect(searchBtn, &QPushButton::clicked, this, &TitleBar::commandPaletteRequested);

    lay->addWidget(logo);
    lay->addWidget(brand);
    lay->addWidget(sep);
    lay->addWidget(openBtn);
    lay->addWidget(docName_, 1);
    lay->addWidget(badge_);
    lay->addSpacing(6);
    lay->addWidget(searchBtn);
}

void TitleBar::setDocumentName(const QString& name) {
    docName_->setText(name);
}

void TitleBar::setLocalBadgeVisible(bool on) {
    badge_->setVisible(on);
}

} // namespace drpdf
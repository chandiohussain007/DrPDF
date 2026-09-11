#include "security_dock.h"

#include "ui/icons/icons.h"
#include "ui/theme/theme.h"

#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

namespace drpdf {

SecurityDock::SecurityDock(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("SecurityDock"));
    setMinimumWidth(280);
    setMaximumWidth(380);

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // Header
    auto* header = new QWidget(this);
    header->setObjectName(QStringLiteral("DockHeader"));
    header->setFixedHeight(36);
    auto* hlay = new QHBoxLayout(header);
    hlay->setContentsMargins(12, 0, 12, 0);
    hlay->setSpacing(6);

    auto* icon = new QLabel(this);
    icon->setPixmap(Icons::pixmap(QStringLiteral("shield"), Theme::instance().tokens().gold, 16));
    auto* title = new QLabel(QStringLiteral("Protect & Cryptography Hub"), this);
    QFont tf = title->font();
    tf.setPixelSize(13);
    tf.setWeight(QFont::DemiBold);
    title->setFont(tf);
    hlay->addWidget(icon);
    hlay->addWidget(title);
    hlay->addStretch();

    // Body
    auto* body = new QWidget(this);
    body->setObjectName(QStringLiteral("DockBody"));
    auto* bodyLay = new QVBoxLayout(body);
    bodyLay->setContentsMargins(12, 8, 12, 8);
    bodyLay->setSpacing(12);

    // Signature section
    auto* sigLabel = new QLabel(QStringLiteral("Digital & Visual Signature"), this);
    sigLabel->setProperty("section", true);
    QFont sf = sigLabel->font();
    sf.setPixelSize(10);
    sf.setWeight(QFont::DemiBold);
    sf.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    sigLabel->setFont(sf);
    sigLabel->setStyleSheet(QStringLiteral("color: #94A3B8; text-transform: uppercase;"));
    bodyLay->addWidget(sigLabel);

    // Signature mode tabs
    auto* sigTabs = new QHBoxLayout();
    auto* drawTab = new QToolButton(this);
    drawTab->setText(QStringLiteral("Draw"));
    drawTab->setObjectName(QStringLiteral("RibbonTab"));
    drawTab->setCheckable(true);
    drawTab->setChecked(true);
    auto* typeTab = new QToolButton(this);
    typeTab->setText(QStringLiteral("Type"));
    typeTab->setObjectName(QStringLiteral("RibbonTab"));
    typeTab->setCheckable(true);
    auto* uploadTab = new QToolButton(this);
    uploadTab->setText(QStringLiteral("Upload"));
    uploadTab->setObjectName(QStringLiteral("RibbonTab"));
    uploadTab->setCheckable(true);
    auto* certTab = new QToolButton(this);
    certTab->setText(QStringLiteral("PKCS#12"));
    certTab->setObjectName(QStringLiteral("RibbonTab"));
    certTab->setCheckable(true);
    sigTabs->addWidget(drawTab);
    sigTabs->addWidget(typeTab);
    sigTabs->addWidget(uploadTab);
    sigTabs->addWidget(certTab);
    bodyLay->addItem(sigTabs);

    // Signature pad
    sigStack_ = new QStackedWidget(this);
    auto* drawPad = new QFrame(this);
    drawPad->setObjectName(QStringLiteral("SignaturePad"));
    drawPad->setFixedHeight(80);
    auto* padLay = new QVBoxLayout(drawPad);
    padLay->setContentsMargins(8, 4, 8, 4);
    auto* padLabel = new QLabel(QStringLiteral("Click or drag to draw signature"), this);
    padLabel->setProperty("muted", true);
    padLabel->setAlignment(Qt::AlignCenter);
    padLay->addWidget(padLabel);
    sigStack_->addWidget(drawPad);
    sigStack_->addWidget(new QWidget(this));
    sigStack_->addWidget(new QWidget(this));
    sigStack_->addWidget(new QWidget(this));
    bodyLay->addWidget(sigStack_);

    // Encryption section
    auto* encLabel = new QLabel(QStringLiteral("Military-Grade Encryption"), this);
    encLabel->setProperty("section", true);
    encLabel->setFont(sf);
    encLabel->setStyleSheet(QStringLiteral("color: #94A3B8; text-transform: uppercase;"));
    bodyLay->addWidget(encLabel);

    auto* passCard = new QFrame(this);
    passCard->setObjectName(QStringLiteral("Card"));
    auto* passLay = new QVBoxLayout(passCard);
    passLay->setContentsMargins(12, 8, 12, 8);
    passLay->setSpacing(6);

    auto* passToggle = new QCheckBox(QStringLiteral("Require Password to Open"), this);
    passToggle->setChecked(true);
    passLay->addWidget(passToggle);

    password_ = new QLineEdit(this);
    password_->setPlaceholderText(QStringLiteral("Master Document Password"));
    password_->setEchoMode(QLineEdit::Password);
    password_->setText(QStringLiteral("Vance•Quantum•Vault•9981#"));
    passLay->addWidget(password_);

    auto* entropyBar = new QFrame(this);
    entropyBar->setFixedHeight(4);
    entropyBar->setStyleSheet(QStringLiteral("background: #10B981; border-radius: 2px;"));
    passLay->addWidget(entropyBar);

    bodyLay->addWidget(passCard);

    // Permissions
    auto* permLabel = new QLabel(QStringLiteral("Granular QPDF Restrictions"), this);
    permLabel->setProperty("section", true);
    permLabel->setFont(sf);
    permLabel->setStyleSheet(QStringLiteral("color: #94A3B8; text-transform: uppercase;"));
    bodyLay->addWidget(permLabel);

    editCheck_ = new QCheckBox(QStringLiteral("Prevent text editing & annotations"), this);
    editCheck_->setChecked(true);
    printCheck_ = new QCheckBox(QStringLiteral("Restrict printing to low-resolution"), this);
    printCheck_->setChecked(true);
    copyCheck_ = new QCheckBox(QStringLiteral("Disallow content copying"), this);
    copyCheck_->setChecked(true);
    rearrangeCheck_ = new QCheckBox(QStringLiteral("Prohibit page rearrange & split"), this);

    bodyLay->addWidget(editCheck_);
    bodyLay->addWidget(printCheck_);
    bodyLay->addWidget(copyCheck_);
    bodyLay->addWidget(rearrangeCheck_);

    bodyLay->addStretch();

    // Action buttons
    auto* sealBtn = new QPushButton(QStringLiteral("Encrypt, Sign & Seal Document"), this);
    sealBtn->setObjectName(QStringLiteral("GoldBtn"));
    sealBtn->setCursor(Qt::PointingHandCursor);
    connect(sealBtn, &QPushButton::clicked, this, &SecurityDock::encryptRequested);

    auto* exportBtn = new QPushButton(QStringLiteral("Export Encrypted Derivative Copy"), this);
    exportBtn->setObjectName(QStringLiteral("GhostBtn"));
    exportBtn->setCursor(Qt::PointingHandCursor);

    bodyLay->addWidget(sealBtn);
    bodyLay->addWidget(exportBtn);

    lay->addWidget(header);
    lay->addWidget(body, 1);
}

QString SecurityDock::password() const { return password_ ? password_->text() : QString(); }
bool SecurityDock::hasPassword() const { return password_ && !password_->text().isEmpty(); }
bool SecurityDock::preventEdit() const { return editCheck_ && editCheck_->isChecked(); }
bool SecurityDock::restrictPrinting() const { return printCheck_ && printCheck_->isChecked(); }
bool SecurityDock::preventCopy() const { return copyCheck_ && copyCheck_->isChecked(); }
bool SecurityDock::preventRearrange() const { return rearrangeCheck_ && rearrangeCheck_->isChecked(); }

} // namespace drpdf
#include "inspector_dock.h"

#include "ui/icons/icons.h"
#include "ui/theme/theme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

namespace drpdf {
namespace {

QFrame* makeSection(const QString& title, QWidget* parent) {
    auto* frame = new QFrame(parent);
    frame->setObjectName(QStringLiteral("DockSection"));
    auto* lay = new QVBoxLayout(frame);
    lay->setContentsMargins(12, 8, 12, 8);
    lay->setSpacing(6);

    auto* label = new QLabel(title, parent);
    label->setProperty("section", true);
    QFont f = label->font();
    f.setPixelSize(10);
    f.setWeight(QFont::DemiBold);
    f.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    label->setFont(f);
    label->setStyleSheet(QStringLiteral("color: #94A3B8; text-transform: uppercase;"));
    lay->addWidget(label);
    return frame;
}

} // namespace

InspectorDock::InspectorDock(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("Inspector"));
    setMinimumWidth(260);
    setMaximumWidth(340);

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // Tab bar
    auto* tabBar = new QWidget(this);
    tabBar->setObjectName(QStringLiteral("DockHeader"));
    tabBar->setFixedHeight(36);
    auto* tlay = new QHBoxLayout(tabBar);
    tlay->setContentsMargins(8, 0, 8, 0);
    tlay->setSpacing(2);

    auto* typographyTab = new QToolButton(this);
    typographyTab->setText(QStringLiteral("Typography"));
    typographyTab->setObjectName(QStringLiteral("RibbonTab"));
    typographyTab->setCheckable(true);
    typographyTab->setChecked(true);

    auto* annotTab = new QToolButton(this);
    annotTab->setText(QStringLiteral("Annotation"));
    annotTab->setObjectName(QStringLiteral("RibbonTab"));
    annotTab->setCheckable(true);

    tlay->addWidget(typographyTab);
    tlay->addWidget(annotTab);
    tlay->addStretch();

    // Stacked content
    stack_ = new QStackedWidget(this);
    stack_->setObjectName(QStringLiteral("DockBody"));

    // Typography page
    auto* typoPage = new QWidget(this);
    auto* typoLay = new QVBoxLayout(typoPage);
    typoLay->setContentsMargins(0, 0, 0, 0);
    typoLay->setSpacing(0);

    auto* fontSection = makeSection(QStringLiteral("Font Family"), this);
    auto* fontCombo = new QComboBox(this);
    fontCombo->addItems({QStringLiteral("Inter Display"), QStringLiteral("JetBrains Mono"),
                          QStringLiteral("Helvetica Neue"), QStringLiteral("Liberation Sans")});
    fontCombo->setObjectName(QStringLiteral("fontCombo"));
    fontSection->layout()->addWidget(fontCombo);
    typoLay->addWidget(fontSection);

    auto* weightSection = makeSection(QStringLiteral("Weight & Posture"), this);
    auto* weightRow = new QHBoxLayout();
    auto* weightCombo = new QComboBox(this);
    weightCombo->addItems({QStringLiteral("SemiBold (600)"), QStringLiteral("Regular (400)"),
                            QStringLiteral("Medium (500)"), QStringLiteral("Bold (700)")});
    auto* postureCombo = new QComboBox(this);
    postureCombo->addItems({QStringLiteral("Normal Posture"), QStringLiteral("Italic Slant")});
    weightRow->addWidget(weightCombo, 1);
    weightRow->addWidget(postureCombo, 1);
    weightSection->layout()->addItem(weightRow);
    typoLay->addWidget(weightSection);

    auto* metricsSection = makeSection(QStringLiteral("Type Metrics"), this);
    auto* metricsRow = new QHBoxLayout();
    auto* sizeSpin = new QLineEdit(this);
    sizeSpin->setText(QStringLiteral("14 pt"));
    sizeSpin->setObjectName(QStringLiteral("valueField"));
    auto* leadingSpin = new QLineEdit(this);
    leadingSpin->setText(QStringLiteral("1.42"));
    leadingSpin->setObjectName(QStringLiteral("valueField"));
    auto* trackingSpin = new QLineEdit(this);
    trackingSpin->setText(QStringLiteral("-0.01em"));
    trackingSpin->setObjectName(QStringLiteral("valueField"));
    metricsRow->addWidget(sizeSpin);
    metricsRow->addWidget(leadingSpin);
    metricsRow->addWidget(trackingSpin);
    metricsSection->layout()->addItem(metricsRow);
    typoLay->addWidget(metricsSection);

    auto* colorSection = makeSection(QStringLiteral("Fill Color"), this);
    auto* colorRow = new QHBoxLayout();
    auto* colorSwatch = new QFrame(this);
    colorSwatch->setObjectName(QStringLiteral("Swatch"));
    colorSwatch->setFixedSize(28, 28);
    colorSwatch->setStyleSheet(QStringLiteral("background: #00D2FF; border-radius: 9999px; border: 1px solid rgba(255,255,255,0.1);"));
    auto* colorHex = new QLabel(QStringLiteral("#00D2FF"), this);
    colorHex->setProperty("value", true);
    colorRow->addWidget(colorSwatch);
    colorRow->addWidget(colorHex);
    colorRow->addStretch();
    colorSection->layout()->addItem(colorRow);
    typoLay->addWidget(colorSection);

    auto* opacitySection = makeSection(QStringLiteral("Layer Opacity"), this);
    auto* opacityRow = new QHBoxLayout();
    auto* opacitySlider = new QSlider(Qt::Horizontal, this);
    opacitySlider->setRange(0, 100);
    opacitySlider->setValue(100);
    auto* opacityLabel = new QLabel(QStringLiteral("100%"), this);
    opacityLabel->setProperty("value", true);
    opacityRow->addWidget(opacitySlider, 1);
    opacityRow->addWidget(opacityLabel);
    opacitySection->layout()->addItem(opacityRow);
    typoLay->addWidget(opacitySection);

    auto* alignSection = makeSection(QStringLiteral("Paragraph Alignment"), this);
    auto* alignRow = new QHBoxLayout();
    auto* alignLeft = new QToolButton(this);
    alignLeft->setIcon(Icons::named(QStringLiteral("edit"), Theme::instance().tokens().accent, 16));
    alignLeft->setObjectName(QStringLiteral("IconBtn"));
    alignLeft->setCheckable(true);
    alignLeft->setChecked(true);
    auto* alignCenter = new QToolButton(this);
    alignCenter->setIcon(Icons::named(QStringLiteral("edit"), Theme::instance().tokens().muted, 16));
    alignCenter->setObjectName(QStringLiteral("IconBtn"));
    alignCenter->setCheckable(true);
    auto* alignRight = new QToolButton(this);
    alignRight->setIcon(Icons::named(QStringLiteral("edit"), Theme::instance().tokens().muted, 16));
    alignRight->setObjectName(QStringLiteral("IconBtn"));
    alignRight->setCheckable(true);
    auto* alignJustify = new QToolButton(this);
    alignJustify->setIcon(Icons::named(QStringLiteral("edit"), Theme::instance().tokens().muted, 16));
    alignJustify->setObjectName(QStringLiteral("IconBtn"));
    alignJustify->setCheckable(true);
    alignRow->addWidget(alignLeft);
    alignRow->addWidget(alignCenter);
    alignRow->addWidget(alignRight);
    alignRow->addWidget(alignJustify);
    alignSection->layout()->addItem(alignRow);
    typoLay->addWidget(alignSection);

    typoLay->addStretch();

    // Document info footer
    auto* infoFrame = new QFrame(this);
    infoFrame->setObjectName(QStringLiteral("DockFooter"));
    auto* infoLay = new QVBoxLayout(infoFrame);
    infoLay->setContentsMargins(12, 8, 12, 8);
    infoLay->setSpacing(2);
    auto* profileLabel = new QLabel(QStringLiteral("PDF 1.7 (ISO 32000-1)"), this);
    profileLabel->setProperty("value", true);
    auto* validLabel = new QLabel(QStringLiteral("Valid Profile"), this);
    validLabel->setStyleSheet(QStringLiteral("color: #10B981;"));
    infoLay->addWidget(profileLabel);
    infoLay->addWidget(validLabel);

    stack_->addWidget(typoPage);
    stack_->addWidget(new QWidget(this)); // Annotation page placeholder

    lay->addWidget(tabBar);
    lay->addWidget(stack_, 1);
    lay->addWidget(infoFrame);
}

void InspectorDock::setPageCount(int count) {
    Q_UNUSED(count);
}

void InspectorDock::setPdfVersion(const QString& version) {
    Q_UNUSED(version);
}

void InspectorDock::setSecurityStatus(const QString& status) {
    Q_UNUSED(status);
}

} // namespace drpdf
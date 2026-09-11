#include "optimization_dock.h"

#include "ui/icons/icons.h"
#include "ui/theme/theme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

namespace drpdf {

OptimizationDock::OptimizationDock(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("OptimizationDock"));
    setMinimumWidth(280);
    setMaximumWidth(360);

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
    icon->setPixmap(Icons::pixmap(QStringLiteral("compress"), Theme::instance().tokens().accent, 16));
    auto* title = new QLabel(QStringLiteral("Optimization Studio"), this);
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

    // Savings card
    auto* savingsCard = new QFrame(this);
    savingsCard->setObjectName(QStringLiteral("Card"));
    auto* savingsLay = new QVBoxLayout(savingsCard);
    savingsLay->setContentsMargins(12, 8, 12, 8);
    savingsLay->setSpacing(4);

    auto* savingsHeader = new QHBoxLayout();
    auto* savingsTitle = new QLabel(QStringLiteral("Storage Reduction Metric"), this);
    savingsTitle->setProperty("section", true);
    QFont sf = savingsTitle->font();
    sf.setPixelSize(10);
    sf.setWeight(QFont::DemiBold);
    savingsTitle->setFont(sf);
    savingsTitle->setStyleSheet(QStringLiteral("color: #94A3B8; text-transform: uppercase;"));
    auto* savingsPct = new QLabel(QStringLiteral("-85.5%"), this);
    savingsPct->setStyleSheet(QStringLiteral("color: #10B981; font-weight: 600; font-size: 12px; background: rgba(16,185,129,0.1); padding: 2px 8px; border-radius: 4px;"));
    savingsHeader->addWidget(savingsTitle);
    savingsHeader->addStretch();
    savingsHeader->addWidget(savingsPct);
    savingsLay->addItem(savingsHeader);

    auto* sizeRow = new QHBoxLayout();
    auto* origSize = new QLabel(QStringLiteral("42.8 MB"), this);
    origSize->setStyleSheet(QStringLiteral("color: #94A3B8; text-decoration: line-through; font-size: 16px; font-weight: 600;"));
    auto* arrow = new QLabel(QStringLiteral("→"), this);
    arrow->setProperty("muted", true);
    auto* projSize = new QLabel(QStringLiteral("6.2 MB"), this);
    projSize->setStyleSheet(QStringLiteral("color: #00D2FF; font-size: 20px; font-weight: 700;"));
    sizeRow->addWidget(origSize);
    sizeRow->addWidget(arrow);
    sizeRow->addWidget(projSize);
    savingsLay->addItem(sizeRow);

    // Progress bar
    auto* prog = new QFrame(this);
    prog->setFixedHeight(8);
    prog->setStyleSheet(QStringLiteral("background: #262A35; border-radius: 4px;"));
    auto* progFill = new QFrame(prog);
    progFill->setFixedWidth(80);
    progFill->setStyleSheet(QStringLiteral("background: #00D2FF; border-radius: 4px;"));
    savingsLay->addWidget(prog);

    bodyLay->addWidget(savingsCard);

    // JPEG Quality slider
    auto* qualityLabel = new QLabel(QStringLiteral("JPEG / WebP Quality"), this);
    qualityLabel->setProperty("section", true);
    qualityLabel->setFont(sf);
    qualityLabel->setStyleSheet(QStringLiteral("color: #94A3B8; text-transform: uppercase;"));
    bodyLay->addWidget(qualityLabel);

    auto* qualityRow = new QHBoxLayout();
    qualitySlider_ = new QSlider(Qt::Horizontal, this);
    qualitySlider_->setRange(10, 100);
    qualitySlider_->setValue(80);
    auto* qualityVal = new QLabel(QStringLiteral("80%"), this);
    qualityVal->setProperty("value", true);
    connect(qualitySlider_, &QSlider::valueChanged, this, [this, qualityVal](int v) {
        qualityVal->setText(QStringLiteral("%1%").arg(v));
        emit qualityChanged(v);
    });
    qualityRow->addWidget(qualitySlider_, 1);
    qualityRow->addWidget(qualityVal);
    bodyLay->addItem(qualityRow);

    // Color downsampling
    auto* colorLabel = new QLabel(QStringLiteral("Color Downsampling Filter"), this);
    colorLabel->setProperty("section", true);
    colorLabel->setFont(sf);
    colorLabel->setStyleSheet(QStringLiteral("color: #94A3B8; text-transform: uppercase;"));
    bodyLay->addWidget(colorLabel);

    colorFilter_ = new QComboBox(this);
    colorFilter_->addItems({QStringLiteral("Bicubic Downsampling to 150 DPI (Web & Screen)"),
                             QStringLiteral("Subsampling to 200 DPI (Balanced)"),
                             QStringLiteral("Lanczos Filtering to 300 DPI (High Print)"),
                             QStringLiteral("Retain Full Resolution (Skip)")});
    bodyLay->addWidget(colorFilter_);

    // Monochrome downsampling
    auto* monoLabel = new QLabel(QStringLiteral("Monochrome Line Art Subsampling"), this);
    monoLabel->setProperty("section", true);
    monoLabel->setFont(sf);
    monoLabel->setStyleSheet(QStringLiteral("color: #94A3B8; text-transform: uppercase;"));
    bodyLay->addWidget(monoLabel);

    monoFilter_ = new QComboBox(this);
    monoFilter_->addItems({QStringLiteral("JBIG2 Adaptive Lossless to 300 DPI"),
                            QStringLiteral("CCITT Group 4 Fax at 300 DPI"),
                            QStringLiteral("JBIG2 Lossy (Extreme Space Saving)")});
    bodyLay->addWidget(monoFilter_);

    // Structural stream purges
    auto* purgeLabel = new QLabel(QStringLiteral("Structural Stream Purges"), this);
    purgeLabel->setProperty("section", true);
    purgeLabel->setFont(sf);
    purgeLabel->setStyleSheet(QStringLiteral("color: #94A3B8; text-transform: uppercase;"));
    bodyLay->addWidget(purgeLabel);

    exifCheck_ = new QCheckBox(QStringLiteral("Strip EXIF, XMP & Private Tags"), this);
    exifCheck_->setChecked(true);
    fontCheck_ = new QCheckBox(QStringLiteral("Deflate Unused Font Stream Glyphs"), this);
    fontCheck_->setChecked(true);
    linearCheck_ = new QCheckBox(QStringLiteral("Linearize for Fast Web Viewing"), this);
    linearCheck_->setChecked(true);
    downsampleCheck_ = new QCheckBox(QStringLiteral("Downsample Embedded CMYK / RGB Bitmaps"), this);
    downsampleCheck_->setChecked(true);

    bodyLay->addWidget(exifCheck_);
    bodyLay->addWidget(fontCheck_);
    bodyLay->addWidget(linearCheck_);
    bodyLay->addWidget(downsampleCheck_);

    bodyLay->addStretch();

    // Optimize button
    auto* optimizeBtn = new QPushButton(QStringLiteral("Optimize & Save Document"), this);
    optimizeBtn->setObjectName(QStringLiteral("CyanBtn"));
    optimizeBtn->setCursor(Qt::PointingHandCursor);
    connect(optimizeBtn, &QPushButton::clicked, this, &OptimizationDock::optimizeRequested);
    bodyLay->addWidget(optimizeBtn);

    lay->addWidget(header);
    lay->addWidget(body, 1);
}

int OptimizationDock::quality() const { return qualitySlider_ ? qualitySlider_->value() : 80; }
int OptimizationDock::dpi() const { return 150; }
bool OptimizationDock::stripExif() const { return exifCheck_ && exifCheck_->isChecked(); }
bool OptimizationDock::subsetFonts() const { return fontCheck_ && fontCheck_->isChecked(); }
bool OptimizationDock::linearize() const { return linearCheck_ && linearCheck_->isChecked(); }
bool OptimizationDock::downsampleImages() const { return downsampleCheck_ && downsampleCheck_->isChecked(); }

} // namespace drpdf
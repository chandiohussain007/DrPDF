#include "assembly_dock.h"

#include "ui/icons/icons.h"
#include "ui/theme/theme.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

namespace drpdf {

AssemblyDock::AssemblyDock(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("AssemblyDock"));
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
    icon->setPixmap(Icons::pixmap(QStringLiteral("merge"), Theme::instance().tokens().accent, 16));
    auto* title = new QLabel(QStringLiteral("Document Assembly"), this);
    QFont tf = title->font();
    tf.setPixelSize(13);
    tf.setWeight(QFont::DemiBold);
    title->setFont(tf);
    hlay->addWidget(icon);
    hlay->addWidget(title);
    hlay->addStretch();

    // Scrollable body
    auto* body = new QWidget(this);
    body->setObjectName(QStringLiteral("DockBody"));
    auto* bodyLay = new QVBoxLayout(body);
    bodyLay->setContentsMargins(12, 8, 12, 8);
    bodyLay->setSpacing(12);

    // Extraction strategy section
    auto* extractLabel = new QLabel(QStringLiteral("Extraction Strategy"), this);
    extractLabel->setProperty("section", true);
    QFont ef = extractLabel->font();
    ef.setPixelSize(10);
    ef.setWeight(QFont::DemiBold);
    ef.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    extractLabel->setFont(ef);
    extractLabel->setStyleSheet(QStringLiteral("color: #94A3B8; text-transform: uppercase;"));
    bodyLay->addWidget(extractLabel);

    auto* extractCard = new QFrame(this);
    extractCard->setObjectName(QStringLiteral("Card"));
    auto* extractLay = new QVBoxLayout(extractCard);
    extractLay->setContentsMargins(12, 8, 12, 8);
    extractLay->setSpacing(8);

    auto* selRadio = new QRadioButton(QStringLiteral("Extract Selected"), this);
    selRadio->setChecked(true);
    auto* rangeRadio = new QRadioButton(QStringLiteral("Split by Range"), this);
    rangeInput_ = new QLineEdit(this);
    rangeInput_->setPlaceholderText(QStringLiteral("1-4, 5-8, 9-12"));
    rangeInput_->setEnabled(false);
    auto* everyRadio = new QRadioButton(QStringLiteral("Split Every N Pages"), this);
    auto* nSpin = new QLineEdit(this);
    nSpin->setPlaceholderText(QStringLiteral("2"));
    nSpin->setEnabled(false);
    nSpin->setMaximumWidth(60);

    modeGroup_ = new QButtonGroup(this);
    modeGroup_->addButton(selRadio, 0);
    modeGroup_->addButton(rangeRadio, 1);
    modeGroup_->addButton(everyRadio, 2);

    extractLay->addWidget(selRadio);
    extractLay->addWidget(rangeRadio);
    extractLay->addWidget(rangeInput_);
    extractLay->addWidget(everyRadio);
    extractLay->addWidget(nSpin);

    bodyLay->addWidget(extractCard);

    // Modifiers
    auto* delCheck = new QCheckBox(QStringLiteral("Delete pages after extracting"), this);
    delCheck->setChecked(true);
    auto* chunkCheck = new QCheckBox(QStringLiteral("Save each chunk as distinct file"), this);
    auto* flattenCheck = new QCheckBox(QStringLiteral("Sanitize & flatten annotations"), this);
    flattenCheck->setChecked(true);
    bodyLay->addWidget(delCheck);
    bodyLay->addWidget(chunkCheck);
    bodyLay->addWidget(flattenCheck);

    // Assembly queue section
    auto* queueLabel = new QLabel(QStringLiteral("Assembly Queue"), this);
    queueLabel->setProperty("section", true);
    queueLabel->setFont(ef);
    queueLabel->setStyleSheet(QStringLiteral("color: #94A3B8; text-transform: uppercase;"));
    bodyLay->addWidget(queueLabel);

    queue_ = new QListWidget(this);
    queue_->setObjectName(QStringLiteral("queueList"));
    queue_->setMaximumHeight(120);
    bodyLay->addWidget(queue_);

    // Output estimate
    auto* estimateLabel = new QLabel(QStringLiteral("Combined Estimated Size: ~ 5.9 MB"), this);
    estimateLabel->setProperty("value", true);
    QFont estf = estimateLabel->font();
    estf.setPixelSize(10);
    estf.setFamily(QStringLiteral("JetBrains Mono"));
    estimateLabel->setFont(estf);
    bodyLay->addWidget(estimateLabel);

    bodyLay->addStretch();

    // Action buttons
    auto* mergeBtn = new QPushButton(QStringLiteral("Merge to New Document"), this);
    mergeBtn->setObjectName(QStringLiteral("CyanBtn"));
    mergeBtn->setCursor(Qt::PointingHandCursor);
    connect(mergeBtn, &QPushButton::clicked, this, &AssemblyDock::mergeRequested);

    auto* cancelBtn = new QPushButton(QStringLiteral("Cancel"), this);
    cancelBtn->setObjectName(QStringLiteral("GhostBtn"));
    cancelBtn->setCursor(Qt::PointingHandCursor);

    bodyLay->addWidget(mergeBtn);
    bodyLay->addWidget(cancelBtn);

    lay->addWidget(header);
    lay->addWidget(body, 1);
}

} // namespace drpdf
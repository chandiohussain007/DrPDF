#include "pages_dock.h"

#include "services/thumbnails.h"
#include "ui/icons/icons.h"
#include "ui/theme/theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>

namespace drpdf {

PagesDock::PagesDock(ThumbnailCache* cache, QWidget* parent)
    : QWidget(parent), cache_(cache) {
    setObjectName(QStringLiteral("PagesDock"));
    setMinimumWidth(220);
    setMaximumWidth(320);

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
    icon->setPixmap(Icons::pixmap(QStringLiteral("thumbnail_bar"), Theme::instance().tokens().accent, 14));
    headerLabel_ = new QLabel(QStringLiteral("Pages"), this);
    QFont hf = headerLabel_->font();
    hf.setPixelSize(13);
    hf.setWeight(QFont::DemiBold);
    headerLabel_->setFont(hf);

    auto* countLabel = new QLabel(QStringLiteral("0"), this);
    countLabel->setStyleSheet(QStringLiteral("background: #262A35; color: #64748B; border-radius: 4px; padding: 1px 6px; font-size: 10px;"));
    countLabel->setObjectName(QStringLiteral("pageCount"));

    auto* rotateL = new QToolButton(this);
    rotateL->setIcon(Icons::named(QStringLiteral("rotate_left"), Theme::instance().tokens().muted, 14));
    rotateL->setToolTip(QStringLiteral("Rotate Left"));
    rotateL->setObjectName(QStringLiteral("IconBtn"));
    connect(rotateL, &QToolButton::clicked, this, [this] { emit rotateRequested(-90); });

    auto* rotateR = new QToolButton(this);
    rotateR->setIcon(Icons::named(QStringLiteral("rotate_right"), Theme::instance().tokens().muted, 14));
    rotateR->setToolTip(QStringLiteral("Rotate Right"));
    rotateR->setObjectName(QStringLiteral("IconBtn"));
    connect(rotateR, &QToolButton::clicked, this, [this] { emit rotateRequested(90); });

    auto* delBtn = new QToolButton(this);
    delBtn->setIcon(Icons::named(QStringLiteral("trash"), Theme::instance().tokens().danger, 14));
    delBtn->setToolTip(QStringLiteral("Delete Page"));
    delBtn->setObjectName(QStringLiteral("IconBtn"));
    connect(delBtn, &QToolButton::clicked, this, &PagesDock::deleteRequested);

    hlay->addWidget(icon);
    hlay->addWidget(headerLabel_);
    hlay->addWidget(countLabel);
    hlay->addStretch();
    hlay->addWidget(rotateL);
    hlay->addWidget(rotateR);
    hlay->addWidget(delBtn);

    // Zoom slider
    auto* zoomBar = new QWidget(this);
    zoomBar->setFixedHeight(28);
    auto* zlay = new QHBoxLayout(zoomBar);
    zlay->setContentsMargins(12, 0, 12, 0);
    zlay->setSpacing(4);

    auto* fitLabel = new QLabel(QStringLiteral("Fit to Panel"), this);
    fitLabel->setProperty("muted", true);
    QFont fl = fitLabel->font();
    fl.setPixelSize(10);
    fitLabel->setFont(fl);

    auto* minusBtn = new QToolButton(this);
    minusBtn->setIcon(Icons::named(QStringLiteral("minus"), Theme::instance().tokens().muted, 12));
    minusBtn->setObjectName(QStringLiteral("IconBtn"));
    connect(minusBtn, &QToolButton::clicked, this, [this] { emit zoomChanged(-10); });

    zoomSlider_ = new QSlider(Qt::Horizontal, this);
    zoomSlider_->setRange(20, 100);
    zoomSlider_->setValue(60);
    connect(zoomSlider_, &QSlider::valueChanged, this, [this] { emit zoomChanged(0); });

    auto* plusBtn = new QToolButton(this);
    plusBtn->setIcon(Icons::named(QStringLiteral("plus"), Theme::instance().tokens().muted, 12));
    plusBtn->setObjectName(QStringLiteral("IconBtn"));
    connect(plusBtn, &QToolButton::clicked, this, [this] { emit zoomChanged(10); });

    zlay->addWidget(fitLabel);
    zlay->addWidget(minusBtn);
    zlay->addWidget(zoomSlider_, 1);
    zlay->addWidget(plusBtn);

    // Thumbnail list
    list_ = new QListWidget(this);
    list_->setViewMode(QListWidget::IconMode);
    list_->setResizeMode(QListWidget::Adjust);
    list_->setMovement(QListWidget::Static);
    list_->setSpacing(8);
    list_->setObjectName(QStringLiteral("thumbList"));
    connect(list_, &QListWidget::itemSelectionChanged, this, &PagesDock::onItemSelectionChanged);

    lay->addWidget(header);
    lay->addWidget(zoomBar);
    lay->addWidget(list_, 1);
}

void PagesDock::setAssembly(core::Assembly* assembly) {
    assembly_ = assembly;
    rebuild();
}

void PagesDock::refresh() {
    rebuild();
}

void PagesDock::rebuild() {
    list_->clear();
    if (!assembly_) return;

    const int count = assembly_->count();
    const int iconSize = std::max(80, zoomSlider_->value() * 2);

    for (int i = 0; i < count; ++i) {
        auto* item = new QListWidgetItem(list_);
        item->setText(QStringLiteral("Page %1").arg(i + 1));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        item->setData(Qt::UserRole, i);
        item->setSizeHint(QSize(iconSize + 20, iconSize + 40));
        list_->addItem(item);

        const auto& spec = assembly_->pages()[static_cast<size_t>(i)];
        cache_->request(spec.file.string().c_str(), spec.index, QSize(iconSize, iconSize));
    }
}

void PagesDock::onItemSelectionChanged() {
    auto* item = list_->currentItem();
    if (item) {
        currentPage_ = item->data(Qt::UserRole).toInt();
        emit pageSelected(currentPage_);
    }
}

} // namespace drpdf
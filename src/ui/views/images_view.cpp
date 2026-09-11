#include "images_view.h"

#include "app/settings.h"
#include "services/image_pdf.h"
#include "ui/components/banner.h"
#include "ui/components/drop_zone.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QListWidget>
#include <QPageSize>
#include <QPushButton>
#include <QResizeEvent>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>


#include <algorithm>


namespace drpdf {

ImagesView::ImagesView(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    auto* title = new QLabel(QStringLiteral("Images to PDF"), this);
    QFont f = title->font();
    f.setPixelSize(22);
    f.setWeight(QFont::DemiBold);
    title->setFont(f);

    drop_ = new DropZone(this);
    drop_->setAcceptPdf(false);
    drop_->setAcceptImages(true);
    drop_->setTitle(QStringLiteral("Drop images"));
    drop_->setMaximumHeight(120);
    connect(drop_, &DropZone::filesDropped, this, &ImagesView::addImages);

    list_ = new QListWidget(this);
    list_->setDragDropMode(QAbstractItemView::InternalMove);
    list_->setDefaultDropAction(Qt::MoveAction);
    list_->setAccessibleName(QStringLiteral("Images"));

    preview_ = new QLabel(this);
    preview_->setAlignment(Qt::AlignCenter);
    preview_->setMinimumSize(240, 240);
    preview_->setText(QStringLiteral("Select an image"));
    preview_->setProperty("muted", true);
    native_ = new QLabel(this);
    native_->setProperty("muted", true);
    width_ = new QSpinBox(this);
    width_->setRange(32, 20000);
    width_->setSuffix(QStringLiteral(" px"));
    width_->setAccessibleName(QStringLiteral("Target width"));
    height_ = new QSpinBox(this);
    height_->setRange(32, 20000);
    height_->setSuffix(QStringLiteral(" px"));
    height_->setAccessibleName(QStringLiteral("Target height"));
    lockAspect_ = new QCheckBox(QStringLiteral("Lock aspect ratio"), this);
    lockAspect_->setChecked(true);

    auto* previewBox = new QWidget(this);
    auto* pv = new QVBoxLayout(previewBox);
    pv->setContentsMargins(0, 0, 0, 0);
    pv->addWidget(preview_, 1);
    pv->addWidget(native_);
    auto* sizeRow = new QHBoxLayout();
    sizeRow->addWidget(new QLabel(QStringLiteral("W"), previewBox));
    sizeRow->addWidget(width_);
    sizeRow->addWidget(new QLabel(QStringLiteral("H"), previewBox));
    sizeRow->addWidget(height_);
    pv->addLayout(sizeRow);
    pv->addWidget(lockAspect_);

    auto* split = new QSplitter(this);
    split->addWidget(list_);
    split->addWidget(previewBox);
    split->setStretchFactor(0, 2);
    split->setStretchFactor(1, 3);

    auto* opts = new QHBoxLayout();
    size_ = new QComboBox(this);
    size_->addItem(QStringLiteral("A4"), static_cast<int>(QPageSize::A4));
    size_->addItem(QStringLiteral("Letter"), static_cast<int>(QPageSize::Letter));
    size_->addItem(QStringLiteral("Legal"), static_cast<int>(QPageSize::Legal));
    landscape_ = new QCheckBox(QStringLiteral("Landscape"), this);
    fit_ = new QCheckBox(QStringLiteral("Page = image size"), this);
    margin_ = new QSpinBox(this);
    margin_->setRange(0, 40);
    margin_->setValue(8);
    margin_->setSuffix(QStringLiteral(" mm"));
    opts->addWidget(new QLabel(QStringLiteral("Page"), this));
    opts->addWidget(size_);
    opts->addWidget(landscape_);
    opts->addWidget(fit_);
    opts->addWidget(new QLabel(QStringLiteral("Margin"), this));
    opts->addWidget(margin_);
    opts->addStretch();

    banner_ = new Banner(this);
    auto* go = new QPushButton(QStringLiteral("Export PDF"), this);
    go->setProperty("primary", true);
    connect(go, &QPushButton::clicked, this, &ImagesView::exportPdf);
    auto* row = new QHBoxLayout();
    auto* add = new QPushButton(QStringLiteral("Add images"), this);
    auto* rem = new QPushButton(QStringLiteral("Remove"), this);
    connect(add, &QPushButton::clicked, this, [this] {
        addImages(QFileDialog::getOpenFileNames(
            this, QStringLiteral("Images"), AppSettings::instance().lastDirectory(),
            QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.webp)")));
    });
    connect(rem, &QPushButton::clicked, this, [this] {
        qDeleteAll(list_->selectedItems());
        showSelected();
    });
    row->addWidget(add);
    row->addWidget(rem);
    row->addStretch();
    row->addWidget(go);

    connect(list_, &QListWidget::currentRowChanged, this, [this](int) { showSelected(); });
    connect(width_, &QSpinBox::valueChanged, this, [this](int) { applySize(true); });
    connect(height_, &QSpinBox::valueChanged, this, [this](int) { applySize(false); });

    root->addWidget(title);
    root->addWidget(drop_);
    root->addLayout(opts);
    root->addWidget(split, 1);
    root->addWidget(banner_);
    root->addLayout(row);
}

void ImagesView::addImages(const QStringList& paths) {
    for (const auto& p : paths) {
        QImageReader r(p);
        r.setAutoTransform(true);
        const QSize sz = r.size();
        auto* it = new QListWidgetItem(QFileInfo(p).fileName(), list_);
        it->setData(Qt::UserRole, p);
        it->setData(Qt::UserRole + 1, sz.width());
        it->setData(Qt::UserRole + 2, sz.height());
        it->setData(Qt::UserRole + 3, sz.width());
        it->setData(Qt::UserRole + 4, sz.height());
        it->setToolTip(p);
    }
    if (list_->currentRow() < 0 && list_->count() > 0) {
        list_->setCurrentRow(0);
    }
}

void ImagesView::showSelected() {
    auto* it = list_->currentItem();
    if (!it) {
        preview_->setPixmap({});
        preview_->setText(QStringLiteral("Select an image"));
        native_->clear();
        return;
    }
    const QString path = it->data(Qt::UserRole).toString();
    QImage img(path);
    if (img.isNull()) {
        preview_->setText(QStringLiteral("Could not preview"));
        return;
    }
    preview_->setPixmap(QPixmap::fromImage(img).scaled(preview_->size(), Qt::KeepAspectRatio,
                                                       Qt::SmoothTransformation));
    native_->setText(QStringLiteral("Original %1 × %2")
                         .arg(it->data(Qt::UserRole + 3).toInt())
                         .arg(it->data(Qt::UserRole + 4).toInt()));
    const bool block = width_->blockSignals(true);
    height_->blockSignals(true);
    width_->setValue(std::max(32, it->data(Qt::UserRole + 1).toInt()));
    height_->setValue(std::max(32, it->data(Qt::UserRole + 2).toInt()));
    width_->blockSignals(block);
    height_->blockSignals(false);
}

void ImagesView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (list_ && list_->currentItem()) {
        showSelected();
    }
}

void ImagesView::applySize(bool fromWidth) {

    auto* it = list_->currentItem();
    if (!it) {
        return;
    }
    const int ow = std::max(1, it->data(Qt::UserRole + 3).toInt());
    const int oh = std::max(1, it->data(Qt::UserRole + 4).toInt());
    int w = width_->value();
    int h = height_->value();
    if (lockAspect_->isChecked()) {
        if (fromWidth) {
            h = std::max(32, int(double(w) * oh / ow));
            const bool b = height_->blockSignals(true);
            height_->setValue(h);
            height_->blockSignals(b);
        } else {
            w = std::max(32, int(double(h) * ow / oh));
            const bool b = width_->blockSignals(true);
            width_->setValue(w);
            width_->blockSignals(b);
        }
    }
    it->setData(Qt::UserRole + 1, w);
    it->setData(Qt::UserRole + 2, h);
}

void ImagesView::exportPdf() {
    QVector<ImageInput> items;
    for (int i = 0; i < list_->count(); ++i) {
        auto* it = list_->item(i);
        ImageInput in;
        in.path = it->data(Qt::UserRole).toString();
        in.width = it->data(Qt::UserRole + 1).toInt();
        in.height = it->data(Qt::UserRole + 2).toInt();
        items.push_back(in);
    }
    if (items.isEmpty()) {
        banner_->showError(QStringLiteral("Add images first."));
        return;
    }
    const QString out = QFileDialog::getSaveFileName(
        this, QStringLiteral("Export PDF"),
        AppSettings::instance().lastDirectory() + QStringLiteral("/images.pdf"),
        QStringLiteral("PDF files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }
    ImagePdfOptions opt;
    opt.pageSize = static_cast<QPageSize::PageSizeId>(size_->currentData().toInt());
    opt.landscape = landscape_->isChecked();
    opt.fitToImage = fit_->isChecked();
    opt.marginMm = margin_->value();
    auto r = writeImagesToPdf(items, out, opt);
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
    } else {
        banner_->showInfo(QStringLiteral("Saved ") + out);
        AppSettings::instance().addRecentFile(out);
        emit exported(out);
    }
}

} // namespace drpdf

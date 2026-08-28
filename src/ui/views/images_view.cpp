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
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

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
    connect(rem, &QPushButton::clicked, this, [this] { qDeleteAll(list_->selectedItems()); });
    row->addWidget(add);
    row->addWidget(rem);
    row->addStretch();
    row->addWidget(go);

    root->addWidget(title);
    root->addWidget(drop_);
    root->addLayout(opts);
    root->addWidget(list_, 1);
    root->addWidget(banner_);
    root->addLayout(row);
}

void ImagesView::addImages(const QStringList& paths) {
    for (const auto& p : paths) {
        auto* it = new QListWidgetItem(QFileInfo(p).fileName(), list_);
        it->setData(Qt::UserRole, p);
        it->setToolTip(p);
    }
}

void ImagesView::exportPdf() {
    QStringList paths;
    for (int i = 0; i < list_->count(); ++i) {
        paths << list_->item(i)->data(Qt::UserRole).toString();
    }
    if (paths.isEmpty()) {
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
    auto r = writeImagesToPdf(paths, out, opt);
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
    } else {
        banner_->showInfo(QStringLiteral("Saved ") + out);
        AppSettings::instance().addRecentFile(out);
        emit exported(out);
    }
}

} // namespace drpdf

#include "merge_view.h"

#include "app/settings.h"
#include "ui/components/banner.h"
#include "ui/components/drop_zone.h"
#include "ui/components/page_grid.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QtConcurrent>
#include <QVBoxLayout>


namespace drpdf {

MergeView::MergeView(ThumbnailCache* cache, QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    root->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("Merge PDFs"), this);
    QFont f = title->font();
    f.setPixelSize(22);
    f.setWeight(QFont::DemiBold);
    title->setFont(f);
    auto* hint = new QLabel(QStringLiteral("Add files, drag pages to reorder, then export one PDF."), this);
    hint->setProperty("muted", true);

    drop_ = new DropZone(this);
    drop_->setTitle(QStringLiteral("Drop PDFs to add"));
    drop_->setMaximumHeight(120);
    connect(drop_, &DropZone::filesDropped, this, &MergeView::addFiles);

    grid_ = new PageGrid(cache, this);
    grid_->setAssembly(&assembly_);

    banner_ = new Banner(this);

    auto* row = new QHBoxLayout();
    auto* addBtn = new QPushButton(QStringLiteral("Add PDFs"), this);
    auto* rot = new QPushButton(QStringLiteral("Rotate 90°"), this);
    auto* del = new QPushButton(QStringLiteral("Remove page"), this);
    auto* clear = new QPushButton(QStringLiteral("Clear"), this);
    auto* go = new QPushButton(QStringLiteral("Save merged PDF"), this);
    go->setProperty("primary", true);
    row->addWidget(addBtn);
    row->addWidget(rot);
    row->addWidget(del);
    row->addWidget(clear);
    row->addStretch();
    row->addWidget(go);

    connect(addBtn, &QPushButton::clicked, this, [this] {
        const auto files = QFileDialog::getOpenFileNames(
            this, QStringLiteral("Add PDFs"), AppSettings::instance().lastDirectory(),
            QStringLiteral("PDF files (*.pdf)"));
        addFiles(files);
    });
    connect(rot, &QPushButton::clicked, this, [this] {
        grid_->applyVisualOrder();
        for (int r : grid_->selectedRows()) {
            assembly_.rotate(r, 90);
        }
        grid_->refresh();
    });
    connect(del, &QPushButton::clicked, this, [this] {
        grid_->applyVisualOrder();
        auto rows = grid_->selectedRows();
        for (int i = rows.size() - 1; i >= 0; --i) {
            assembly_.remove(rows[i]);
        }
        grid_->refresh();
    });
    connect(clear, &QPushButton::clicked, this, [this] {
        assembly_.clear();
        grid_->refresh();
    });
    connect(go, &QPushButton::clicked, this, &MergeView::doMerge);

    root->addWidget(title);
    root->addWidget(hint);
    root->addWidget(drop_);
    root->addWidget(grid_, 1);
    root->addWidget(banner_);
    root->addLayout(row);
}

void MergeView::addFiles(const QStringList& paths) {
    for (const auto& p : paths) {
        if (!p.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive)) {
            continue;
        }
        auto r = assembly_.addDocument(std::filesystem::path(p.toStdString()));
        if (!r) {
            banner_->showError(QString::fromStdString(r.error()));
        } else {
            AppSettings::instance().addRecentFile(p);
            AppSettings::instance().setLastDirectory(QFileInfo(p).absolutePath());
        }
    }
    grid_->refresh();
}

void MergeView::doMerge() {
    grid_->applyVisualOrder();
    if (assembly_.includedCount() == 0) {
        banner_->showError(QStringLiteral("Add at least one PDF first."));
        return;
    }
    const QString out = QFileDialog::getSaveFileName(
        this, QStringLiteral("Save merged PDF"),
        AppSettings::instance().lastDirectory() + QStringLiteral("/merged.pdf"),
        QStringLiteral("PDF files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }
    banner_->showProgress(QStringLiteral("Merging…"), 0, 1);
    const auto copy = assembly_;
    const auto path = std::filesystem::path(out.toStdString());
    QPointer<MergeView> self(this);
    (void)QtConcurrent::run([self, copy, path, out]() mutable {
        auto result = copy.write(path);
        if (!self) {
            return;
        }
        QMetaObject::invokeMethod(self.data(), [self, result, out]() {

            if (!self) {
                return;
            }
            if (!result) {
                self->banner_->showError(QString::fromStdString(result.error()));
            } else {
                self->banner_->showInfo(QStringLiteral("Saved ") + out);
                AppSettings::instance().addRecentFile(out);
                emit self->exported(out);
            }
        });
    });
}

} // namespace drpdf

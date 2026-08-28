#include "organize_view.h"

#include "app/settings.h"
#include "ui/components/banner.h"
#include "ui/components/drop_zone.h"
#include "ui/components/page_grid.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QtConcurrent>
#include <QVBoxLayout>

namespace drpdf {

OrganizeView::OrganizeView(ThumbnailCache* cache, QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    auto* title = new QLabel(QStringLiteral("Organize pages"), this);
    QFont f = title->font();
    f.setPixelSize(22);
    f.setWeight(QFont::DemiBold);
    title->setFont(f);
    auto* hint = new QLabel(QStringLiteral("Drag to reorder. Rotate or delete, then Save as — original stays untouched."),
                            this);
    hint->setProperty("muted", true);

    drop_ = new DropZone(this);
    drop_->setMaximumHeight(120);
    drop_->setTitle(QStringLiteral("Drop a PDF to organize"));
    connect(drop_, &DropZone::filesDropped, this, [this](const QStringList& p) {
        if (!p.isEmpty()) {
            loadFile(p.first());
        }
    });
    grid_ = new PageGrid(cache, this);
    grid_->setAssembly(&assembly_);
    banner_ = new Banner(this);

    auto* row = new QHBoxLayout();
    auto* rot = new QPushButton(QStringLiteral("Rotate 90°"), this);
    auto* rot180 = new QPushButton(QStringLiteral("Rotate 180°"), this);
    auto* del = new QPushButton(QStringLiteral("Delete pages"), this);
    del->setProperty("danger", true);
    auto* save = new QPushButton(QStringLiteral("Save as…"), this);
    save->setProperty("primary", true);
    row->addWidget(rot);
    row->addWidget(rot180);
    row->addWidget(del);
    row->addStretch();
    row->addWidget(save);

    connect(rot, &QPushButton::clicked, this, [this] {
        grid_->applyVisualOrder();
        for (int r : grid_->selectedRows()) {
            assembly_.rotate(r, 90);
        }
        grid_->refresh();
    });
    connect(rot180, &QPushButton::clicked, this, [this] {
        grid_->applyVisualOrder();
        for (int r : grid_->selectedRows()) {
            assembly_.rotate(r, 180);
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
    connect(save, &QPushButton::clicked, this, &OrganizeView::saveAs);

    root->addWidget(title);
    root->addWidget(hint);
    root->addWidget(drop_);
    root->addWidget(grid_, 1);
    root->addWidget(banner_);
    root->addLayout(row);
}

void OrganizeView::loadFile(const QString& path) {
    assembly_.clear();
    auto r = assembly_.addDocument(std::filesystem::path(path.toStdString()));
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
        return;
    }
    AppSettings::instance().addRecentFile(path);
    grid_->refresh();
}

void OrganizeView::saveAs() {
    grid_->applyVisualOrder();
    const QString out = QFileDialog::getSaveFileName(
        this, QStringLiteral("Save organized PDF"),
        AppSettings::instance().lastDirectory() + QStringLiteral("/organized.pdf"),
        QStringLiteral("PDF files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }
    auto copy = assembly_;
    QPointer<OrganizeView> self(this);
    (void)QtConcurrent::run([self, copy, out]() mutable {
        auto r = copy.write(std::filesystem::path(out.toStdString()));
        if (!self) {
            return;
        }
        QMetaObject::invokeMethod(self.data(), [self, r, out]() {

            if (!self) {
                return;
            }
            if (!r) {
                self->banner_->showError(QString::fromStdString(r.error()));
            } else {
                self->banner_->showInfo(QStringLiteral("Saved ") + out);
                emit self->exported(out);
            }
        });
    });
}

} // namespace drpdf

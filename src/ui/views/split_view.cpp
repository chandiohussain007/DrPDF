#include "split_view.h"

#include "app/settings.h"
#include "core/ranges.h"
#include "ui/components/banner.h"
#include "ui/components/drop_zone.h"
#include "ui/components/page_grid.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QSpinBox>
#include <QtConcurrent>
#include <QVBoxLayout>


namespace drpdf {

SplitView::SplitView(ThumbnailCache* cache, QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    root->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("Split / Extract"), this);
    QFont f = title->font();
    f.setPixelSize(22);
    f.setWeight(QFont::DemiBold);
    title->setFont(f);

    drop_ = new DropZone(this);
    drop_->setTitle(QStringLiteral("Drop a PDF to split"));
    drop_->setMaximumHeight(120);
    connect(drop_, &DropZone::filesDropped, this, [this](const QStringList& p) {
        if (!p.isEmpty()) {
            loadFile(p.first());
        }
    });

    auto* opts = new QHBoxLayout();
    mode_ = new QComboBox(this);
    mode_->addItems({QStringLiteral("Extract selected pages"), QStringLiteral("Page ranges"),
                     QStringLiteral("Every N pages")});
    ranges_ = new QLineEdit(this);
    ranges_->setPlaceholderText(QStringLiteral("e.g. 1-3,5,8-z"));
    everyN_ = new QSpinBox(this);
    everyN_->setRange(1, 999);
    everyN_->setValue(1);
    everyN_->setPrefix(QStringLiteral("N = "));
    auto* apply = new QPushButton(QStringLiteral("Apply"), this);
    opts->addWidget(new QLabel(QStringLiteral("Mode"), this));
    opts->addWidget(mode_, 1);
    opts->addWidget(ranges_, 1);
    opts->addWidget(everyN_);
    opts->addWidget(apply);
    connect(apply, &QPushButton::clicked, this, &SplitView::applyMode);

    grid_ = new PageGrid(cache, this);
    grid_->setAssembly(&assembly_);
    banner_ = new Banner(this);

    auto* go = new QPushButton(QStringLiteral("Export"), this);
    go->setProperty("primary", true);
    connect(go, &QPushButton::clicked, this, &SplitView::doExport);

    auto* row = new QHBoxLayout();
    row->addStretch();
    row->addWidget(go);

    root->addWidget(title);
    root->addWidget(drop_);
    root->addLayout(opts);
    root->addWidget(grid_, 1);
    root->addWidget(banner_);
    root->addLayout(row);
}

void SplitView::loadFile(const QString& path) {
    assembly_.clear();
    source_ = path;
    auto r = assembly_.addDocument(std::filesystem::path(path.toStdString()));
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
        return;
    }
    AppSettings::instance().addRecentFile(path);
    grid_->refresh();
    banner_->showInfo(QStringLiteral("%1 pages — choose a split mode.").arg(r.value().pageCount));
}

void SplitView::applyMode() {
    if (assembly_.count() == 0) {
        return;
    }
    const int mode = mode_->currentIndex();
    if (mode == 0) {
        const auto sel = grid_->selectedRows();
        for (int i = 0; i < assembly_.count(); ++i) {
            assembly_.setIncluded(i, sel.contains(i));
        }
        if (sel.isEmpty()) {
            banner_->showError(QStringLiteral("Select pages in the grid first."));
        }
    } else if (mode == 1) {
        auto parsed = core::parsePageRanges(ranges_->text().toStdString(), assembly_.count());
        if (!parsed) {
            banner_->showError(QString::fromStdString(parsed.error()));
            return;
        }
        for (int i = 0; i < assembly_.count(); ++i) {
            assembly_.setIncluded(i, false);
        }
        for (int idx : parsed.value()) {
            assembly_.setIncluded(idx, true);
        }
    } else {
        banner_->showInfo(QStringLiteral("Every N pages exports multiple files on Export."));
    }
    grid_->refresh();
}

void SplitView::doExport() {
    if (assembly_.count() == 0) {
        banner_->showError(QStringLiteral("Load a PDF first."));
        return;
    }
    grid_->applyVisualOrder();
    if (mode_->currentIndex() == 2) {
        const int n = everyN_->value();
        const QString dir = QFileDialog::getExistingDirectory(
            this, QStringLiteral("Choose output folder"), AppSettings::instance().lastDirectory());
        if (dir.isEmpty()) {
            return;
        }
        int part = 1;
        for (int start = 0; start < assembly_.count(); start += n, ++part) {
            core::Assembly chunk;
            chunk.addDocument(std::filesystem::path(source_.toStdString()));
            for (int i = 0; i < chunk.count(); ++i) {
                chunk.setIncluded(i, i >= start && i < start + n);
            }
            const QString out =
                QStringLiteral("%1/split-part-%2.pdf").arg(dir).arg(part, 2, 10, QChar('0'));
            auto r = chunk.write(std::filesystem::path(out.toStdString()));
            if (!r) {
                banner_->showError(QString::fromStdString(r.error()));
                return;
            }
        }
        banner_->showInfo(QStringLiteral("Wrote split files to ") + dir);
        return;
    }

    const QString out = QFileDialog::getSaveFileName(
        this, QStringLiteral("Export pages"),
        AppSettings::instance().lastDirectory() + QStringLiteral("/extracted.pdf"),
        QStringLiteral("PDF files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }
    auto copy = assembly_;
    QPointer<SplitView> self(this);
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

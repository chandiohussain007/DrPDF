#include "compress_view.h"

#include "app/settings.h"
#include "core/assembly.h"
#include "ui/components/banner.h"
#include "ui/components/drop_zone.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace drpdf {

CompressView::CompressView(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    auto* title = new QLabel(QStringLiteral("Compress"), this);
    QFont f = title->font();
    f.setPixelSize(22);
    f.setWeight(QFont::DemiBold);
    title->setFont(f);
    auto* hint = new QLabel(
        QStringLiteral("Milestone 1 ships lossless stream/object-stream optimization (QPDF). "
                       "Image downsampling (low/medium/high fidelity) lands in Milestone 4."),
        this);
    hint->setWordWrap(true);
    hint->setProperty("muted", true);

    drop_ = new DropZone(this);
    drop_->setTitle(QStringLiteral("Drop a PDF to compress"));
    drop_->setMaximumHeight(140);
    connect(drop_, &DropZone::filesDropped, this, [this](const QStringList& p) {
        if (!p.isEmpty()) {
            loadFile(p.first());
        }
    });

    tier_ = new QComboBox(this);
    tier_->addItem(QStringLiteral("High fidelity — lossless streams (available now)"));
    tier_->addItem(QStringLiteral("Medium — image downsample (Milestone 4)"));
    tier_->addItem(QStringLiteral("Low — aggressive downsample (Milestone 4)"));

    banner_ = new Banner(this);
    auto* go = new QPushButton(QStringLiteral("Optimize PDF"), this);
    go->setProperty("primary", true);
    connect(go, &QPushButton::clicked, this, &CompressView::run);

    auto* row = new QHBoxLayout();
    row->addStretch();
    row->addWidget(go);

    root->addWidget(title);
    root->addWidget(hint);
    root->addWidget(drop_);
    root->addWidget(tier_);
    root->addWidget(banner_);
    root->addLayout(row);
    root->addStretch();
}

void CompressView::loadFile(const QString& path) {
    path_ = path;
    auto info = core::inspectPdf(std::filesystem::path(path.toStdString()));
    if (!info) {
        banner_->showError(QString::fromStdString(info.error()));
        return;
    }
    const double mb = static_cast<double>(info.value().bytes) / (1024.0 * 1024.0);
    banner_->showInfo(QStringLiteral("%1 — %2 pages, %3 MB")
                          .arg(QFileInfo(path).fileName())
                          .arg(info.value().pageCount)
                          .arg(mb, 0, 'f', 2));
}

void CompressView::run() {
    if (path_.isEmpty()) {
        banner_->showError(QStringLiteral("Drop a PDF first."));
        return;
    }
    if (tier_->currentIndex() != 0) {
        banner_->showError(QStringLiteral("Image downsampling is Milestone 4. Use High fidelity for now."));
        return;
    }
    const QString out = QFileDialog::getSaveFileName(
        this, QStringLiteral("Save optimized PDF"),
        AppSettings::instance().lastDirectory() + QStringLiteral("/optimized.pdf"),
        QStringLiteral("PDF files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }
    auto r = core::optimizePdf(std::filesystem::path(path_.toStdString()),
                               std::filesystem::path(out.toStdString()));
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
        return;
    }
    std::error_code ec;
    const auto inB = std::filesystem::file_size(std::filesystem::path(path_.toStdString()), ec);
    const auto outB = std::filesystem::file_size(std::filesystem::path(out.toStdString()), ec);
    banner_->showInfo(QStringLiteral("Saved %1  (%2 KB → %3 KB)")
                          .arg(out)
                          .arg(inB / 1024)
                          .arg(outB / 1024));
    emit exported(out);
}

} // namespace drpdf

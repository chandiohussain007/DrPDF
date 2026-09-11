#include "ocr_view.h"

#include "app/settings.h"
#include "services/ocr_engine.h"
#include "ui/components/banner.h"
#include "ui/components/drop_zone.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QtConcurrent>
#include <QVBoxLayout>

namespace drpdf {

OcrView::OcrView(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    auto* title = new QLabel(QStringLiteral("OCR"), this);
    QFont f = title->font();
    f.setPixelSize(22);
    f.setWeight(QFont::DemiBold);
    title->setFont(f);
    auto* hint = new QLabel(
        QStringLiteral("Runs Tesseract on this machine and overlays invisible text so the PDF "
                       "becomes searchable. No cloud. Install tesseract-ocr plus a language pack."),
        this);
    hint->setWordWrap(true);
    hint->setProperty("muted", true);

    drop_ = new DropZone(this);
    drop_->setTitle(QStringLiteral("Drop a scanned PDF"));
    drop_->setMaximumHeight(130);
    connect(drop_, &DropZone::filesDropped, this, [this](const QStringList& p) {
        if (!p.isEmpty()) {
            loadFile(p.first());
        }
    });

    lang_ = new QComboBox(this);
    const auto langs = tesseractLanguages();
    for (const auto& l : langs) {
        lang_->addItem(l);
    }
    if (lang_->findText(QStringLiteral("eng")) >= 0) {
        lang_->setCurrentText(QStringLiteral("eng"));
    }
    sidecar_ = new QCheckBox(QStringLiteral("Also write a sidecar .txt"), this);
    progress_ = new QProgressBar(this);
    progress_->setRange(0, 1);
    progress_->setValue(0);
    progress_->setTextVisible(true);

    banner_ = new Banner(this);
    const QString exe = findTesseract();
    if (exe.isEmpty()) {
        banner_->showError(QStringLiteral(
            "Tesseract not found. brew/apt install tesseract, or set TESSERACT_PATH."));
    } else {
        banner_->showInfo(QStringLiteral("Using ") + exe);
    }

    auto* go = new QPushButton(QStringLiteral("Make searchable"), this);
    go->setProperty("primary", true);
    connect(go, &QPushButton::clicked, this, &OcrView::run);

    auto* row = new QHBoxLayout();
    row->addWidget(new QLabel(QStringLiteral("Language"), this));
    row->addWidget(lang_);
    row->addWidget(sidecar_);
    row->addStretch();
    row->addWidget(go);

    root->addWidget(title);
    root->addWidget(hint);
    root->addWidget(drop_);
    root->addLayout(row);
    root->addWidget(progress_);
    root->addWidget(banner_);
    root->addStretch();
}

void OcrView::loadFile(const QString& path) {
    path_ = path;
    banner_->showInfo(QFileInfo(path).fileName());
    AppSettings::instance().addRecentFile(path);
}

void OcrView::run() {
    if (path_.isEmpty()) {
        banner_->showError(QStringLiteral("Drop a PDF first."));
        return;
    }
    const QString out = QFileDialog::getSaveFileName(
        this, QStringLiteral("Save searchable PDF"),
        AppSettings::instance().lastDirectory() + QStringLiteral("/searchable.pdf"),
        QStringLiteral("PDF files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }
    OcrOptions opt;
    opt.language = lang_->currentText();
    opt.sidecarTxt = sidecar_->isChecked();
    progress_->setRange(0, 0);
    banner_->showInfo(QStringLiteral("OCR running locally…"));

    auto* watcher = new QFutureWatcher<core::Result<void>>(this);
    connect(watcher, &QFutureWatcher<core::Result<void>>::finished, this, [this, watcher, out] {
        progress_->setRange(0, 1);
        progress_->setValue(1);
        auto r = watcher->result();
        watcher->deleteLater();
        if (!r) {
            banner_->showError(QString::fromStdString(r.error()));
            return;
        }
        banner_->showInfo(QStringLiteral("Saved searchable PDF ") + out);
        emit exported(out);
    });
    const QString input = path_;
    watcher->setFuture(QtConcurrent::run([input, out, opt] {
        return ocrPdf(input, out, opt, {});
    }));
}

} // namespace drpdf

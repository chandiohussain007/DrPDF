#include "watermark_view.h"

#include "app/settings.h"
#include "core/overlay.h"
#include "ui/components/banner.h"
#include "ui/components/drop_zone.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>


namespace drpdf {

WatermarkView::WatermarkView(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    auto* title = new QLabel(QStringLiteral("Watermark & page marks"), this);
    QFont f = title->font();
    f.setPixelSize(22);
    f.setWeight(QFont::DemiBold);
    title->setFont(f);
    auto* hint = new QLabel(
        QStringLiteral("Applies across pages as extra content streams. Original file is never overwritten."),
        this);
    hint->setWordWrap(true);
    hint->setProperty("muted", true);

    drop_ = new DropZone(this);
    drop_->setTitle(QStringLiteral("Drop a PDF"));
    drop_->setMaximumHeight(120);
    connect(drop_, &DropZone::filesDropped, this, [this](const QStringList& p) {
        if (!p.isEmpty()) {
            loadFile(p.first());
        }
    });

    mark_ = new QLineEdit(QStringLiteral("CONFIDENTIAL"), this);
    size_ = new QDoubleSpinBox(this);
    size_->setRange(8, 200);
    size_->setValue(54);
    size_->setSuffix(QStringLiteral(" pt"));
    angle_ = new QDoubleSpinBox(this);
    angle_->setRange(-180, 180);
    angle_->setValue(45);
    angle_->setSuffix(QStringLiteral("°"));
    opacity_ = new QDoubleSpinBox(this);
    opacity_->setRange(0.05, 1.0);
    opacity_->setSingleStep(0.05);
    opacity_->setValue(0.16);
    tiled_ = new QCheckBox(QStringLiteral("Tile across the page"), this);

    header_ = new QLineEdit(this);
    header_->setPlaceholderText(QStringLiteral("Header (optional)"));
    footer_ = new QLineEdit(this);
    footer_->setPlaceholderText(QStringLiteral("Footer (optional)"));
    pageFmt_ = new QLineEdit(QStringLiteral("Page {n} of {total}"), this);
    pagePos_ = new QComboBox(this);
    pagePos_->addItem(QStringLiteral("Bottom center"), static_cast<int>(core::PageMarkPos::BottomCenter));
    pagePos_->addItem(QStringLiteral("Bottom left"), static_cast<int>(core::PageMarkPos::BottomLeft));
    pagePos_->addItem(QStringLiteral("Bottom right"), static_cast<int>(core::PageMarkPos::BottomRight));
    pagePos_->addItem(QStringLiteral("Top center"), static_cast<int>(core::PageMarkPos::TopCenter));
    pagePos_->addItem(QStringLiteral("Top left"), static_cast<int>(core::PageMarkPos::TopLeft));
    pagePos_->addItem(QStringLiteral("Top right"), static_cast<int>(core::PageMarkPos::TopRight));

    auto* form = new QFormLayout();
    form->addRow(QStringLiteral("Watermark"), mark_);
    form->addRow(QStringLiteral("Size"), size_);
    form->addRow(QStringLiteral("Angle"), angle_);
    form->addRow(QStringLiteral("Opacity"), opacity_);
    form->addRow(QString(), tiled_);
    form->addRow(QStringLiteral("Header"), header_);
    form->addRow(QStringLiteral("Footer"), footer_);
    form->addRow(QStringLiteral("Page numbers"), pageFmt_);
    form->addRow(QStringLiteral("Number position"), pagePos_);

    banner_ = new Banner(this);
    auto* go = new QPushButton(QStringLiteral("Apply & save as…"), this);
    go->setProperty("primary", true);
    connect(go, &QPushButton::clicked, this, &WatermarkView::apply);

    auto* row = new QHBoxLayout();
    row->addStretch();
    row->addWidget(go);

    root->addWidget(title);
    root->addWidget(hint);
    root->addWidget(drop_);
    root->addLayout(form);
    root->addWidget(banner_);
    root->addLayout(row);
    root->addStretch();
}

void WatermarkView::loadFile(const QString& path) {
    path_ = path;
    banner_->showInfo(QFileInfo(path).fileName());
    AppSettings::instance().addRecentFile(path);
}

void WatermarkView::apply() {
    if (path_.isEmpty()) {
        banner_->showError(QStringLiteral("Drop a PDF first."));
        return;
    }
    const QString out = QFileDialog::getSaveFileName(
        this, QStringLiteral("Save stamped PDF"),
        AppSettings::instance().lastDirectory() + QStringLiteral("/stamped.pdf"),
        QStringLiteral("PDF files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }

    QString mid = out + QStringLiteral(".tmp.pdf");
    QString current = path_;
    if (!mark_->text().trimmed().isEmpty()) {
        core::WatermarkOptions wm;
        wm.text = mark_->text().toStdString();
        wm.fontSize = size_->value();
        wm.rotationDeg = angle_->value();
        wm.opacity = opacity_->value();
        wm.tiled = tiled_->isChecked();
        auto r = core::applyWatermark(std::filesystem::path(current.toStdString()),
                                      std::filesystem::path(mid.toStdString()), wm);
        if (!r) {
            banner_->showError(QString::fromStdString(r.error()));
            QFile::remove(mid);
            return;
        }
        current = mid;
    }

    core::HeaderFooterOptions hf;
    hf.header = header_->text().toStdString();
    hf.footer = footer_->text().toStdString();
    hf.pageNumberFormat = pageFmt_->text().toStdString();
    hf.pageNumberPos = static_cast<core::PageMarkPos>(pagePos_->currentData().toInt());
    const bool needHf =
        !header_->text().isEmpty() || !footer_->text().isEmpty() || !pageFmt_->text().isEmpty();
    if (needHf) {
        const QString next = out;
        auto r = core::applyHeaderFooter(std::filesystem::path(current.toStdString()),
                                         std::filesystem::path(next.toStdString()), hf);
        if (current == mid) {
            QFile::remove(mid);
        }
        if (!r) {
            banner_->showError(QString::fromStdString(r.error()));
            return;
        }
    } else if (current == mid) {
        if (QFile::exists(out)) {
            QFile::remove(out);
        }
        QFile::rename(mid, out);
    } else {
        banner_->showError(QStringLiteral("Enter a watermark, header, footer, or page-number format."));
        return;
    }
    emit exported(out);
    banner_->showInfo(QStringLiteral("Saved ") + out);
}

} // namespace drpdf

#include "viewer_view.h"

#include "app/settings.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPdfDocument>
#include <QPdfView>
#include <QPushButton>
#include <QVBoxLayout>

namespace drpdf {

ViewerView::ViewerView(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 12, 16, 12);
    auto* top = new QHBoxLayout();
    auto* title = new QLabel(QStringLiteral("Viewer"), this);
    QFont f = title->font();
    f.setPixelSize(22);
    f.setWeight(QFont::DemiBold);
    title->setFont(f);
    meta_ = new QLabel(this);
    meta_->setProperty("muted", true);
    top->addWidget(title);
    top->addStretch();
    top->addWidget(meta_);

    doc_ = new QPdfDocument(this);
    view_ = new QPdfView(this);
    view_->setDocument(doc_);
    view_->setPageMode(QPdfView::MultiPage);
    view_->setZoomMode(QPdfView::FitInView);


    root->addLayout(top);
    root->addWidget(view_, 1);
}

void ViewerView::openFile(const QString& path) {
    const auto err = doc_->load(path);
    if (err != QPdfDocument::Error::None) {
        meta_->setText(QStringLiteral("Could not open file"));
        return;
    }
    AppSettings::instance().addRecentFile(path);
    meta_->setText(QStringLiteral("%1 pages").arg(doc_->pageCount()));
}

} // namespace drpdf

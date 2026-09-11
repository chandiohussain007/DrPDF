#include "viewer_view.h"

#include "app/settings.h"
#include "services/thumbnails.h"
#include "ui/icons/icons.h"
#include "ui/theme/theme.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>

#include <QLabel>
#include <QListWidget>
#include <QPdfDocument>
#include <QPdfPageNavigator>
#include <QPdfView>
#include <QPixmap>
#include <QPointF>

#include <QPushButton>
#include <QShortcut>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>

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

    auto* bar = new QHBoxLayout();
    auto* minus = new QPushButton(QStringLiteral("−"), this);
    auto* plus = new QPushButton(QStringLiteral("+"), this);
    auto* fitW = new QPushButton(QStringLiteral("Fit width"), this);
    auto* fitPage = new QPushButton(QStringLiteral("Fit page"), this);
    auto* actual = new QPushButton(QStringLiteral("Actual size"), this);
    auto* prev = new QPushButton(QStringLiteral("Prev"), this);
    auto* next = new QPushButton(QStringLiteral("Next"), this);
    zoomLabel_ = new QLabel(QStringLiteral("100%"), this);
    zoomLabel_->setProperty("muted", true);
    minus->setAccessibleName(QStringLiteral("Zoom out"));
    plus->setAccessibleName(QStringLiteral("Zoom in"));
    fitW->setAccessibleName(QStringLiteral("Fit to width"));
    fitPage->setAccessibleName(QStringLiteral("Fit page"));
    actual->setAccessibleName(QStringLiteral("Actual size"));
    prev->setAccessibleName(QStringLiteral("Previous page"));
    next->setAccessibleName(QStringLiteral("Next page"));
    minus->setFixedWidth(36);
    plus->setFixedWidth(36);
    minus->setToolTip(QStringLiteral("Zoom out (Ctrl+-)"));
    plus->setToolTip(QStringLiteral("Zoom in (Ctrl++)"));
    actual->setToolTip(QStringLiteral("Actual size (Ctrl+0)"));
    fitW->setToolTip(QStringLiteral("Fit to width (Ctrl+1)"));
    bar->addWidget(prev);
    bar->addWidget(next);
    bar->addSpacing(12);
    bar->addWidget(minus);
    bar->addWidget(zoomLabel_);
    bar->addWidget(plus);
    bar->addWidget(fitW);
    bar->addWidget(fitPage);
    bar->addWidget(actual);
    bar->addStretch();

    collapseThumbs_ = new QToolButton(this);
    collapseThumbs_->setText(QStringLiteral("Pages"));
    collapseThumbs_->setCheckable(true);
    collapseThumbs_->setChecked(true);
    collapseThumbs_->setAccessibleName(QStringLiteral("Toggle page thumbnails"));
    bar->addWidget(collapseThumbs_);

    doc_ = new QPdfDocument(this);
    view_ = new QPdfView(this);
    view_->setDocument(doc_);
    view_->setPageMode(QPdfView::PageMode::MultiPage);
    view_->setZoomMode(QPdfView::ZoomMode::FitInView);
    view_->setAccessibleName(QStringLiteral("PDF pages"));

    thumbs_ = new QListWidget(this);
    thumbs_->setObjectName(QStringLiteral("ThumbStrip"));
    thumbs_->setFixedWidth(132);
    thumbs_->setIconSize(QSize(96, 128));
    thumbs_->setSpacing(8);
    thumbs_->setMovement(QListView::Static);
    thumbs_->setAccessibleName(QStringLiteral("Page thumbnails"));
    thumbs_->setSelectionMode(QAbstractItemView::SingleSelection);

    auto* body = new QHBoxLayout();
    body->setSpacing(8);
    body->addWidget(thumbs_);
    body->addWidget(view_, 1);

    connect(collapseThumbs_, &QToolButton::toggled, this, [this](bool on) { thumbs_->setVisible(on); });
    connect(minus, &QPushButton::clicked, this, [this] { zoomBy(1.0 / 1.25); });
    connect(plus, &QPushButton::clicked, this, [this] { zoomBy(1.25); });
    connect(fitW, &QPushButton::clicked, this, [this] {
        view_->setZoomMode(QPdfView::ZoomMode::FitToWidth);
        updateZoomLabel();
    });
    connect(fitPage, &QPushButton::clicked, this, [this] {
        view_->setZoomMode(QPdfView::ZoomMode::FitInView);
        updateZoomLabel();
    });
    connect(actual, &QPushButton::clicked, this, [this] { setCustomZoom(1.0); });
    connect(prev, &QPushButton::clicked, this, [this] {
        if (auto* nav = view_->pageNavigator()) {
            jumpTo(std::max(0, nav->currentPage() - 1));
        }
    });
    connect(next, &QPushButton::clicked, this, [this] {
        if (auto* nav = view_->pageNavigator()) {
            jumpTo(std::min(std::max(0, doc_->pageCount() - 1), nav->currentPage() + 1));
        }
    });
    connect(thumbs_, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0) {
            jumpTo(row);
        }
    });

    auto* scIn = new QShortcut(QKeySequence::ZoomIn, this);
    connect(scIn, &QShortcut::activated, this, [this] { zoomBy(1.25); });
    auto* scOut = new QShortcut(QKeySequence::ZoomOut, this);
    connect(scOut, &QShortcut::activated, this, [this] { zoomBy(1.0 / 1.25); });
    auto* sc0 = new QShortcut(QKeySequence(QStringLiteral("Ctrl+0")), this);
    connect(sc0, &QShortcut::activated, this, [this] { setCustomZoom(1.0); });
    auto* sc1 = new QShortcut(QKeySequence(QStringLiteral("Ctrl+1")), this);
    connect(sc1, &QShortcut::activated, this, [this] {
        view_->setZoomMode(QPdfView::ZoomMode::FitToWidth);
        updateZoomLabel();
    });

    root->addLayout(top);
    root->addLayout(bar);
    root->addLayout(body, 1);
}

void ViewerView::setThumbnailCache(ThumbnailCache* cache) {
    cache_ = cache;
    if (cache_) {
        connect(cache_, &ThumbnailCache::ready, this, [this](const QString& path, int page, const QImage& img) {
            if (path != path_ || img.isNull() || page < 0 || page >= thumbs_->count()) {
                return;
            }
            thumbs_->item(page)->setIcon(QIcon(QPixmap::fromImage(img)));
        });
    }
}

void ViewerView::setCustomZoom(qreal factor) {
    factor = std::clamp(factor, 0.25, 6.0);
    view_->setZoomMode(QPdfView::ZoomMode::Custom);
    view_->setZoomFactor(factor);
    updateZoomLabel();
}

void ViewerView::zoomBy(qreal mul) {
    qreal z = view_->zoomFactor();
    if (z <= 0) {
        z = 1.0;
    }
    setCustomZoom(z * mul);
}

void ViewerView::updateZoomLabel() {
    zoomLabel_->setText(QStringLiteral("%1%").arg(int(view_->zoomFactor() * 100)));
}

void ViewerView::jumpTo(int page) {
    if (auto* nav = view_->pageNavigator()) {
        nav->jump(page, QPointF(), nav->currentZoom());
    }
    if (thumbs_->currentRow() != page && page >= 0 && page < thumbs_->count()) {
        thumbs_->blockSignals(true);
        thumbs_->setCurrentRow(page);
        thumbs_->blockSignals(false);
    }
}

void ViewerView::rebuildThumbs() {
    thumbs_->clear();
    if (path_.isEmpty() || doc_->pageCount() <= 0) {
        return;
    }
    const auto& t = Theme::instance().tokens();
    for (int i = 0; i < doc_->pageCount(); ++i) {
        auto* it = new QListWidgetItem(QStringLiteral("%1").arg(i + 1), thumbs_);
        it->setTextAlignment(Qt::AlignHCenter);
        QPixmap ph(96, 128);
        ph.fill(t.surface2);
        if (cache_) {
            QImage img = cache_->get(path_, i, QSize(96, 128));
            if (img.isNull()) {
                cache_->request(path_, i, QSize(96, 128));
            } else {
                ph = QPixmap::fromImage(img);
            }
        }
        it->setIcon(QIcon(ph));
        it->setToolTip(QStringLiteral("Page %1").arg(i + 1));
    }
}

void ViewerView::openFile(const QString& path) {
    const auto err = doc_->load(path);
    if (err != QPdfDocument::Error::None) {
        meta_->setText(QStringLiteral("Could not open file"));
        return;
    }
    path_ = path;
    AppSettings::instance().addRecentFile(path);
    meta_->setText(QStringLiteral("%1 pages").arg(doc_->pageCount()));
    view_->setZoomMode(QPdfView::ZoomMode::FitInView);
    updateZoomLabel();
    rebuildThumbs();
}

} // namespace drpdf

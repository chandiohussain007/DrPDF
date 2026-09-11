#include "page_grid.h"

#include "services/thumbnails.h"
#include "ui/theme/theme.h"

#include <QDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QStyledItemDelegate>
#include <QTransform>
#include <QVBoxLayout>


#include <algorithm>

namespace drpdf {
namespace {

constexpr int kOverlayCount = 3;

QRect overlayStrip(const QRect& item) {
    return QRect(item.left() + 10, item.bottom() - 36, item.width() - 20, 26);
}

QRect overlayBtn(const QRect& item, int which) {
    const QRect strip = overlayStrip(item);
    const int w = (strip.width() - 8) / kOverlayCount;
    return QRect(strip.left() + which * (w + 4), strip.top(), w, strip.height());
}

class ThumbDelegate : public QStyledItemDelegate {
public:
    explicit ThumbDelegate(PageGrid* grid) : QStyledItemDelegate(grid), grid_(grid) {}

    void paint(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const override {
        const auto& t = Theme::instance().tokens();
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        QRect r = opt.rect.adjusted(4, 4, -4, -4);
        QPainterPath path;
        path.addRoundedRect(r, 12, 12);
        const bool selected = opt.state & QStyle::State_Selected;
        const bool hover = opt.state & QStyle::State_MouseOver;
        if (hover) {
            p->fillPath(path.translated(0, 1), t.shadow);
        }
        p->fillPath(path, hover || selected ? t.surfaceHover : t.surface);
        p->setPen(QPen(selected ? t.accent : (hover ? t.gold : t.border), selected ? 1.6 : 1));
        p->drawPath(path);

        const QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect thumb(r.left() + 12, r.top() + 10, r.width() - 24, r.height() - 58);
        icon.paint(p, thumb, Qt::AlignCenter);

        p->setPen(t.text);
        QFont f = opt.font;
        f.setPixelSize(11);
        p->setFont(f);
        p->drawText(QRect(r.left() + 8, r.bottom() - 44, r.width() - 16, 16),
                    Qt::AlignHCenter | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());

        if (hover || selected) {
            const char* labels[kOverlayCount] = {"Rot", "Del", "Zoom"};
            for (int i = 0; i < kOverlayCount; ++i) {
                const QRect b = overlayBtn(r, i);
                QPainterPath bp;
                bp.addRoundedRect(b, 8, 8);
                p->fillPath(bp, i == 1 ? t.danger : t.surface2);
                p->setPen(i == 1 ? QColor("#0B0F17") : t.text);
                p->drawText(b, Qt::AlignCenter, QString::fromLatin1(labels[i]));
            }
        }
        p->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const override {
        return QSize(156, 220);
    }

private:
    PageGrid* grid_ = nullptr;
};

} // namespace

PageGrid::PageGrid(ThumbnailCache* cache, QWidget* parent)
    : QListWidget(parent), cache_(cache) {
    setViewMode(QListView::IconMode);
    setIconSize(QSize(120, 160));
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Snap);
    setDragDropMode(InternalMove);
    setSelectionMode(ExtendedSelection);
    setSpacing(10);
    setUniformItemSizes(true);
    setMouseTracking(true);
    setItemDelegate(new ThumbDelegate(this));
    setAccessibleName(QStringLiteral("Page thumbnails"));
    setAccessibleDescription(
        QStringLiteral("Drag to reorder. Hover a card for rotate, delete, or zoom."));

    fab_ = new QWidget(this);
    fab_->setObjectName(QStringLiteral("Fab"));
    fab_->setVisible(false);
    auto* fabLay = new QHBoxLayout(fab_);
    fabLay->setContentsMargins(10, 8, 10, 8);
    fabLay->setSpacing(8);
    auto* rot = new QPushButton(QStringLiteral("Rotate"), fab_);
    auto* del = new QPushButton(QStringLiteral("Delete"), fab_);
    del->setProperty("danger", true);
    auto* zoom = new QPushButton(QStringLiteral("Zoom"), fab_);
    rot->setAccessibleName(QStringLiteral("Rotate selected pages"));
    del->setAccessibleName(QStringLiteral("Delete selected pages"));
    zoom->setAccessibleName(QStringLiteral("Zoom selected page"));
    fabLay->addWidget(rot);
    fabLay->addWidget(del);
    fabLay->addWidget(zoom);
    connect(rot, &QPushButton::clicked, this, [this] { emit rotateRequested(90); });
    connect(del, &QPushButton::clicked, this, &PageGrid::deleteRequested);
    connect(zoom, &QPushButton::clicked, this, [this] {
        previewSelected();
        emit zoomRequested();
    });

    if (cache_) {
        connect(cache_, &ThumbnailCache::ready, this, &PageGrid::updateItemIcon);
    }
    connect(this, &QListWidget::itemSelectionChanged, this, [this] {
        fab_->setVisible(!selectedItems().isEmpty());
        positionFab();
        emit selectionChangedRows();
    });
}

void PageGrid::setAssembly(core::Assembly* assembly) {
    assembly_ = assembly;
    refresh();
}

void PageGrid::refresh() { rebuild(); }

QVector<int> PageGrid::selectedRows() const {
    QVector<int> rows;
    const auto items = selectedItems();
    for (auto* it : items) {
        rows.push_back(row(it));
    }
    std::sort(rows.begin(), rows.end());
    return rows;
}

void PageGrid::applyVisualOrder() {
    if (!assembly_) {
        return;
    }
    std::vector<core::PageSpec> next;
    next.reserve(static_cast<size_t>(count()));
    for (int i = 0; i < count(); ++i) {
        auto* it = item(i);
        core::PageSpec spec;
        spec.file = it->data(Qt::UserRole).toString().toStdString();
        spec.index = it->data(Qt::UserRole + 1).toInt();
        spec.rotation = it->data(Qt::UserRole + 2).toInt();
        spec.included = it->data(Qt::UserRole + 3).toBool();
        next.push_back(std::move(spec));
    }
    assembly_->pages() = std::move(next);
}

void PageGrid::rebuild() {
    const int oldRow = currentRow();
    clear();
    if (!assembly_) {
        return;
    }
    const auto& t = Theme::instance().tokens();
    for (int i = 0; i < assembly_->count(); ++i) {
        const auto& spec = assembly_->pages()[static_cast<size_t>(i)];
        const QString path = QString::fromStdString(spec.file.string());
        auto* item = new QListWidgetItem(this);
        const QString name = QFileInfo(path).fileName();
        item->setText(QStringLiteral("p.%1").arg(spec.index + 1));
        item->setToolTip(QStringLiteral("%1 — page %2").arg(name).arg(spec.index + 1));
        item->setSizeHint(QSize(156, 220));
        item->setData(Qt::UserRole, path);
        item->setData(Qt::UserRole + 1, spec.index);
        item->setData(Qt::UserRole + 2, spec.rotation);
        item->setData(Qt::UserRole + 3, spec.included);

        QImage img;
        if (cache_) {
            img = cache_->get(path, spec.index, QSize(120, 160));
            if (img.isNull()) {
                cache_->request(path, spec.index, QSize(120, 160));
            }
        }
        if (!img.isNull()) {
            QTransform xf;
            xf.rotate(spec.rotation);
            img = img.transformed(xf, Qt::SmoothTransformation);
            item->setIcon(QIcon(QPixmap::fromImage(img)));
        } else {
            QPixmap ph(120, 160);
            ph.fill(t.surface2);
            item->setIcon(QIcon(ph));
        }
    }
    if (oldRow >= 0 && oldRow < count()) {
        setCurrentRow(oldRow);
    }
    positionFab();
}

void PageGrid::updateItemIcon(const QString& path, int page, const QImage& image) {
    if (image.isNull()) {
        return;
    }
    for (int i = 0; i < count(); ++i) {
        auto* it = item(i);
        if (it->data(Qt::UserRole).toString() == path && it->data(Qt::UserRole + 1).toInt() == page) {
            QImage img = image;
            QTransform xf;
            xf.rotate(it->data(Qt::UserRole + 2).toInt());
            img = img.transformed(xf, Qt::SmoothTransformation);
            it->setIcon(QIcon(QPixmap::fromImage(img)));
            break;
        }
    }
}

void PageGrid::positionFab() {
    if (!fab_) {
        return;
    }
    fab_->adjustSize();
    const int x = (width() - fab_->width()) / 2;
    const int y = height() - fab_->height() - 16;
    fab_->move(std::max(12, x), std::max(12, y));
    fab_->raise();
}

void PageGrid::resizeEvent(QResizeEvent* event) {
    QListWidget::resizeEvent(event);
    positionFab();
}

void PageGrid::mouseMoveEvent(QMouseEvent* event) {
    hoverRow_ = indexAt(event->position().toPoint()).row();
    QListWidget::mouseMoveEvent(event);
    viewport()->update();
}

void PageGrid::leaveEvent(QEvent* event) {
    hoverRow_ = -1;
    QListWidget::leaveEvent(event);
    viewport()->update();
}

int PageGrid::overlayHit(const QModelIndex& index, const QPoint& pos) const {
    if (!index.isValid()) {
        return -1;
    }
    const QRect vis = visualRect(index).adjusted(4, 4, -4, -4);
    for (int i = 0; i < kOverlayCount; ++i) {
        if (overlayBtn(vis, i).contains(pos)) {
            return i;
        }
    }
    return -1;
}

void PageGrid::mousePressEvent(QMouseEvent* event) {
    const QModelIndex idx = indexAt(event->position().toPoint());
    const int hit = overlayHit(idx, event->position().toPoint());
    if (hit >= 0 && event->button() == Qt::LeftButton) {
        setCurrentIndex(idx);
        if (!item(idx.row())->isSelected()) {
            item(idx.row())->setSelected(true);
        }
        if (hit == 0) {
            emit rotateRequested(90);
        } else if (hit == 1) {
            emit deleteRequested();
        } else {
            previewSelected();
            emit zoomRequested();
        }
        return;
    }
    QListWidget::mousePressEvent(event);
}

void PageGrid::previewSelected() {
    auto rows = selectedRows();
    if (rows.isEmpty() && hoverRow_ >= 0) {
        rows.push_back(hoverRow_);
    }
    if (rows.isEmpty() || !assembly_) {
        return;
    }
    const auto& spec = assembly_->pages()[static_cast<size_t>(rows.first())];
    const QString path = QString::fromStdString(spec.file.string());
    QImage img;
    if (cache_) {
        img = cache_->get(path, spec.index, QSize(600, 800));
        if (img.isNull()) {
            cache_->request(path, spec.index, QSize(600, 800));
            img = cache_->get(path, spec.index, QSize(120, 160));
        }
    }
    if (img.isNull()) {
        return;
    }
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle(QStringLiteral("Page preview"));
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    auto* lay = new QVBoxLayout(dlg);
    auto* lab = new QLabel(dlg);
    lab->setPixmap(QPixmap::fromImage(img).scaled(520, 720, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    lab->setAlignment(Qt::AlignCenter);
    lab->setAccessibleName(QStringLiteral("Enlarged page preview"));
    lay->addWidget(lab);
    dlg->resize(560, 760);
    dlg->show();
}

} // namespace drpdf

#include "page_grid.h"

#include "services/thumbnails.h"
#include "ui/theme/theme.h"

#include <QFileInfo>
#include <QIcon>
#include <QPixmap>
#include <QTransform>


#include <algorithm>


namespace drpdf {

PageGrid::PageGrid(ThumbnailCache* cache, QWidget* parent)
    : QListWidget(parent), cache_(cache) {
    setViewMode(QListView::IconMode);
    setIconSize(QSize(120, 160));
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Snap);
    setDragDropMode(InternalMove);
    setSelectionMode(ExtendedSelection);
    setSpacing(12);
    setUniformItemSizes(true);
    setWordWrap(true);
    if (cache_) {
        connect(cache_, &ThumbnailCache::ready, this, [this](const QString&, int, const QImage&) {
            rebuild();
        });
    }
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
        item->setText(QStringLiteral("%1\n%2  ·  p.%3")
                          .arg(name, spec.included ? QStringLiteral("on") : QStringLiteral("off"),
                               QString::number(spec.index + 1)));
        item->setToolTip(path);
        item->setSizeHint(QSize(148, 210));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
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
        if (!spec.included) {
            item->setForeground(t.muted);
        }
    }
    if (oldRow >= 0 && oldRow < count()) {
        setCurrentRow(oldRow);
    }
}

} // namespace drpdf

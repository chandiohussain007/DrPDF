#pragma once

#include "core/assembly.h"

#include <QListWidget>

class QWidget;
class QMouseEvent;

namespace drpdf {

class ThumbnailCache;

class PageGrid : public QListWidget {
    Q_OBJECT
public:
    explicit PageGrid(ThumbnailCache* cache, QWidget* parent = nullptr);

    void setAssembly(core::Assembly* assembly);
    void refresh();
    void applyVisualOrder();
    QVector<int> selectedRows() const;

signals:
    void orderChanged();
    void selectionChangedRows();
    void rotateRequested(int degrees);
    void deleteRequested();
    void zoomRequested();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void rebuild();
    void updateItemIcon(const QString& path, int page, const QImage& image);
    void positionFab();
    void previewSelected();
    int overlayHit(const QModelIndex& index, const QPoint& pos) const;

    ThumbnailCache* cache_ = nullptr;
    core::Assembly* assembly_ = nullptr;
    QWidget* fab_ = nullptr;
    int hoverRow_ = -1;
};

} // namespace drpdf

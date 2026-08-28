#pragma once

#include "core/assembly.h"

#include <QListWidget>

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

private:
    void rebuild();
    ThumbnailCache* cache_ = nullptr;
    core::Assembly* assembly_ = nullptr;
};

} // namespace drpdf

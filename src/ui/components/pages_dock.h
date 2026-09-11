#pragma once

#include "core/assembly.h"

#include <QWidget>

class QListWidget;
class QLabel;
class QToolButton;
class QSlider;

namespace drpdf {

class ThumbnailCache;

class PagesDock : public QWidget {
    Q_OBJECT
public:
    explicit PagesDock(ThumbnailCache* cache, QWidget* parent = nullptr);

    void setAssembly(core::Assembly* assembly);
    void refresh();
    int currentPage() const { return currentPage_; }

signals:
    void pageSelected(int page);
    void rotateRequested(int degrees);
    void deleteRequested();
    void zoomChanged(int delta);

private:
    void rebuild();
    void onItemSelectionChanged();

    ThumbnailCache* cache_ = nullptr;
    core::Assembly* assembly_ = nullptr;
    QListWidget* list_ = nullptr;
    QLabel* headerLabel_ = nullptr;
    QSlider* zoomSlider_ = nullptr;
    int currentPage_ = 0;
};

} // namespace drpdf
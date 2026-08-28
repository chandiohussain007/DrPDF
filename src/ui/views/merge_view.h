#pragma once

#include "core/assembly.h"

#include <QWidget>

namespace drpdf {

class Banner;
class DropZone;
class PageGrid;
class ThumbnailCache;

class MergeView : public QWidget {
    Q_OBJECT
public:
    explicit MergeView(ThumbnailCache* cache, QWidget* parent = nullptr);
    void addFiles(const QStringList& paths);

signals:
    void exported(const QString& path);

private:
    void doMerge();
    core::Assembly assembly_;
    DropZone* drop_ = nullptr;
    PageGrid* grid_ = nullptr;
    Banner* banner_ = nullptr;
};

} // namespace drpdf

#pragma once

#include "core/assembly.h"

#include <QWidget>

namespace drpdf {

class Banner;
class DropZone;
class PageGrid;
class ThumbnailCache;

class OrganizeView : public QWidget {
    Q_OBJECT
public:
    explicit OrganizeView(ThumbnailCache* cache, QWidget* parent = nullptr);
    void loadFile(const QString& path);

signals:
    void exported(const QString& path);

private:
    void saveAs();
    core::Assembly assembly_;
    DropZone* drop_ = nullptr;
    PageGrid* grid_ = nullptr;
    Banner* banner_ = nullptr;
};

} // namespace drpdf

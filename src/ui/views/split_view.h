#pragma once

#include "core/assembly.h"

#include <QWidget>

class QComboBox;
class QLineEdit;
class QSpinBox;

namespace drpdf {

class Banner;
class DropZone;
class PageGrid;
class ThumbnailCache;

class SplitView : public QWidget {
    Q_OBJECT
public:
    explicit SplitView(ThumbnailCache* cache, QWidget* parent = nullptr);
    void loadFile(const QString& path);

signals:
    void exported(const QString& path);

private:
    void applyMode();
    void doExport();
    core::Assembly assembly_;
    QString source_;
    DropZone* drop_ = nullptr;
    PageGrid* grid_ = nullptr;
    Banner* banner_ = nullptr;
    QComboBox* mode_ = nullptr;
    QLineEdit* ranges_ = nullptr;
    QSpinBox* everyN_ = nullptr;
};

} // namespace drpdf

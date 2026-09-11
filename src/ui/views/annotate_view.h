#pragma once

#include "services/working_copy.h"

#include <QWidget>

class QListWidget;
class QSpinBox;
class QButtonGroup;
class QLineEdit;

namespace drpdf {

class Banner;
class DropZone;
class PageCanvas;

class AnnotateView : public QWidget {
    Q_OBJECT
public:
    explicit AnnotateView(QWidget* parent = nullptr);
    void loadFile(const QString& path);

signals:
    void exported(const QString& path);

private:
    void reload();
    void refreshList();
    void saveAs();

    WorkingCopy working_;
    DropZone* drop_ = nullptr;
    PageCanvas* canvas_ = nullptr;
    QListWidget* list_ = nullptr;
    QSpinBox* page_ = nullptr;
    QLineEdit* note_ = nullptr;
    QButtonGroup* tools_ = nullptr;
    Banner* banner_ = nullptr;
};

} // namespace drpdf

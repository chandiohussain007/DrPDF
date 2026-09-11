#pragma once

#include "services/working_copy.h"

#include <QWidget>

class QListWidget;
class QLineEdit;
class QSpinBox;
class QLabel;

namespace drpdf {

class Banner;
class DropZone;
class PageCanvas;

class EditView : public QWidget {
    Q_OBJECT
public:
    explicit EditView(QWidget* parent = nullptr);
    void loadFile(const QString& path);

signals:
    void exported(const QString& path);

private:
    void reload();
    void refreshRuns();
    void replaceSelected();
    void addTextAt(QPointF pdf);
    void addImageAt(QRectF pdf);
    void redactAt(QRectF pdf);
    void saveAs();

    WorkingCopy working_;
    DropZone* drop_ = nullptr;
    PageCanvas* canvas_ = nullptr;
    QListWidget* runs_ = nullptr;
    QLineEdit* editor_ = nullptr;
    QSpinBox* page_ = nullptr;
    QSpinBox* fontSize_ = nullptr;
    Banner* banner_ = nullptr;
    QLabel* hint_ = nullptr;
};

} // namespace drpdf

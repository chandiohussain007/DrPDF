#pragma once

#include "services/working_copy.h"

#include <QImage>
#include <QRectF>
#include <QWidget>

class QLineEdit;
class QSpinBox;
class QCheckBox;

namespace drpdf {

class Banner;
class DropZone;
class PageCanvas;
class SignaturePad;

class SignView : public QWidget {
    Q_OBJECT
public:
    explicit SignView(QWidget* parent = nullptr);
    void loadFile(const QString& path);

signals:
    void exported(const QString& path);

private:
    QImage currentSignature() const;
    void placeOnPage(QRectF pdf);
    void saveAs();

    WorkingCopy working_;
    DropZone* drop_ = nullptr;
    PageCanvas* canvas_ = nullptr;
    SignaturePad* pad_ = nullptr;
    QLineEdit* typed_ = nullptr;
    QSpinBox* page_ = nullptr;
    QLineEdit* p12_ = nullptr;
    QLineEdit* p12pass_ = nullptr;
    QCheckBox* crypto_ = nullptr;
    Banner* banner_ = nullptr;
    QRectF lastRect_;
};

} // namespace drpdf

#pragma once

#include <QWidget>

class QListWidget;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QLabel;
class QResizeEvent;


namespace drpdf {

class Banner;
class DropZone;

class ImagesView : public QWidget {
    Q_OBJECT
public:
    explicit ImagesView(QWidget* parent = nullptr);
    void addImages(const QStringList& paths);

signals:
    void exported(const QString& path);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:

    void exportPdf();
    void showSelected();
    void applySize(bool fromWidth);

    DropZone* drop_ = nullptr;
    QListWidget* list_ = nullptr;
    QLabel* preview_ = nullptr;
    QLabel* native_ = nullptr;
    QComboBox* size_ = nullptr;
    QCheckBox* landscape_ = nullptr;
    QCheckBox* fit_ = nullptr;
    QCheckBox* lockAspect_ = nullptr;
    QSpinBox* margin_ = nullptr;
    QSpinBox* width_ = nullptr;
    QSpinBox* height_ = nullptr;
    Banner* banner_ = nullptr;
};

} // namespace drpdf

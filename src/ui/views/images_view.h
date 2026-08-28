#pragma once

#include <QWidget>

class QListWidget;
class QComboBox;
class QCheckBox;
class QSpinBox;

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

private:
    void exportPdf();
    DropZone* drop_ = nullptr;
    QListWidget* list_ = nullptr;
    QComboBox* size_ = nullptr;
    QCheckBox* landscape_ = nullptr;
    QCheckBox* fit_ = nullptr;
    QSpinBox* margin_ = nullptr;
    Banner* banner_ = nullptr;
};

} // namespace drpdf

#pragma once

#include <QWidget>

class QLineEdit;
class QDoubleSpinBox;
class QCheckBox;
class QComboBox;
class QSpinBox;

namespace drpdf {

class Banner;
class DropZone;

class WatermarkView : public QWidget {
    Q_OBJECT
public:
    explicit WatermarkView(QWidget* parent = nullptr);
    void loadFile(const QString& path);

signals:
    void exported(const QString& path);

private:
    void apply();
    QString path_;
    DropZone* drop_ = nullptr;
    QLineEdit* mark_ = nullptr;
    QDoubleSpinBox* size_ = nullptr;
    QDoubleSpinBox* angle_ = nullptr;
    QDoubleSpinBox* opacity_ = nullptr;
    QCheckBox* tiled_ = nullptr;
    QLineEdit* header_ = nullptr;
    QLineEdit* footer_ = nullptr;
    QLineEdit* pageFmt_ = nullptr;
    QComboBox* pagePos_ = nullptr;
    Banner* banner_ = nullptr;
};

} // namespace drpdf

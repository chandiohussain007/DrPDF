#pragma once

#include <QWidget>

class QSlider;
class QComboBox;
class QCheckBox;
class QLabel;

namespace drpdf {

class OptimizationDock : public QWidget {
    Q_OBJECT
public:
    explicit OptimizationDock(QWidget* parent = nullptr);

    int quality() const;
    int dpi() const;
    bool stripExif() const;
    bool subsetFonts() const;
    bool linearize() const;
    bool downsampleImages() const;

signals:
    void qualityChanged(int quality);
    void dpiChanged(int dpi);
    void optimizeRequested();

private:
    QSlider* qualitySlider_ = nullptr;
    QComboBox* colorFilter_ = nullptr;
    QComboBox* monoFilter_ = nullptr;
    QCheckBox* exifCheck_ = nullptr;
    QCheckBox* fontCheck_ = nullptr;
    QCheckBox* linearCheck_ = nullptr;
    QCheckBox* downsampleCheck_ = nullptr;
    QLabel* savingsLabel_ = nullptr;
};

} // namespace drpdf
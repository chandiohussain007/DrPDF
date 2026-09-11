#pragma once

#include <QWidget>

class QComboBox;
class QCheckBox;
class QProgressBar;

namespace drpdf {

class Banner;
class DropZone;

class OcrView : public QWidget {
    Q_OBJECT
public:
    explicit OcrView(QWidget* parent = nullptr);
    void loadFile(const QString& path);

signals:
    void exported(const QString& path);

private:
    void run();
    QString path_;
    DropZone* drop_ = nullptr;
    QComboBox* lang_ = nullptr;
    QCheckBox* sidecar_ = nullptr;
    QProgressBar* progress_ = nullptr;
    Banner* banner_ = nullptr;
};

} // namespace drpdf

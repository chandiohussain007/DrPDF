#pragma once

#include <QWidget>

class QComboBox;

namespace drpdf {

class Banner;
class DropZone;

class CompressView : public QWidget {
    Q_OBJECT
public:
    explicit CompressView(QWidget* parent = nullptr);
    void loadFile(const QString& path);

signals:
    void exported(const QString& path);

private:
    void run();
    QString path_;
    DropZone* drop_ = nullptr;
    QComboBox* tier_ = nullptr;
    Banner* banner_ = nullptr;
};

} // namespace drpdf

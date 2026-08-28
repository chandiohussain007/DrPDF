#pragma once

#include <QWidget>

class QLineEdit;

namespace drpdf {

class Banner;
class DropZone;

class ProtectView : public QWidget {
    Q_OBJECT
public:
    explicit ProtectView(QWidget* parent = nullptr);
    void loadFile(const QString& path);

signals:
    void exported(const QString& path);

private:
    void lock();
    void unlock();
    QString path_;
    DropZone* drop_ = nullptr;
    QLineEdit* current_ = nullptr;
    QLineEdit* user_ = nullptr;
    QLineEdit* owner_ = nullptr;
    Banner* banner_ = nullptr;
};

} // namespace drpdf

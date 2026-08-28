#pragma once

#include <QWidget>

class QLabel;
class QProgressBar;

namespace drpdf {

class Banner : public QWidget {
    Q_OBJECT
public:
    explicit Banner(QWidget* parent = nullptr);

    void showInfo(const QString& text);
    void showError(const QString& text);
    void showProgress(const QString& text, int current, int total);
    void hideBanner();

private:
    QLabel* label_ = nullptr;
    QProgressBar* bar_ = nullptr;
};

} // namespace drpdf

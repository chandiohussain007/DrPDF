#pragma once

#include <QWidget>

class QLabel;
class QSlider;
class QToolButton;

namespace drpdf {

class StatusBar : public QWidget {
    Q_OBJECT
public:
    explicit StatusBar(QWidget* parent = nullptr);

    void setPageInfo(int page, int total);
    void setZoom(int percent);
    void setPrivacyText(const QString& text);

signals:
    void prevPage();
    void nextPage();
    void firstPage();
    void lastPage();
    void zoomChanged(int percent);
    void fitWidth();
    void fitPage();

private:
    QLabel* pageLabel_ = nullptr;
    QSlider* zoomSlider_ = nullptr;
    QLabel* zoomLabel_ = nullptr;
    QLabel* privacy_ = nullptr;
};

} // namespace drpdf
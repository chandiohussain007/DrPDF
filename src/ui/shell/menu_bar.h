#pragma once

#include <QWidget>

class QMenuBar;

namespace drpdf {

class MenuBar : public QWidget {
    Q_OBJECT
public:
    explicit MenuBar(QWidget* parent = nullptr);

signals:
    void toolChosen(int toolIndex);

private:
    QMenuBar* bar_ = nullptr;
};

} // namespace drpdf
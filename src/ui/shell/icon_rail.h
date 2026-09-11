#pragma once

#include <QWidget>

class QToolButton;
class QVBoxLayout;

namespace drpdf {

class IconRail : public QWidget {
    Q_OBJECT
public:
    explicit IconRail(QWidget* parent = nullptr);

    void setCurrent(int index);

signals:
    void panelChosen(int index);

private:
    QToolButton* addBtn(const QString& icon, const QString& tip, int index);
    QVBoxLayout* lay_ = nullptr;
    QVector<QToolButton*> buttons_;
};

} // namespace drpdf
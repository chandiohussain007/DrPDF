#pragma once

#include "ui/tools.h"

#include <QVector>
#include <QWidget>


class QToolButton;
class QVBoxLayout;

namespace drpdf {

class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget* parent = nullptr);
    void setCurrent(Tool tool);
    void setCollapsed(bool collapsed);
    bool collapsed() const { return collapsed_; }

signals:
    void toolChosen(Tool tool);
    void collapseToggled(bool collapsed);

private:
    QToolButton* addNav(const QString& icon, const QString& text, Tool tool);
    void relabel();

    bool collapsed_ = false;
    QVBoxLayout* nav_ = nullptr;
    QVector<QToolButton*> buttons_;
    QVector<QWidget*> headers_;
};

} // namespace drpdf

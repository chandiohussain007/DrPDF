#pragma once

#include "ui/tools.h"

#include <QWidget>

namespace drpdf {

class ToolCard : public QWidget {
    Q_OBJECT
public:
    ToolCard(Tool tool, const QString& iconName, const QString& title, const QString& subtitle,
             bool ready, QWidget* parent = nullptr);

    Tool tool() const { return tool_; }

signals:
    void activated(Tool tool);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    Tool tool_;
    QString iconName_;
    QString title_;
    QString subtitle_;
    bool ready_ = true;
    bool hover_ = false;
};

} // namespace drpdf

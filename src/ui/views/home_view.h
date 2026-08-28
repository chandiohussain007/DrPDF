#pragma once

#include "ui/tools.h"

#include <QWidget>

namespace drpdf {

class HomeView : public QWidget {
    Q_OBJECT
public:
    explicit HomeView(QWidget* parent = nullptr);

signals:
    void toolChosen(Tool tool);
    void filesDropped(const QStringList& paths);
};

} // namespace drpdf

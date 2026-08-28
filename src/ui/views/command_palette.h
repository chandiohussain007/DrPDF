#pragma once

#include "ui/tools.h"

#include <QDialog>

class QLineEdit;
class QListWidget;

namespace drpdf {

class CommandPalette : public QDialog {
    Q_OBJECT
public:
    explicit CommandPalette(QWidget* parent = nullptr);

signals:
    void toolChosen(Tool tool);

private:
    void filter(const QString& text);
    QLineEdit* input_ = nullptr;
    QListWidget* list_ = nullptr;
};

} // namespace drpdf

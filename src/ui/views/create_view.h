#pragma once

#include <QWidget>

class QTextEdit;
class QFontComboBox;
class QSpinBox;

namespace drpdf {

class Banner;

class CreateView : public QWidget {
    Q_OBJECT
public:
    explicit CreateView(QWidget* parent = nullptr);

signals:
    void exported(const QString& path);

private:
    void exportPdf();
    QTextEdit* editor_ = nullptr;
    QFontComboBox* font_ = nullptr;
    QSpinBox* size_ = nullptr;
    Banner* banner_ = nullptr;
};

} // namespace drpdf

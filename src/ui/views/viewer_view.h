#pragma once

#include <QWidget>

class QPdfDocument;
class QPdfView;
class QLabel;

namespace drpdf {

class ViewerView : public QWidget {
    Q_OBJECT
public:
    explicit ViewerView(QWidget* parent = nullptr);
    void openFile(const QString& path);

private:
    QPdfDocument* doc_ = nullptr;
    QPdfView* view_ = nullptr;
    QLabel* meta_ = nullptr;
};

} // namespace drpdf

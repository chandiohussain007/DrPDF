#pragma once

#include <QWidget>

class QStackedWidget;
class QToolButton;

namespace drpdf {

class InspectorDock : public QWidget {
    Q_OBJECT
public:
    explicit InspectorDock(QWidget* parent = nullptr);

    void setPageCount(int count);
    void setPdfVersion(const QString& version);
    void setSecurityStatus(const QString& status);

signals:
    void fontFamilyChanged(const QString& family);
    void fontSizeChanged(int size);
    void colorChanged(const QColor& color);
    void opacityChanged(int opacity);
    void alignmentChanged(int align);

private:
    QStackedWidget* stack_ = nullptr;
};

} // namespace drpdf
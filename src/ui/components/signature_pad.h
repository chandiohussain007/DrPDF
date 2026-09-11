#pragma once

#include <QImage>
#include <QPointF>
#include <QVector>
#include <QWidget>

namespace drpdf {

class SignaturePad : public QWidget {
    Q_OBJECT
public:
    explicit SignaturePad(QWidget* parent = nullptr);
    QImage toImage() const;
    bool isEmpty() const { return strokes_.isEmpty(); }
    void clear();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QVector<QVector<QPointF>> strokes_;
    QVector<QPointF> current_;
    bool drawing_ = false;
};

} // namespace drpdf

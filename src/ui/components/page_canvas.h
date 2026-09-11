#pragma once

#include <QImage>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QVector>
#include <QWidget>

namespace drpdf {

class PageCanvas : public QWidget {
    Q_OBJECT
public:
    enum class Mode { None, Rect, Ink, Place };

    explicit PageCanvas(QWidget* parent = nullptr);

    void setPage(const QImage& image, QSizeF pointSize);
    void setMode(Mode mode);
    Mode mode() const { return mode_; }
    void clearPreviewStroke();

signals:
    void rectDrawn(QRectF pdfRect);
    void strokeDrawn(QVector<QPointF> pdfPoints);
    void placed(QPointF pdfPoint);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QRectF imageRect() const;
    QPointF toPdf(const QPointF& widget) const;
    QPointF toWidget(const QPointF& pdf) const;

    QImage image_;
    QSizeF points_{612, 792};
    Mode mode_ = Mode::None;
    bool dragging_ = false;
    QPointF start_;
    QPointF current_;
    QVector<QPointF> stroke_;
};

} // namespace drpdf

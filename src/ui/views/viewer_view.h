#pragma once

#include <QWidget>

class QPdfDocument;
class QPdfView;
class QLabel;
class QListWidget;
class QToolButton;

namespace drpdf {

class ThumbnailCache;

class ViewerView : public QWidget {
    Q_OBJECT
public:
    explicit ViewerView(QWidget* parent = nullptr);
    void openFile(const QString& path);
    void setThumbnailCache(ThumbnailCache* cache);

private:
    void setCustomZoom(qreal factor);
    void zoomBy(qreal mul);
    void updateZoomLabel();
    void rebuildThumbs();
    void jumpTo(int page);

    QPdfDocument* doc_ = nullptr;
    QPdfView* view_ = nullptr;
    QLabel* meta_ = nullptr;
    QLabel* zoomLabel_ = nullptr;
    QListWidget* thumbs_ = nullptr;
    QToolButton* collapseThumbs_ = nullptr;
    ThumbnailCache* cache_ = nullptr;
    QString path_;
};

} // namespace drpdf

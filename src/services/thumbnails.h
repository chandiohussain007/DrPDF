#pragma once

#include <QHash>
#include <QImage>
#include <QObject>
#include <QSet>
#include <QSize>
#include <QString>


namespace drpdf {

class ThumbnailCache : public QObject {
    Q_OBJECT
public:
    explicit ThumbnailCache(QObject* parent = nullptr);

    QImage get(const QString& path, int page, const QSize& size) const;
    void request(const QString& path, int page, const QSize& size);

signals:
    void ready(const QString& path, int page, const QImage& image);

private:
    QString key(const QString& path, int page, const QSize& size) const;
    QHash<QString, QImage> cache_;
    QSet<QString> inflight_;
;

} // namespace drpdf

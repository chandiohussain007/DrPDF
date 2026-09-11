#pragma once

#include <QHash>
#include <QImage>
#include <QList>
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
    void insert(const QString& k, const QImage& img);

    QHash<QString, QImage> cache_;
    QSet<QString> inflight_;
    QList<QString> lru_;
    static constexpr int kMaxEntries = 256;
};

} // namespace drpdf

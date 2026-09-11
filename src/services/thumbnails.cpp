#include "thumbnails.h"

#include <QPdfDocument>
#include <QPointer>
#include <QtConcurrent>

namespace drpdf {

ThumbnailCache::ThumbnailCache(QObject* parent) : QObject(parent) {}

QString ThumbnailCache::key(const QString& path, int page, const QSize& size) const {
    return path + QLatin1Char('#') + QString::number(page) + QLatin1Char('@') +
           QString::number(size.width()) + QLatin1Char('x') + QString::number(size.height());
}

QImage ThumbnailCache::get(const QString& path, int page, const QSize& size) const {
    return cache_.value(key(path, page, size));
}

void ThumbnailCache::insert(const QString& k, const QImage& img) {
    if (cache_.contains(k)) {
        lru_.removeAll(k);
    }
    cache_.insert(k, img);
    lru_.push_back(k);
    while (lru_.size() > kMaxEntries) {
        const QString old = lru_.takeFirst();
        cache_.remove(old);
    }
}

void ThumbnailCache::request(const QString& path, int page, const QSize& size) {
    const QString k = key(path, page, size);
    if (cache_.contains(k) || inflight_.contains(k)) {
        return;
    }
    inflight_.insert(k);

    QPointer<ThumbnailCache> self(this);
    (void)QtConcurrent::run([self, path, page, size, k]() {
        QPdfDocument doc;
        const auto err = doc.load(path);
        QImage img;
        if (err == QPdfDocument::Error::None && page >= 0 && page < doc.pageCount()) {
            img = doc.render(page, size);
        }
        if (!self) {
            return;
        }
        QMetaObject::invokeMethod(
            self.data(),
            [self, path, page, img, k]() {
                if (!self) {
                    return;
                }
                self->inflight_.remove(k);
                if (!img.isNull()) {
                    self->insert(k, img);
                }
                emit self->ready(path, page, img);
            },
            Qt::QueuedConnection);
    });
}

} // namespace drpdf

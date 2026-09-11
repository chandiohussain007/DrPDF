#include "icons.h"

#include <QByteArray>
#include <QFile>
#include <QPainter>
#include <QPixmapCache>
#include <QSvgRenderer>
#include <QHash>
#include <algorithm>

namespace drpdf {
namespace {

// Friendly names used across the UI mapped onto the vendored Qlementine set.
// Unknown names still resolve to :/icons/<name>.svg via the fallback below.
const QHash<QString, QString>& iconAliases() {
    static const QHash<QString, QString> m{
        {QStringLiteral("sign"), QStringLiteral("certified")},
        {QStringLiteral("protect"), QStringLiteral("lock")},
        {QStringLiteral("annotate"), QStringLiteral("highlight")},
        {QStringLiteral("create"), QStringLiteral("add-file")},
        {QStringLiteral("images"), QStringLiteral("image")},
        {QStringLiteral("organize"), QStringLiteral("layers")},
        {QStringLiteral("viewer"), QStringLiteral("pages")},
        {QStringLiteral("watermark"), QStringLiteral("page-text")},
        {QStringLiteral("edit"), QStringLiteral("pen")},
        {QStringLiteral("back"), QStringLiteral("arrow-left")},
        {QStringLiteral("hand"), QStringLiteral("cursor")},
    };
    return m;
}

// Qlementine icon SVGs are authored as black glyphs with fill="#000".
// Recolor = string-replace the fill before handing it to QSvgRenderer.
QString recoloredSvg(const QString& resourcePath, const QColor& color) {
    QFile f(resourcePath);
    if (!f.open(QIODevice::ReadOnly)) {
        return {};
    }
    QString svg = QString::fromUtf8(f.readAll());
    svg.replace(QStringLiteral("fill=\"#000\""),
                QStringLiteral("fill=\"%1\"").arg(color.name(QColor::HexRgb)));
    svg.replace(QStringLiteral("fill='#000'"),
                QStringLiteral("fill='%1'").arg(color.name(QColor::HexRgb)));
    return svg;
}

} // namespace

QPixmap Icons::pixmap(const QString& nameIn, const QColor& color, int size) {
    const QString name = iconAliases().value(nameIn, nameIn);
    const QString key =
        QStringLiteral("ic|%1|%2|%3").arg(name, color.name(QColor::HexRgb)).arg(size);
    QPixmap cached;
    if (QPixmapCache::find(key, &cached)) {
        return cached;
    }

    const QString path = QStringLiteral(":/icons/%1.svg").arg(name);
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);

    const QString svg = recoloredSvg(path, color);
    if (svg.isEmpty()) {
        // Fallback: draw a neutral document glyph so the UI never shows a hole.
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(color, std::max(1.4, size / 12.0)));
        p.drawRect(size * 0.22, size * 0.16, size * 0.56, size * 0.68);
        p.end();
    } else {
        QSvgRenderer renderer(svg.toUtf8());
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        renderer.render(&p, QRectF(0, 0, size, size));
        p.end();
    }

    QPixmapCache::insert(key, pm);
    return pm;
}

QIcon Icons::named(const QString& name, const QColor& color, int size) {
    QIcon icon;
    for (const int s : {16, 20, 24, 32, 48}) {
        icon.addPixmap(pixmap(name, color, s));
    }
    Q_UNUSED(size);
    return icon;
}

QIcon Icons::app(int size) {
    QPixmap pm = logo(size);
    if (pm.isNull()) {
        pm = pixmap(QStringLiteral("pdf"), QColor("#00D2FF"), size);
    }
    return QIcon(pm);
}

QPixmap Icons::logo(int height) {
    QPixmap raw(QStringLiteral(":/logo.png"));
    if (raw.isNull()) {
        return {};
    }
    return raw.scaledToHeight(height, Qt::SmoothTransformation);
}

} // namespace drpd
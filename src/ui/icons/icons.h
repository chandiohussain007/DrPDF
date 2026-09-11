#pragma once

#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QString>

namespace drpdf {

// Icon system: vendors a curated set of Qlementine Icons SVGs (MIT,
// https://github.com/oclero/qlementine-icons) into :/icons/<name>.svg and
// recolors them at render time through QSvgRenderer.
class Icons {
public:
    // Returns the icon for a logical name (see src/resources/icons/).
    static QIcon named(const QString& name, const QColor& color, int size = 32);
    static QPixmap pixmap(const QString& name, const QColor& color, int size = 32);
    static QIcon app(int size = 64);

    // The provided brand logo (:/logo.png).
    static QPixmap logo(int height = 28);
};

} // namespace drpdf


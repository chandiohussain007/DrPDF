#pragma once

#include <QIcon>
#include <QPixmap>
#include <QString>

namespace drpdf {

class Icons {
public:
    static QIcon app(int size = 64);
    static QIcon named(const QString& name, const QColor& color, int size = 32);
    static QPixmap pixmap(const QString& name, const QColor& color, int size = 32);
};

} // namespace drpdf

#pragma once

#include <QColor>
#include <QString>

class QApplication;
class QWidget;

namespace drpdf {

struct Tokens {
    QColor bg;
    QColor surface;
    QColor surfaceHover;
    QColor surface2;
    QColor border;
    QColor text;
    QColor muted;
    QColor accent;
    QColor accentHover;
    QColor accentText;
    QColor danger;
    QColor warning;
    QColor success;
    QColor shadow;
    int radius = 14;
    int radiusSm = 10;
};

class Theme {
public:
    static Theme& instance();

    bool dark() const { return dark_; }
    void setDark(bool dark);
    void toggle();
    void apply(QApplication& app);

    const Tokens& tokens() const { return tokens_; }
    QString stylesheet() const;

private:
    Theme() = default;
    void rebuild();

    bool dark_ = true;
    Tokens tokens_;
};

} // namespace drpdf

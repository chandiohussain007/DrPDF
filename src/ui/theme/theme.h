#pragma once

#include <QColor>
#include <QString>

class QApplication;

namespace drpdf {

struct Tokens {
    QColor bg;              // surface-deep-midnight #06090f
    QColor surface;          // surface-midnight-base #0b0f19
    QColor surfaceHover;     // surface-container-high #262a35
    QColor surface2;        // surface-midnight-elevated #111827
    QColor surface3;        // surface-container-low #171b26
    QColor surface4;        // surface-container #1c1f2a
    QColor border;           // glass-border-hairline rgba(255,255,255,0.08)
    QColor text;             // text-primary #f8fafc
    QColor muted;           // text-secondary #94a3b8
    QColor muted2;          // text-muted #64748b
    QColor accent;           // accent-electric-cyan #00d2ff
    QColor accentHover;      // accent-azure-glow #38bdf8
    QColor accentSolid;      // accent-azure-solid #0ea5e9
    QColor accentText;        // on-primary #003543
    QColor gold;              // security-gold-base #f59e0b
    QColor goldLight;        // security-gold-light #fbbf24
    QColor danger;           // system-red-danger #f43f5e
    QColor warning;           // security-gold-base #f59e0b
    QColor success;           // system-green-success #10b981
    QColor shadow;           // rgba(0,0,0,90)
    int radius = 12;
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
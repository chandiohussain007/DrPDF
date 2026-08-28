#include "theme.h"

#include <QApplication>
#include <QPalette>
#include <QStyleFactory>

namespace drpdf {

Theme& Theme::instance() {
    static Theme t;
    return t;
}

void Theme::rebuild() {
    Tokens t;
    if (dark_) {
        t.bg = QColor("#0B1220");
        t.surface = QColor("#121A2B");
        t.surfaceHover = QColor("#182236");
        t.surface2 = QColor("#1C2740");
        t.border = QColor("#2A3A55");
        t.text = QColor("#F1F5F9");
        t.muted = QColor("#8BA0B8");
        t.accent = QColor("#12B5A0");
        t.accentHover = QColor("#2DD4BF");
        t.accentText = QColor("#042F2E");
        t.danger = QColor("#FB7185");
        t.warning = QColor("#FBBF24");
        t.success = QColor("#34D399");
        t.shadow = QColor(0, 0, 0, 80);
    } else {
        t.bg = QColor("#F3F5F8");
        t.surface = QColor("#FFFFFF");
        t.surfaceHover = QColor("#F8FAFC");
        t.surface2 = QColor("#EEF2F6");
        t.border = QColor("#E2E8F0");
        t.text = QColor("#0F172A");
        t.muted = QColor("#64748B");
        t.accent = QColor("#0F766E");
        t.accentHover = QColor("#0D9488");
        t.accentText = QColor("#FFFFFF");
        t.danger = QColor("#E11D48");
        t.warning = QColor("#D97706");
        t.success = QColor("#059669");
        t.shadow = QColor(15, 23, 42, 28);
    }
    tokens_ = t;
}

QString Theme::stylesheet() const {
    const auto& t = tokens_;
    auto hex = [](const QColor& c) { return c.name(QColor::HexRgb); };
    return QString(R"(
* { font-family: "Segoe UI Variable", "Segoe UI", "SF Pro Display", "Inter", "Ubuntu", sans-serif; }
QMainWindow, QDialog, QWidget#Root { background: %1; color: %2; }
QLabel { color: %2; }
QLabel[muted="true"] { color: %3; }
QLineEdit, QPlainTextEdit, QTextEdit, QSpinBox, QComboBox, QListWidget, QTreeWidget {
    background: %4; color: %2; border: 1px solid %5; border-radius: 10px; padding: 8px 10px;
    selection-background-color: %6; selection-color: %7;
}
QLineEdit:focus, QTextEdit:focus, QComboBox:focus { border: 1px solid %6; }
QPushButton {
    background: %4; color: %2; border: 1px solid %5; border-radius: 10px;
    padding: 8px 14px; font-weight: 600;
}
QPushButton:hover { background: %8; }
QPushButton:pressed { background: %9; }
QPushButton[primary="true"] {
    background: %6; color: %7; border: 1px solid %6;
}
QPushButton[primary="true"]:hover { background: %10; }
QPushButton[danger="true"] { color: %11; border-color: %11; }
QToolButton {
    background: transparent; color: %2; border: none; border-radius: 10px; padding: 8px;
}
QToolButton:hover { background: %8; }
QToolButton:checked { background: %9; color: %6; }
QScrollBar:vertical { background: transparent; width: 10px; margin: 4px; }
QScrollBar::handle:vertical { background: %5; border-radius: 5px; min-height: 32px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal { background: transparent; height: 10px; margin: 4px; }
QScrollBar::handle:horizontal { background: %5; border-radius: 5px; min-width: 32px; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
QMenu { background: %4; color: %2; border: 1px solid %5; border-radius: 10px; padding: 6px; }
QMenu::item { padding: 8px 16px; border-radius: 8px; }
QMenu::item:selected { background: %8; }
QStatusBar { background: %1; color: %3; border-top: 1px solid %5; }
QProgressBar { background: %9; border: none; border-radius: 6px; height: 8px; text-align: center; }
QProgressBar::chunk { background: %6; border-radius: 6px; }
QSplitter::handle { background: %5; }
QToolBar { background: transparent; border: none; spacing: 6px; }
QCheckBox, QRadioButton { color: %2; spacing: 8px; }
QGroupBox { color: %2; border: 1px solid %5; border-radius: 12px; margin-top: 12px; padding: 12px; }
QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }
QListWidget { outline: none; }
QListWidget::item { border-radius: 10px; padding: 6px; }
QListWidget::item:selected { background: %8; }
QHeaderView::section { background: %4; color: %3; border: none; padding: 8px; }
)")
        .arg(hex(t.bg), hex(t.text), hex(t.muted), hex(t.surface), hex(t.border), hex(t.accent),
             hex(t.accentText), hex(t.surfaceHover), hex(t.surface2), hex(t.accentHover),
             hex(t.danger));
}

void Theme::setDark(bool dark) {
    dark_ = dark;
    rebuild();
}

void Theme::toggle() {
    setDark(!dark_);
}

void Theme::apply(QApplication& app) {
    rebuild();
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QPalette pal = app.palette();
    pal.setColor(QPalette::Window, tokens_.bg);
    pal.setColor(QPalette::WindowText, tokens_.text);
    pal.setColor(QPalette::Base, tokens_.surface);
    pal.setColor(QPalette::AlternateBase, tokens_.surface2);
    pal.setColor(QPalette::Text, tokens_.text);
    pal.setColor(QPalette::Button, tokens_.surface);
    pal.setColor(QPalette::ButtonText, tokens_.text);
    pal.setColor(QPalette::Highlight, tokens_.accent);
    pal.setColor(QPalette::HighlightedText, tokens_.accentText);
    pal.setColor(QPalette::PlaceholderText, tokens_.muted);
    pal.setColor(QPalette::ToolTipBase, tokens_.surface2);
    pal.setColor(QPalette::ToolTipText, tokens_.text);
    app.setPalette(pal);
    app.setStyleSheet(stylesheet());
}

} // namespace drpdf

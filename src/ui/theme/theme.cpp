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
        t.bg = QColor("#06090F"); t.surface = QColor("#0B0F19");
        t.surfaceHover = QColor("#262A35"); t.surface2 = QColor("#111827");
        t.surface3 = QColor("#171B26"); t.surface4 = QColor("#1C1F2A");
        t.border = QColor(255, 255, 255, 20);
        t.text = QColor("#F8FAFC"); t.muted = QColor("#94A3B8"); t.muted2 = QColor("#64748B");
        t.accent = QColor("#00D2FF"); t.accentHover = QColor("#38BDF8"); t.accentSolid = QColor("#0EA5E9");
        t.accentText = QColor("#003543"); t.gold = QColor("#F59E0B"); t.goldLight = QColor("#FBBF24");
        t.danger = QColor("#F43F5E"); t.warning = QColor("#F59E0B"); t.success = QColor("#10B981");
        t.shadow = QColor(0, 0, 0, 110);
    } else {
        t.bg = QColor("#F6F7FB"); t.surface = QColor("#FFFFFF"); t.surfaceHover = QColor("#EEF2F7");
        t.surface2 = QColor("#F1F5F9"); t.surface3 = QColor("#E8EDF3"); t.surface4 = QColor("#F8FAFC");
        t.border = QColor("#DDE3EC"); t.text = QColor("#0F172A"); t.muted = QColor("#5A6B7F");
        t.muted2 = QColor("#8B99AB"); t.accent = QColor("#0284C7"); t.accentHover = QColor("#0369A1");
        t.accentSolid = QColor("#0EA5E9"); t.accentText = QColor("#FFFFFF");
        t.gold = QColor("#B45309"); t.goldLight = QColor("#D97706"); t.danger = QColor("#E11D48");
        t.warning = QColor("#D97706"); t.success = QColor("#059669"); t.shadow = QColor(15, 23, 42, 30);
    }
    tokens_ = t;
}

QString Theme::stylesheet() const {
    const auto& ti = tokens_;
    auto hex = [](const QColor& c) { return c.name(QColor::HexRgb); };
    auto rgba = [](const QColor& c) {
        return QStringLiteral("rgba(%1,%2,%3,%4)")
            .arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alphaF(), 0, 'f', 3);
    };
    return QString(
    R"(* { font-family: "Segoe UI Variable Text", "Segoe UI", "Inter", "Ubuntu", sans-serif; outline: none; }
QMainWindow, QDialog, QWidget#Root { background: %1; color: %2; }
QLabel { color: %2; background: transparent; }
QLabel[muted="true"] { color: %3; }
QLabel[gold="true"] { color: %12; }
QLabel[accent="true"] { color: %6; }
QLabel[strong="true"] { color: %2; font-weight: 600; }
QLabel[danger="true"] { color: %11; }
QLabel[success="true"] { color: %16; }
QLabel[mono="true"] { font-family: "JetBrains Mono","Consolas",monospace; font-size: 11px; }
QLabel[section="true"] { color: %18; font-size: 10px; font-weight: 600; letter-spacing: 1px; }
QLineEdit, QPlainTextEdit, QTextEdit, QSpinBox, QDoubleSpinBox, QComboBox {
    background: %1; color: %2; border: 1px solid %5; border-radius: 8px;
    padding: 6px 10px; selection-background-color: %6; selection-color: %7; min-height: 18px; }
QLineEdit:focus, QTextEdit:focus, QSpinBox:focus, QComboBox:focus { border: 1px solid %6; }
QComboBox::drop-down { border: none; width: 22px; }
QComboBox QAbstractItemView { background: %4; color: %2; border: 1px solid %5; border-radius: 8px; selection-background-color: %8; selection-color: %2; padding: 4px; }
QPushButton { background: %4; color: %2; border: 1px solid %5; border-radius: 8px; padding: 7px 16px; font-weight: 600; font-size: 12px; }
QPushButton:hover { background: %8; }
QPushButton:pressed { background: %9; }
QPushButton:disabled { color: %18; border-color: %5; background: %17; }
QPushButton[primary="true"], QPushButton#CyanBtn { background: %6; color: %7; border: 1px solid %6; font-weight: 700; }
QPushButton[primary="true"]:hover, QPushButton#CyanBtn:hover { background: %10; }
QPushButton#GoldBtn { background: %12; color: #06090F; border: 1px solid %12; font-weight: 700; }
QPushButton#GoldBtn:hover { background: %15; }
QPushButton[danger="true"] { color: %11; border-color: rgba(244,63,94,0.4); }
QToolButton { background: transparent; color: %3; border: none; border-radius: 8px; padding: 6px; }
QToolButton:hover { background: %8; color: %2; }
QToolButton:pressed { background: %9; }
QToolButton:checked { background: %9; color: %6; }
QToolButton:disabled { color: %18; }
QToolButton#IconBtn { padding: 5px; }
QToolButton#RibbonTab { border: none; border-radius: 6px 6px 0 0; padding: 7px 14px; font-weight: 600; font-size: 12px; color: %3; }
QToolButton#RibbonTab:hover { color: %2; }
QToolButton#RibbonTab:checked { color: %6; border-bottom: 2px solid %6; }
QToolButton#RailBtn { padding: 8px; border-radius: 8px; }
QToolButton#RailBtn:checked { background: rgba(0,210,255,0.12); }
QFrame#Card, QFrame#DockSection { background: %17; border: 1px solid %13; border-radius: 12px; }
QWidget#DockHeader, QFrame#DockHeader { background: %9; border: none; }
QWidget#DockBody { background: transparent; }
QFrame#DropZone { background: %17; border: 1.5px dashed %18; border-radius: 14px; }
QFrame#DropZone[hover="true"] { background: rgba(0,210,255,0.06); border-color: %6; }
QFrame#DropZone QLabel[title="true"] { color: %2; font-size: 14px; font-weight: 600; }
QFrame#DropZone QLabel { color: %3; }
QFrame#Pill { background: %4; border: 1px solid %13; border-radius: 14px; }
QFrame#Pill[accent="true"] { background: rgba(0,210,255,0.1); border-color: rgba(0,210,255,0.3); }
QFrame#Pill[gold="true"] { background: rgba(245,158,11,0.12); border-color: rgba(245,158,11,0.35); }
QWidget#TitleBar { background: %9; border-bottom: 1px solid %13; }
QWidget#MenuBar { background: %9; border-bottom: 1px solid %13; }
QWidget#Ribbon, QWidget#RibbonTabRow, QWidget#ContextBar { background: %9; }
QWidget#ContextBar { border-bottom: 1px solid %13; }
QWidget#IconRail { background: %9; border-right: 1px solid %13; }
QWidget#StatusBar { background: %9; border-top: 1px solid %13; }
)"
    R"(
QMenuBar { background: transparent; color: %3; border: none; padding: 0; }
QMenuBar::item { background: transparent; padding: 4px 10px; border-radius: 6px; color: %3; font-size: 12px; }
QMenuBar::item:selected { background: %8; color: %2; }
QMenu { background: %4; color: %2; border: 1px solid %5; border-radius: 10px; padding: 6px; }
QMenu::item { padding: 7px 24px 7px 12px; border-radius: 6px; }
QMenu::item:selected { background: %8; }
QMenu::item:disabled { color: %18; }
QMenu::separator { height: 1px; background: %13; margin: 5px 8px; }
QListWidget, QTreeWidget { background: transparent; color: %2; border: none; }
QListWidget::item { border-radius: 8px; padding: 6px; color: %2; }
QListWidget::item:selected { background: rgba(0,210,255,0.14); color: %2; }
QListWidget::item:hover { background: %8; }
QScrollBar:vertical { background: transparent; width: 9px; margin: 3px; }
QScrollBar::handle:vertical { background: %13; border-radius: 4px; min-height: 28px; }
QScrollBar::handle:vertical:hover { background: %18; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal { background: transparent; height: 9px; margin: 3px; }
QScrollBar::handle:horizontal { background: %13; border-radius: 4px; min-width: 28px; }
QScrollBar::handle:horizontal:hover { background: %18; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
QSlider::groove:horizontal { background: %9; height: 3px; border-radius: 1px; }
QSlider::handle:horizontal { background: %6; width: 13px; height: 13px; margin: -5px 0; border-radius: 6px; }
QSlider::handle:horizontal:hover { background: %10; }
QSlider::sub-page:horizontal { background: %6; border-radius: 1px; }
QProgressBar { background: %9; border: none; border-radius: 3px; height: 6px; color: transparent; }
QProgressBar::chunk { background: %6; border-radius: 3px; }
QSplitter::handle { background: %13; }
QToolBar { background: transparent; border: none; spacing: 4px; }
QCheckBox, QRadioButton { color: %3; spacing: 8px; }
QCheckBox:hover, QRadioButton:hover { color: %2; }
QCheckBox::indicator { width: 15px; height: 15px; border-radius: 4px; border: 1px solid %18; background: %1; }
QCheckBox::indicator:checked { background: %6; border-color: %6; image: none; }
QRadioButton::indicator { width: 14px; height: 14px; border-radius: 7px; border: 1px solid %18; background: %1; }
QRadioButton::indicator:checked { border: 2px solid %6; background: %7; }
QToolTip { background: %4; color: %2; border: 1px solid %5; border-radius: 6px; padding: 5px 9px; font-size: 11px; }
QStatusBar { background: %9; color: %3; border-top: 1px solid %13; }
QSizeGrip { background: transparent; }
QScrollArea { background: transparent; border: none; }
QAbstractScrollArea { background: %1; }
)")
        .arg(hex(ti.bg), hex(ti.text), hex(ti.muted), hex(ti.surface), hex(ti.border), hex(ti.accent),
             hex(ti.accentText), hex(ti.surfaceHover), hex(ti.surface2), hex(ti.accentHover),
             hex(ti.danger), hex(ti.gold), rgba(ti.border), hex(ti.gold), hex(ti.goldLight),
             hex(ti.success), hex(ti.surface3), hex(ti.muted2), hex(ti.accentSolid));
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

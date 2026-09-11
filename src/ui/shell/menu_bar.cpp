#include "menu_bar.h"

#include "ui/tools.h"

#include <QAction>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>

namespace drpdf {

MenuBar::MenuBar(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("MenuBar"));
    setFixedHeight(24);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(4, 0, 4, 0);
    lay->setSpacing(0);

    bar_ = new QMenuBar(this);
    bar_->setNativeMenuBar(false);

    auto* fileMenu = bar_->addMenu(QStringLiteral("File"));
    auto* openAct = fileMenu->addAction(QStringLiteral("Open…"));
    openAct->setShortcut(QKeySequence::Open);
    fileMenu->addSeparator();
    auto* saveAct = fileMenu->addAction(QStringLiteral("Save As…"));
    saveAct->setShortcut(QKeySequence::SaveAs);
    fileMenu->addSeparator();
    auto* exitAct = fileMenu->addAction(QStringLiteral("Exit"));
    exitAct->setShortcut(QKeySequence::Quit);

    auto* editMenu = bar_->addMenu(QStringLiteral("Edit"));
    editMenu->addAction(QStringLiteral("Undo"))->setShortcut(QKeySequence::Undo);
    editMenu->addAction(QStringLiteral("Redo"))->setShortcut(QKeySequence::Redo);
    editMenu->addSeparator();
    editMenu->addAction(QStringLiteral("Cut"))->setShortcut(QKeySequence::Cut);
    editMenu->addAction(QStringLiteral("Copy"))->setShortcut(QKeySequence::Copy);
    editMenu->addAction(QStringLiteral("Paste"))->setShortcut(QKeySequence::Paste);

    auto* viewMenu = bar_->addMenu(QStringLiteral("View"));
    auto* themeAct = viewMenu->addAction(QStringLiteral("Toggle Dark / Light"));
    themeAct->setShortcut(Qt::CTRL | Qt::Key_T);
    viewMenu->addSeparator();
    viewMenu->addAction(QStringLiteral("Zoom In"))->setShortcut(QKeySequence::ZoomIn);
    viewMenu->addAction(QStringLiteral("Zoom Out"))->setShortcut(QKeySequence::ZoomOut);
    viewMenu->addAction(QStringLiteral("Fit Page"))->setShortcut(Qt::CTRL | Qt::Key_1);
    viewMenu->addAction(QStringLiteral("Actual Size"))->setShortcut(Qt::CTRL | Qt::Key_0);

    auto* docMenu = bar_->addMenu(QStringLiteral("Document"));
    docMenu->addAction(QStringLiteral("Page Thumbnails"));
    docMenu->addAction(QStringLiteral("Bookmarks & Outline"));
    docMenu->addAction(QStringLiteral("Annotations"));
    docMenu->addSeparator();
    docMenu->addAction(QStringLiteral("Attachments"));
    docMenu->addAction(QStringLiteral("Digital Signatures"));

    auto* toolsMenu = bar_->addMenu(QStringLiteral("Tools"));
    toolsMenu->addAction(QStringLiteral("Edit Text"));
    toolsMenu->addAction(QStringLiteral("Annotate"));
    toolsMenu->addAction(QStringLiteral("Watermark"));
    toolsMenu->addAction(QStringLiteral("OCR"));
    toolsMenu->addAction(QStringLiteral("Compress"));

    auto* secMenu = bar_->addMenu(QStringLiteral("Security"));
    secMenu->addAction(QStringLiteral("Protect with AES-256"));
    secMenu->addAction(QStringLiteral("Remove Security"));
    secMenu->addAction(QStringLiteral("Sign Document"));

    auto* helpMenu = bar_->addMenu(QStringLiteral("Help"));
    helpMenu->addAction(QStringLiteral("About Dr PDF"));
    helpMenu->addAction(QStringLiteral("Keyboard Shortcuts"));

    lay->addWidget(bar_);
    lay->addStretch();
}

} // namespace drpdf
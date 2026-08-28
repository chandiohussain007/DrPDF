#include "main_window.h"

#include "app/settings.h"
#include "services/thumbnails.h"
#include "ui/icons/icons.h"
#include "ui/theme/theme.h"
#include "ui/views/coming_soon_view.h"
#include "ui/views/command_palette.h"
#include "ui/views/compress_view.h"
#include "ui/views/create_view.h"
#include "ui/views/home_view.h"
#include "ui/views/images_view.h"
#include "ui/views/merge_view.h"
#include "ui/views/organize_view.h"
#include "ui/views/protect_view.h"
#include "ui/views/split_view.h"
#include "ui/views/viewer_view.h"

#include <QApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMimeData>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolButton>
#include <QVBoxLayout>

namespace drpdf {
namespace {

bool allPdf(const QStringList& paths) {
    if (paths.isEmpty()) {
        return false;
    }
    for (const auto& p : paths) {
        if (!p.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive)) {
            return false;
        }
    }
    return true;
}

bool allImages(const QStringList& paths) {
    if (paths.isEmpty()) {
        return false;
    }
    static const QStringList ext{".png", ".jpg", ".jpeg", ".bmp", ".tif", ".tiff", ".webp", ".heic"};
    for (const auto& p : paths) {
        bool ok = false;
        for (const auto& e : ext) {
            if (p.endsWith(e, Qt::CaseInsensitive)) {
                ok = true;
            }
        }
        if (!ok) {
            return false;
        }
    }
    return true;
}

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Dr PDF"));
    setWindowIcon(Icons::app(64));
    setMinimumSize(1100, 720);
    resize(1280, 840);
    setAcceptDrops(true);

    thumbs_ = new ThumbnailCache(this);

    auto* root = new QWidget(this);
    root->setObjectName(QStringLiteral("Root"));
    auto* outer = new QVBoxLayout(root);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto* top = new QWidget(root);
    auto* topLay = new QHBoxLayout(top);
    topLay->setContentsMargins(16, 10, 16, 10);
    topLay->setSpacing(8);

    back_ = new QToolButton(top);
    back_->setIcon(Icons::named(QStringLiteral("back"), Theme::instance().tokens().text, 22));
    back_->setToolTip(QStringLiteral("Home"));
    back_->setVisible(false);
    connect(back_, &QToolButton::clicked, this, [this] { showTool(Tool::Home); });

    auto* brand = new QLabel(QStringLiteral("Dr PDF"), top);
    QFont bf = brand->font();
    bf.setPixelSize(15);
    bf.setWeight(QFont::DemiBold);
    brand->setFont(bf);

    title_ = new QLabel(top);
    title_->setProperty("muted", true);

    auto* openBtn = new QToolButton(top);
    openBtn->setText(QStringLiteral("Open"));
    openBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    openBtn->setIcon(Icons::named(QStringLiteral("open"), Theme::instance().tokens().accent, 18));
    connect(openBtn, &QToolButton::clicked, this, [this] {
        const auto files = QFileDialog::getOpenFileNames(
            this, QStringLiteral("Open"), AppSettings::instance().lastDirectory(),
            QStringLiteral("PDF and images (*.pdf *.png *.jpg *.jpeg *.bmp *.tif *.webp);;PDF (*.pdf)"));
        openPaths(files);
    });

    auto* paletteBtn = new QToolButton(top);
    paletteBtn->setText(QStringLiteral("Ctrl+K"));
    paletteBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    paletteBtn->setIcon(Icons::named(QStringLiteral("search"), Theme::instance().tokens().muted, 18));
    connect(paletteBtn, &QToolButton::clicked, this, [this] {
        CommandPalette pal(this);
        connect(&pal, &CommandPalette::toolChosen, this, &MainWindow::showTool);
        pal.exec();
    });

    auto* themeBtn = new QToolButton(top);
    themeBtn->setToolTip(QStringLiteral("Toggle theme"));
    auto setThemeIcon = [themeBtn] {
        const bool dark = Theme::instance().dark();
        themeBtn->setIcon(Icons::named(dark ? QStringLiteral("sun") : QStringLiteral("moon"),
                                       Theme::instance().tokens().text, 20));
    };
    setThemeIcon();
    connect(themeBtn, &QToolButton::clicked, this, [this, setThemeIcon] {
        Theme::instance().toggle();
        AppSettings::instance().setDarkTheme(Theme::instance().dark());
        applyTheme();
        setThemeIcon();
        back_->setIcon(Icons::named(QStringLiteral("back"), Theme::instance().tokens().text, 22));
    });

    topLay->addWidget(back_);
    topLay->addWidget(brand);
    topLay->addSpacing(12);
    topLay->addWidget(title_, 1);
    topLay->addWidget(paletteBtn);
    topLay->addWidget(openBtn);
    topLay->addWidget(themeBtn);

    stack_ = new QStackedWidget(root);
    home_ = new HomeView(stack_);
    create_ = new CreateView(stack_);
    images_ = new ImagesView(stack_);
    merge_ = new MergeView(thumbs_, stack_);
    split_ = new SplitView(thumbs_, stack_);
    organize_ = new OrganizeView(thumbs_, stack_);
    compress_ = new CompressView(stack_);
    protect_ = new ProtectView(stack_);
    viewer_ = new ViewerView(stack_);
    auto* edit = new ComingSoonView(
        Tool::Edit, QStringLiteral("Milestone 2"),
        QStringLiteral("Click-to-edit existing text blocks, add/remove text boxes, replace images, redact."),
        stack_);
    auto* sign = new ComingSoonView(
        Tool::Sign, QStringLiteral("Milestone 4"),
        QStringLiteral("Draw, type, or stamp a signature. Optional local PKCS#12 certificate."),
        stack_);
    auto* ocr = new ComingSoonView(
        Tool::Ocr, QStringLiteral("Milestone 4"),
        QStringLiteral("Tesseract OCR, fully offline, optional language packs."), stack_);
    auto* annotate = new ComingSoonView(
        Tool::Annotate, QStringLiteral("Milestone 2"),
        QStringLiteral("Highlight, comment, and freehand on the PDF annotation layer."), stack_);
    auto* watermark = new ComingSoonView(
        Tool::Watermark, QStringLiteral("Milestone 2"),
        QStringLiteral("Batch watermark, page numbers, headers and footers."), stack_);

    stack_->addWidget(home_);        // 0
    stack_->addWidget(create_);      // 1
    stack_->addWidget(images_);      // 2
    stack_->addWidget(merge_);       // 3
    stack_->addWidget(split_);       // 4
    stack_->addWidget(organize_);    // 5
    stack_->addWidget(compress_);    // 6
    stack_->addWidget(protect_);     // 7
    stack_->addWidget(viewer_);      // 8
    stack_->addWidget(edit);         // 9
    stack_->addWidget(sign);         // 10
    stack_->addWidget(ocr);          // 11
    stack_->addWidget(annotate);     // 12
    stack_->addWidget(watermark);    // 13

    connect(home_, &HomeView::toolChosen, this, &MainWindow::showTool);
    connect(home_, &HomeView::filesDropped, this, &MainWindow::routeDropped);
    auto openViewer = [this](const QString& path) {
        viewer_->openFile(path);
        showTool(Tool::Viewer);
    };
    connect(merge_, &MergeView::exported, this, openViewer);
    connect(split_, &SplitView::exported, this, openViewer);
    connect(organize_, &OrganizeView::exported, this, openViewer);
    connect(create_, &CreateView::exported, this, openViewer);
    connect(images_, &ImagesView::exported, this, openViewer);
    connect(protect_, &ProtectView::exported, this, openViewer);
    connect(compress_, &CompressView::exported, this, openViewer);

    outer->addWidget(top);
    outer->addWidget(stack_, 1);
    setCentralWidget(root);
    statusBar()->showMessage(QStringLiteral("Offline · no telemetry · no account"));
    showTool(Tool::Home);
}

void MainWindow::applyTheme() {
    if (auto* app = qobject_cast<QApplication*>(QApplication::instance())) {
        Theme::instance().apply(*app);
    }
}

void MainWindow::showTool(Tool tool) {
    int index = 0;
    switch (tool) {
    case Tool::Home:
        index = 0;
        break;
    case Tool::Create:
        index = 1;
        break;
    case Tool::Images:
        index = 2;
        break;
    case Tool::Merge:
        index = 3;
        break;
    case Tool::Split:
        index = 4;
        break;
    case Tool::Organize:
        index = 5;
        break;
    case Tool::Compress:
        index = 6;
        break;
    case Tool::Protect:
        index = 7;
        break;
    case Tool::Viewer:
        index = 8;
        break;
    case Tool::Edit:
        index = 9;
        break;
    case Tool::Sign:
        index = 10;
        break;
    case Tool::Ocr:
        index = 11;
        break;
    case Tool::Annotate:
        index = 12;
        break;
    case Tool::Watermark:
        index = 13;
        break;
    }
    stack_->setCurrentIndex(index);
    back_->setVisible(tool != Tool::Home);
    title_->setText(tool == Tool::Home ? QString() : toolTitle(tool));
}

void MainWindow::openPaths(const QStringList& paths) { routeDropped(paths); }

void MainWindow::routeDropped(const QStringList& paths) {
    if (paths.isEmpty()) {
        return;
    }
    if (allImages(paths)) {
        images_->addImages(paths);
        showTool(Tool::Images);
        return;
    }
    if (allPdf(paths) && paths.size() > 1) {
        merge_->addFiles(paths);
        showTool(Tool::Merge);
        return;
    }
    if (allPdf(paths) && paths.size() == 1) {
        viewer_->openFile(paths.first());
        showTool(Tool::Viewer);
        return;
    }
    if (allPdf(QStringList{paths.first()})) {
        viewer_->openFile(paths.first());
        showTool(Tool::Viewer);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    QStringList paths;
    for (const auto& url : event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            paths << url.toLocalFile();
        }
    }
    routeDropped(paths);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_K) {
        CommandPalette pal(this);
        connect(&pal, &CommandPalette::toolChosen, this, &MainWindow::showTool);
        pal.exec();
        return;
    }
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_O) {
        const auto files = QFileDialog::getOpenFileNames(
            this, QStringLiteral("Open"), AppSettings::instance().lastDirectory(),
            QStringLiteral("PDF files (*.pdf)"));
        openPaths(files);
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        showTool(Tool::Home);
        return;
    }
    QMainWindow::keyPressEvent(event);
}

} // namespace drpdf

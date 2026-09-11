#pragma once

#include "ui/tools.h"

#include <QMainWindow>
#include <QStringList>

class QStackedWidget;
class QLabel;
class QToolButton;

namespace drpdf {

class ThumbnailCache;
class HomeView;
class MergeView;
class SplitView;
class OrganizeView;
class CreateView;
class ImagesView;
class ProtectView;
class CompressView;
class ViewerView;
class EditView;
class AnnotateView;
class WatermarkView;
class SignView;
class OcrView;
class Sidebar;
class TitleBar;
class MenuBar;
class RibbonBar;
class IconRail;
class StatusBar;



class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

    void openPaths(const QStringList& paths);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void showTool(Tool tool);
    void applyTheme();
    void routeDropped(const QStringList& paths);

    QStackedWidget* stack_ = nullptr;
    QLabel* title_ = nullptr;
    QToolButton* back_ = nullptr;
    ThumbnailCache* thumbs_ = nullptr;
    Sidebar* sidebar_ = nullptr;

    TitleBar* titleBar_ = nullptr;
    MenuBar* menuBar_ = nullptr;
    RibbonBar* ribbonBar_ = nullptr;
    IconRail* iconRail_ = nullptr;
    StatusBar* statusBar_ = nullptr;

    HomeView* home_ = nullptr;
    MergeView* merge_ = nullptr;
    SplitView* split_ = nullptr;
    OrganizeView* organize_ = nullptr;
    CreateView* create_ = nullptr;
    ImagesView* images_ = nullptr;
    ProtectView* protect_ = nullptr;
    CompressView* compress_ = nullptr;
    ViewerView* viewer_ = nullptr;
    EditView* edit_ = nullptr;
    AnnotateView* annotate_ = nullptr;
    WatermarkView* watermark_ = nullptr;
    SignView* sign_ = nullptr;
    OcrView* ocr_ = nullptr;
};



} // namespace drpdf
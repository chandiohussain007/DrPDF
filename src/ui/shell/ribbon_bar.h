#pragma once

#include "ui/tools.h"

#include <QWidget>

class QHBoxLayout;
class QToolButton;
class QLabel;

namespace drpdf {

class RibbonBar : public QWidget {
    Q_OBJECT
public:
    explicit RibbonBar(QWidget* parent = nullptr);

    void setCurrentTool(Tool tool);
    void setDocumentOpen(bool open);

signals:
    void toolChosen(Tool tool);
    void quickSignRequested();
    void mergeRequested();
    void splitRequested();
    void rotateRequested();
    void openRequested();

private:
    QToolButton* addTab(const QString& label, Tool tool);
    void rebuildContextBar();
    QWidget* buildEditContext();
    QWidget* buildOrganizeContext();
    QWidget* buildSecurityContext();
    QWidget* buildOcrContext();
    QWidget* buildCompressContext();
    QWidget* buildHomeContext();

    Tool current_ = Tool::Home;
    QHBoxLayout* tabs_ = nullptr;
    QWidget* contextHost_ = nullptr;
    QHBoxLayout* contextLay_ = nullptr;
    QVector<QToolButton*> tabButtons_;
    bool docOpen_ = false;
    QToolButton* quickSign_ = nullptr;
};

} // namespace drpdf
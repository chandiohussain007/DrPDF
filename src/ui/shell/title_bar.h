#pragma once

#include <QWidget>

class QLabel;
class QToolButton;

namespace drpdf {

class TitleBar : public QWidget {
    Q_OBJECT
public:
    explicit TitleBar(QWidget* parent = nullptr);

    void setDocumentName(const QString& name);
    void setLocalBadgeVisible(bool on);

signals:
    void openRequested();
    void commandPaletteRequested();

private:
    QLabel* docName_ = nullptr;
    QLabel* badge_ = nullptr;
};

} // namespace drpdf
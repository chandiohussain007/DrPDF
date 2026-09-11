#pragma once

#include <QWidget>

class QButtonGroup;
class QLineEdit;
class QFrame;
class QListWidget;

namespace drpdf {

class AssemblyDock : public QWidget {
    Q_OBJECT
public:
    explicit AssemblyDock(QWidget* parent = nullptr);

signals:
    void extractSelected();
    void extractByRange(const QString& range);
    void extractEveryN(int n);
    void mergeRequested();
    void addFileRequested();

private:
    QButtonGroup* modeGroup_ = nullptr;
    QLineEdit* rangeInput_ = nullptr;
    QListWidget* queue_ = nullptr;
};

} // namespace drpdf
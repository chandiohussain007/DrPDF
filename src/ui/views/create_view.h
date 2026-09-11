#pragma once

#include <QWidget>

class QTextEdit;
class QFontComboBox;
class QSpinBox;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QToolButton;
class QLabel;
class QCheckBox;
class QTimer;
class QScrollArea;

namespace drpdf {

class Banner;

class CreateView : public QWidget {
    Q_OBJECT
public:
    explicit CreateView(QWidget* parent = nullptr);

signals:
    void exported(const QString& path);

private:
    void exportPdf();
    void saveDraft();
    void openDraft();
    void autosave();
    void maybeRecover();
    void applyHeading(int index);
    void insertTable();
    void insertPageBreak();
    void insertImage();
    void toggleList(bool numbered);
    void setAlign(Qt::Alignment align);
    void setColor(bool background);
    void syncToolbar();
    void applyPageMetrics();
    void updateStats();
    QString autosavePath() const;

    QTextEdit* editor_ = nullptr;
    QWidget* paper_ = nullptr;
    QScrollArea* scroller_ = nullptr;
    QFontComboBox* font_ = nullptr;
    QSpinBox* size_ = nullptr;
    QComboBox* heading_ = nullptr;
    QComboBox* pageSize_ = nullptr;
    QComboBox* orientation_ = nullptr;
    QDoubleSpinBox* margin_ = nullptr;
    QLineEdit* header_ = nullptr;
    QLineEdit* footer_ = nullptr;
    QCheckBox* pageNumbers_ = nullptr;
    QToolButton* bold_ = nullptr;
    QToolButton* italic_ = nullptr;
    QToolButton* underline_ = nullptr;
    QLabel* stats_ = nullptr;
    Banner* banner_ = nullptr;
    QTimer* autosaveTimer_ = nullptr;
    bool syncing_ = false;
};

} // namespace drpdf

#pragma once

#include <QStringList>
#include <QWidget>

namespace drpdf {

class DropZone : public QWidget {
    Q_OBJECT
public:
    explicit DropZone(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setSubtitle(const QString& subtitle);
    void setAcceptImages(bool on) { acceptImages_ = on; }
    void setAcceptPdf(bool on) { acceptPdf_ = on; }

signals:
    void filesDropped(const QStringList& paths);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QStringList filter(const QStringList& in) const;
    QString title_ = QStringLiteral("Drop files here");
    QString subtitle_ = QStringLiteral("or click to browse");
    bool hover_ = false;
    bool acceptPdf_ = true;
    bool acceptImages_ = false;
};

} // namespace drpdf

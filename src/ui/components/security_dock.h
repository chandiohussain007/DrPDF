#pragma once

#include <QWidget>

class QLineEdit;
class QCheckBox;
class QButtonGroup;
class QStackedWidget;

namespace drpdf {

class SecurityDock : public QWidget {
    Q_OBJECT
public:
    explicit SecurityDock(QWidget* parent = nullptr);

    QString password() const;
    bool hasPassword() const;
    bool preventEdit() const;
    bool restrictPrinting() const;
    bool preventCopy() const;
    bool preventRearrange() const;

signals:
    void signRequested();
    void encryptRequested();
    void signatureModeChanged(int mode);

private:
    QStackedWidget* sigStack_ = nullptr;
    QLineEdit* password_ = nullptr;
    QCheckBox* editCheck_ = nullptr;
    QCheckBox* printCheck_ = nullptr;
    QCheckBox* copyCheck_ = nullptr;
    QCheckBox* rearrangeCheck_ = nullptr;
};

} // namespace drpdf
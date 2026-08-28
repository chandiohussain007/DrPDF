#include "protect_view.h"

#include "app/settings.h"
#include "core/assembly.h"
#include "ui/components/banner.h"
#include "ui/components/drop_zone.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace drpdf {

ProtectView::ProtectView(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    auto* title = new QLabel(QStringLiteral("Protect"), this);
    QFont f = title->font();
    f.setPixelSize(22);
    f.setWeight(QFont::DemiBold);
    title->setFont(f);
    auto* hint = new QLabel(
        QStringLiteral("AES-256 encryption via QPDF. Passwords never leave this machine."), this);
    hint->setProperty("muted", true);
    hint->setWordWrap(true);

    drop_ = new DropZone(this);
    drop_->setTitle(QStringLiteral("Drop a PDF"));
    drop_->setMaximumHeight(120);
    connect(drop_, &DropZone::filesDropped, this, [this](const QStringList& p) {
        if (!p.isEmpty()) {
            loadFile(p.first());
        }
    });

    current_ = new QLineEdit(this);
    current_->setEchoMode(QLineEdit::Password);
    current_->setPlaceholderText(QStringLiteral("Current password if already locked"));
    user_ = new QLineEdit(this);
    user_->setEchoMode(QLineEdit::Password);
    user_->setPlaceholderText(QStringLiteral("User password (required to open)"));
    owner_ = new QLineEdit(this);
    owner_->setEchoMode(QLineEdit::Password);
    owner_->setPlaceholderText(QStringLiteral("Owner password (optional)"));

    auto* form = new QFormLayout();
    form->addRow(QStringLiteral("Current"), current_);
    form->addRow(QStringLiteral("New user"), user_);
    form->addRow(QStringLiteral("New owner"), owner_);

    banner_ = new Banner(this);
    auto* lockBtn = new QPushButton(QStringLiteral("Lock with AES-256"), this);
    lockBtn->setProperty("primary", true);
    auto* unlockBtn = new QPushButton(QStringLiteral("Remove password"), this);
    connect(lockBtn, &QPushButton::clicked, this, &ProtectView::lock);
    connect(unlockBtn, &QPushButton::clicked, this, &ProtectView::unlock);

    auto* row = new QHBoxLayout();
    row->addWidget(unlockBtn);
    row->addStretch();
    row->addWidget(lockBtn);

    root->addWidget(title);
    root->addWidget(hint);
    root->addWidget(drop_);
    root->addLayout(form);
    root->addWidget(banner_);
    root->addLayout(row);
    root->addStretch();
}

void ProtectView::loadFile(const QString& path) {
    path_ = path;
    auto info = core::inspectPdf(std::filesystem::path(path.toStdString()),
                                 current_->text().toStdString());
    if (!info) {
        banner_->showError(QString::fromStdString(info.error()));
        return;
    }
    banner_->showInfo(QFileInfo(path).fileName() +
                      (info.value().encrypted ? QStringLiteral(" — encrypted")
                                              : QStringLiteral(" — not encrypted")));
}

void ProtectView::lock() {
    if (path_.isEmpty()) {
        banner_->showError(QStringLiteral("Drop a PDF first."));
        return;
    }
    if (user_->text().isEmpty()) {
        banner_->showError(QStringLiteral("Set a user password."));
        return;
    }
    const QString out = QFileDialog::getSaveFileName(
        this, QStringLiteral("Save locked PDF"),
        AppSettings::instance().lastDirectory() + QStringLiteral("/locked.pdf"),
        QStringLiteral("PDF files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }
    auto r = core::encryptPdf(std::filesystem::path(path_.toStdString()),
                              std::filesystem::path(out.toStdString()), user_->text().toStdString(),
                              owner_->text().toStdString(), current_->text().toStdString());
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
    } else {
        banner_->showInfo(QStringLiteral("Saved ") + out);
        emit exported(out);
    }
}

void ProtectView::unlock() {
    if (path_.isEmpty()) {
        banner_->showError(QStringLiteral("Drop a PDF first."));
        return;
    }
    const QString out = QFileDialog::getSaveFileName(
        this, QStringLiteral("Save unlocked PDF"),
        AppSettings::instance().lastDirectory() + QStringLiteral("/unlocked.pdf"),
        QStringLiteral("PDF files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }
    auto r = core::decryptPdf(std::filesystem::path(path_.toStdString()),
                              std::filesystem::path(out.toStdString()),
                              current_->text().toStdString());
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
    } else {
        banner_->showInfo(QStringLiteral("Saved ") + out);
        emit exported(out);
    }
}

} // namespace drpdf

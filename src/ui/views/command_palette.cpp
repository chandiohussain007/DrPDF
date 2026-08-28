#include "command_palette.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>

namespace drpdf {

CommandPalette::CommandPalette(QWidget* parent) : QDialog(parent) {
    setWindowTitle(QStringLiteral("Command palette"));
    setModal(true);
    setMinimumSize(420, 360);
    auto* lay = new QVBoxLayout(this);
    input_ = new QLineEdit(this);
    input_->setPlaceholderText(QStringLiteral("Jump to a tool…"));
    list_ = new QListWidget(this);
    lay->addWidget(input_);
    lay->addWidget(list_);

    const Tool tools[] = {Tool::Create,   Tool::Images, Tool::Merge,   Tool::Split,
                          Tool::Organize, Tool::Compress, Tool::Protect, Tool::Viewer,
                          Tool::Edit,     Tool::Sign,   Tool::Ocr,     Tool::Annotate,
                          Tool::Watermark};
    for (auto t : tools) {
        auto* it = new QListWidgetItem(toolTitle(t), list_);
        it->setData(Qt::UserRole, static_cast<int>(t));
    }
    connect(input_, &QLineEdit::textChanged, this, &CommandPalette::filter);
    connect(list_, &QListWidget::itemActivated, this, [this](QListWidgetItem* it) {
        emit toolChosen(static_cast<Tool>(it->data(Qt::UserRole).toInt()));
        accept();
    });
}

void CommandPalette::filter(const QString& text) {
    for (int i = 0; i < list_->count(); ++i) {
        auto* it = list_->item(i);
        it->setHidden(!it->text().contains(text, Qt::CaseInsensitive));
    }
}

} // namespace drpdf

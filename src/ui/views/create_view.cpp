#include "create_view.h"

#include "app/settings.h"
#include "services/rich_pdf.h"
#include "ui/components/banner.h"

#include <QFileDialog>
#include <QFontComboBox>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextList>
#include <QUrl>
#include <QVBoxLayout>



namespace drpdf {

CreateView::CreateView(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);

    auto* title = new QLabel(QStringLiteral("Create PDF"), this);
    QFont f = title->font();
    f.setPixelSize(22);
    f.setWeight(QFont::DemiBold);
    title->setFont(f);

    auto* bar = new QHBoxLayout();
    font_ = new QFontComboBox(this);
    size_ = new QSpinBox(this);
    size_->setRange(8, 72);
    size_->setValue(12);
    auto* bold = new QPushButton(QStringLiteral("B"), this);
    auto* italic = new QPushButton(QStringLiteral("I"), this);
    auto* ul = new QPushButton(QStringLiteral("List"), this);
    auto* image = new QPushButton(QStringLiteral("Image"), this);
    auto* exportBtn = new QPushButton(QStringLiteral("Export PDF"), this);
    exportBtn->setProperty("primary", true);
    bar->addWidget(font_);
    bar->addWidget(size_);
    bar->addWidget(bold);
    bar->addWidget(italic);
    bar->addWidget(ul);
    bar->addWidget(image);
    bar->addStretch();
    bar->addWidget(exportBtn);

    editor_ = new QTextEdit(this);
    editor_->setPlaceholderText(QStringLiteral("Start writing… headings, lists, images. Export writes a real PDF on disk."));
    editor_->setFontPointSize(12);

    banner_ = new Banner(this);

    connect(font_, &QFontComboBox::currentFontChanged, this, [this](const QFont& font) {
        QTextCharFormat fmt;
        fmt.setFontFamily(font.family());
        editor_->mergeCurrentCharFormat(fmt);
    });
    connect(size_, &QSpinBox::valueChanged, this, [this](int v) { editor_->setFontPointSize(v); });
    connect(bold, &QPushButton::clicked, this, [this] {
        QTextCharFormat fmt;
        fmt.setFontWeight(editor_->fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
        editor_->mergeCurrentCharFormat(fmt);
    });
    connect(italic, &QPushButton::clicked, this, [this] {
        QTextCharFormat fmt;
        fmt.setFontItalic(!editor_->fontItalic());
        editor_->mergeCurrentCharFormat(fmt);
    });
    connect(ul, &QPushButton::clicked, this, [this] {
        auto cursor = editor_->textCursor();
        auto* list = cursor.currentList();
        if (list) {
            list->remove(cursor.block());
        } else {
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDisc);
            cursor.createList(lf);
        }
    });
    connect(image, &QPushButton::clicked, this, [this] {
        const auto path = QFileDialog::getOpenFileName(
            this, QStringLiteral("Insert image"), AppSettings::instance().lastDirectory(),
            QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp *.webp)"));
        if (!path.isEmpty()) {
            QImage img(path);
            if (!img.isNull()) {
                const QUrl url = QUrl::fromLocalFile(path);
                editor_->document()->addResource(QTextDocument::ImageResource, url, img);
                editor_->textCursor().insertImage(url.toString());
            }
        }

    });
    connect(exportBtn, &QPushButton::clicked, this, &CreateView::exportPdf);

    root->addWidget(title);
    root->addLayout(bar);
    root->addWidget(editor_, 1);
    root->addWidget(banner_);
}

void CreateView::exportPdf() {
    const QString out = QFileDialog::getSaveFileName(
        this, QStringLiteral("Export PDF"),
        AppSettings::instance().lastDirectory() + QStringLiteral("/document.pdf"),
        QStringLiteral("PDF files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }
    auto r = writeTextDocumentToPdf(editor_->document(), out);
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
    } else {
        banner_->showInfo(QStringLiteral("Saved ") + out);
        AppSettings::instance().addRecentFile(out);
        emit exported(out);
    }
}

} // namespace drpdf

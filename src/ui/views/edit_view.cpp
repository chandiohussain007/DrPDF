#include "edit_view.h"

#include "app/settings.h"
#include "core/overlay.h"
#include "core/text_edit.h"
#include "ui/components/banner.h"
#include "ui/components/drop_zone.h"
#include "ui/components/page_canvas.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImage>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPdfDocument>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>

#include <algorithm>
#include <cstring>



namespace drpdf {

EditView::EditView(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    auto* title = new QLabel(QStringLiteral("Edit text"), this);
    QFont f = title->font();
    f.setPixelSize(22);
    f.setWeight(QFont::DemiBold);
    title->setFont(f);
    hint_ = new QLabel(
        QStringLiteral("Block-level replace on simple fonts (WinAnsi/Helvetica). CID/custom "
                       "encodings stay listed but may not rewrite cleanly. Longer replacements "
                       "do not reflow across the page."),
        this);
    hint_->setWordWrap(true);
    hint_->setProperty("muted", true);

    drop_ = new DropZone(this);
    drop_->setTitle(QStringLiteral("Drop a PDF to edit"));
    drop_->setMaximumHeight(110);
    connect(drop_, &DropZone::filesDropped, this, [this](const QStringList& p) {
        if (!p.isEmpty()) {
            loadFile(p.first());
        }
    });

    auto* tools = new QHBoxLayout();
    page_ = new QSpinBox(this);
    page_->setMinimum(1);
    page_->setPrefix(QStringLiteral("Page "));
    fontSize_ = new QSpinBox(this);
    fontSize_->setRange(6, 96);
    fontSize_->setValue(12);
    fontSize_->setSuffix(QStringLiteral(" pt"));
    auto* modeText = new QPushButton(QStringLiteral("Place text"), this);
    auto* modeImage = new QPushButton(QStringLiteral("Place image"), this);
    auto* modeRedact = new QPushButton(QStringLiteral("Redact"), this);
    auto* save = new QPushButton(QStringLiteral("Save as…"), this);
    save->setProperty("primary", true);
    tools->addWidget(page_);
    tools->addWidget(fontSize_);
    tools->addWidget(modeText);
    tools->addWidget(modeImage);
    tools->addWidget(modeRedact);
    tools->addStretch();
    tools->addWidget(save);

    canvas_ = new PageCanvas(this);
    runs_ = new QListWidget(this);
    runs_->setMinimumWidth(280);
    editor_ = new QLineEdit(this);
    editor_->setPlaceholderText(QStringLiteral("Selected text run"));
    auto* replace = new QPushButton(QStringLiteral("Replace run"), this);

    auto* right = new QWidget(this);
    auto* rightLay = new QVBoxLayout(right);
    rightLay->setContentsMargins(0, 0, 0, 0);
    auto* listLabel = new QLabel(QStringLiteral("Text runs on this page"), right);
    rightLay->addWidget(listLabel);
    rightLay->addWidget(runs_, 1);
    rightLay->addWidget(editor_);
    rightLay->addWidget(replace);

    auto* split = new QSplitter(this);
    split->addWidget(canvas_);
    split->addWidget(right);
    split->setStretchFactor(0, 3);
    split->setStretchFactor(1, 2);

    banner_ = new Banner(this);

    connect(page_, &QSpinBox::valueChanged, this, [this](int) { reload(); });
    connect(runs_, &QListWidget::currentRowChanged, this, [this](int row) {
        if (auto* it = runs_->item(row)) {
            editor_->setText(it->data(Qt::UserRole + 1).toString());
        }
    });
    connect(replace, &QPushButton::clicked, this, &EditView::replaceSelected);
    connect(modeText, &QPushButton::clicked, this, [this] {
        canvas_->setProperty("op", QStringLiteral("text"));
        canvas_->setMode(PageCanvas::Mode::Place);
        banner_->showInfo(QStringLiteral("Click the page to place a new text box."));
    });
    connect(modeImage, &QPushButton::clicked, this, [this] {
        canvas_->setProperty("op", QStringLiteral("image"));
        canvas_->setMode(PageCanvas::Mode::Rect);
        banner_->showInfo(QStringLiteral("Drag a rectangle, then pick an image."));
    });
    connect(modeRedact, &QPushButton::clicked, this, [this] {
        canvas_->setProperty("op", QStringLiteral("redact"));
        canvas_->setMode(PageCanvas::Mode::Rect);
        banner_->showInfo(
            QStringLiteral("Drag a rectangle to redact. Text whose origin falls in the box is removed."));
    });
    connect(canvas_, &PageCanvas::placed, this, &EditView::addTextAt);
    connect(canvas_, &PageCanvas::rectDrawn, this, [this](QRectF pdf) {
        const QString op = canvas_->property("op").toString();
        if (op == QLatin1String("image")) {
            addImageAt(pdf);
        } else if (op == QLatin1String("redact")) {
            redactAt(pdf);
        }
    });
    connect(save, &QPushButton::clicked, this, &EditView::saveAs);


    root->addWidget(title);
    root->addWidget(hint_);
    root->addWidget(drop_);
    root->addLayout(tools);
    root->addWidget(split, 1);
    root->addWidget(banner_);
}

void EditView::loadFile(const QString& path) {
    if (!working_.load(path)) {
        banner_->showError(QStringLiteral("Could not open file."));
        return;
    }
    AppSettings::instance().addRecentFile(path);
    QPdfDocument doc;
    if (doc.load(working_.currentPath()) != QPdfDocument::Error::None) {
        banner_->showError(QStringLiteral("Preview failed to load."));
        return;
    }
    page_->setMaximum(std::max(1, doc.pageCount()));
    page_->setValue(1);
    canvas_->setMode(PageCanvas::Mode::None);
    reload();
    banner_->showInfo(QFileInfo(path).fileName());
}

void EditView::reload() {
    if (!working_.isOpen()) {
        return;
    }
    QPdfDocument doc;
    if (doc.load(working_.currentPath()) != QPdfDocument::Error::None) {
        return;
    }
    const int page = std::clamp(page_->value() - 1, 0, std::max(0, doc.pageCount() - 1));
    page_->setMaximum(std::max(1, doc.pageCount()));
    const QSizeF pts = doc.pagePointSize(page);
    const QSize px(std::max(400, int(pts.width() * 1.4)), std::max(500, int(pts.height() * 1.4)));
    canvas_->setPage(doc.render(page, px), pts);
    refreshRuns();
}

void EditView::refreshRuns() {
    runs_->clear();
    if (!working_.isOpen()) {
        return;
    }
    auto extracted = core::extractTextRuns(std::filesystem::path(working_.currentPath().toStdString()));
    if (!extracted) {
        banner_->showError(QString::fromStdString(extracted.error()));
        return;
    }
    const int page = page_->value() - 1;
    for (const auto& run : extracted.value()) {
        if (run.page != page) {
            continue;
        }
        auto* it = new QListWidgetItem(
            QStringLiteral("p.%1  %2").arg(run.index + 1).arg(QString::fromStdString(run.text)),
            runs_);
        it->setData(Qt::UserRole, run.index);
        it->setData(Qt::UserRole + 1, QString::fromStdString(run.text));
        it->setToolTip(QStringLiteral("font %1  %2 pt  (%3, %4)")
                           .arg(QString::fromStdString(run.font))
                           .arg(run.size)
                           .arg(run.x, 0, 'f', 1)
                           .arg(run.y, 0, 'f', 1));
    }
}

void EditView::replaceSelected() {
    auto* it = runs_->currentItem();
    if (!it) {
        banner_->showError(QStringLiteral("Select a text run first."));
        return;
    }
    const int idx = it->data(Qt::UserRole).toInt();
    const QString text = editor_->text();
    auto r = working_.apply([&](const auto& in, const auto& out) {
        return core::replaceTextRun(in, out, page_->value() - 1, idx, text.toStdString());
    });
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
        return;
    }
    reload();
    banner_->showInfo(QStringLiteral("Replaced text run."));
}

void EditView::addTextAt(QPointF pdf) {
    if (canvas_->mode() != PageCanvas::Mode::Place) {
        return;
    }
    bool ok = false;
    const QString text = QInputDialog::getText(this, QStringLiteral("New text"),
                                               QStringLiteral("Text"), QLineEdit::Normal,
                                               QString(), &ok);
    if (!ok || text.isEmpty()) {
        return;
    }
    core::NewTextBox box;
    box.page = page_->value() - 1;
    box.text = text.toStdString();
    box.x = pdf.x();
    box.y = pdf.y();
    box.fontSize = fontSize_->value();
    auto r = working_.apply([&](const auto& in, const auto& out) { return core::addTextBox(in, out, box); });
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
        return;
    }
    canvas_->setMode(PageCanvas::Mode::None);
    reload();
}

void EditView::addImageAt(QRectF pdf) {
    const QString path = QFileDialog::getOpenFileName(

        this, QStringLiteral("Image"), AppSettings::instance().lastDirectory(),
        QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp *.webp *.tif)"));
    if (path.isEmpty()) {
        return;
    }
    QImage img(path);
    if (img.isNull()) {
        banner_->showError(QStringLiteral("Could not read image."));
        return;
    }
    img = img.convertToFormat(QImage::Format_RGB888);
    core::ImageStamp stamp;
    stamp.page = page_->value() - 1;
    stamp.x = pdf.x();
    stamp.y = pdf.y();
    stamp.width = pdf.width();
    stamp.height = pdf.height();
    stamp.image.width = img.width();
    stamp.image.height = img.height();
    stamp.image.components = 3;
    stamp.image.samples.resize(static_cast<size_t>(img.width()) * static_cast<size_t>(img.height()) * 3);
    for (int y = 0; y < img.height(); ++y) {
        memcpy(stamp.image.samples.data() + static_cast<size_t>(y) * img.width() * 3,
               img.constScanLine(y), static_cast<size_t>(img.width()) * 3);
    }
    auto r = working_.apply([&](const auto& in, const auto& out) { return core::stampImage(in, out, stamp); });
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
        return;
    }
    canvas_->setMode(PageCanvas::Mode::None);
    canvas_->setProperty("op", QString());
    reload();
}

void EditView::redactAt(QRectF pdf) {
    core::RedactBox box;
    box.page = page_->value() - 1;
    box.llx = pdf.x();
    box.lly = pdf.y();
    box.urx = pdf.x() + pdf.width();
    box.ury = pdf.y() + pdf.height();
    auto r = working_.apply([&](const auto& in, const auto& out) {
        return core::redactBoxes(in, out, {box});
    });
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
        return;
    }
    canvas_->setMode(PageCanvas::Mode::None);
    canvas_->setProperty("op", QString());
    reload();
    banner_->showInfo(QStringLiteral("Redacted region. Save as a new file — original is untouched."));
}

void EditView::saveAs() {
    if (!working_.isOpen()) {
        banner_->showError(QStringLiteral("Open a PDF first."));
        return;
    }
    const QString out = QFileDialog::getSaveFileName(
        this, QStringLiteral("Save edited PDF"),
        AppSettings::instance().lastDirectory() + QStringLiteral("/edited.pdf"),
        QStringLiteral("PDF files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }
    auto r = working_.saveAs(out);
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
        return;
    }
    AppSettings::instance().addRecentFile(out);
    emit exported(out);
    banner_->showInfo(QStringLiteral("Saved ") + out);
}

} // namespace drpdf

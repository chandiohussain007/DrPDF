#include "annotate_view.h"

#include "app/settings.h"
#include "core/annotations.h"
#include "ui/components/banner.h"
#include "ui/components/drop_zone.h"
#include "ui/components/page_canvas.h"

#include <QButtonGroup>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPdfDocument>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>

#include <algorithm>


namespace drpdf {

AnnotateView::AnnotateView(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    auto* title = new QLabel(QStringLiteral("Annotate"), this);
    QFont f = title->font();
    f.setPixelSize(22);
    f.setWeight(QFont::DemiBold);
    title->setFont(f);
    auto* hint = new QLabel(
        QStringLiteral("Highlights, comments, ink, and free-text are real PDF annotations — they "
                       "open in other readers. Nothing is uploaded."),
        this);
    hint->setWordWrap(true);
    hint->setProperty("muted", true);

    drop_ = new DropZone(this);
    drop_->setTitle(QStringLiteral("Drop a PDF to annotate"));
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
    auto* hl = new QPushButton(QStringLiteral("Highlight"), this);
    auto* cmt = new QPushButton(QStringLiteral("Comment"), this);
    auto* ink = new QPushButton(QStringLiteral("Ink"), this);
    auto* ft = new QPushButton(QStringLiteral("Free text"), this);
    hl->setCheckable(true);
    cmt->setCheckable(true);
    ink->setCheckable(true);
    ft->setCheckable(true);
    tools_ = new QButtonGroup(this);
    tools_->setExclusive(true);
    tools_->addButton(hl, 0);
    tools_->addButton(cmt, 1);
    tools_->addButton(ink, 2);
    tools_->addButton(ft, 3);
    auto* save = new QPushButton(QStringLiteral("Save as…"), this);
    save->setProperty("primary", true);
    tools->addWidget(page_);
    tools->addWidget(hl);
    tools->addWidget(cmt);
    tools->addWidget(ink);
    tools->addWidget(ft);
    tools->addStretch();
    tools->addWidget(save);

    note_ = new QLineEdit(this);
    note_->setPlaceholderText(QStringLiteral("Comment / free-text contents"));

    canvas_ = new PageCanvas(this);
    list_ = new QListWidget(this);
    list_->setMinimumWidth(260);
    auto* del = new QPushButton(QStringLiteral("Delete selected"), this);
    del->setProperty("danger", true);

    auto* right = new QWidget(this);
    auto* rl = new QVBoxLayout(right);
    rl->setContentsMargins(0, 0, 0, 0);
    rl->addWidget(new QLabel(QStringLiteral("Annotations"), right));
    rl->addWidget(list_, 1);
    rl->addWidget(del);

    auto* split = new QSplitter(this);
    split->addWidget(canvas_);
    split->addWidget(right);
    split->setStretchFactor(0, 3);
    split->setStretchFactor(1, 2);

    banner_ = new Banner(this);

    auto setMode = [this] {
        const int id = tools_->checkedId();
        if (id == 0 || id == 3) {
            canvas_->setMode(PageCanvas::Mode::Rect);
        } else if (id == 1) {
            canvas_->setMode(PageCanvas::Mode::Place);
        } else if (id == 2) {
            canvas_->setMode(PageCanvas::Mode::Ink);
        } else {
            canvas_->setMode(PageCanvas::Mode::None);
        }
    };
    connect(tools_, &QButtonGroup::idClicked, this, [setMode](int) { setMode(); });
    connect(page_, &QSpinBox::valueChanged, this, [this](int) { reload(); });
    connect(canvas_, &PageCanvas::rectDrawn, this, [this](QRectF pdf) {
        const int id = tools_->checkedId();
        if (id == 0) {
            core::HighlightSpec spec;
            spec.page = page_->value() - 1;
            spec.llx = pdf.x();
            spec.lly = pdf.y();
            spec.urx = pdf.x() + pdf.width();
            spec.ury = pdf.y() + pdf.height();
            auto r = working_.apply(
                [&](const auto& in, const auto& out) { return core::addHighlight(in, out, spec); });
            if (!r) {
                banner_->showError(QString::fromStdString(r.error()));
            } else {
                reload();
            }
        } else if (id == 3) {
            core::FreeTextSpec spec;
            spec.page = page_->value() - 1;
            spec.llx = pdf.x();
            spec.lly = pdf.y();
            spec.urx = pdf.x() + pdf.width();
            spec.ury = pdf.y() + pdf.height();
            spec.text = note_->text().isEmpty() ? std::string("Note") : note_->text().toStdString();
            auto r = working_.apply(
                [&](const auto& in, const auto& out) { return core::addFreeText(in, out, spec); });
            if (!r) {
                banner_->showError(QString::fromStdString(r.error()));
            } else {
                reload();
            }
        }
    });
    connect(canvas_, &PageCanvas::placed, this, [this](QPointF pdf) {
        if (tools_->checkedId() != 1) {
            return;
        }
        core::CommentSpec spec;
        spec.page = page_->value() - 1;
        spec.x = pdf.x();
        spec.y = pdf.y();
        spec.text = note_->text().isEmpty() ? std::string("Comment") : note_->text().toStdString();
        auto r = working_.apply(
            [&](const auto& in, const auto& out) { return core::addComment(in, out, spec); });
        if (!r) {
            banner_->showError(QString::fromStdString(r.error()));
        } else {
            reload();
        }
    });
    connect(canvas_, &PageCanvas::strokeDrawn, this, [this](const QVector<QPointF>& pts) {
        if (tools_->checkedId() != 2) {
            return;
        }
        core::InkSpec spec;
        spec.page = page_->value() - 1;
        for (const auto& p : pts) {
            spec.points.emplace_back(p.x(), p.y());
        }
        auto r = working_.apply(
            [&](const auto& in, const auto& out) { return core::addInk(in, out, spec); });
        if (!r) {
            banner_->showError(QString::fromStdString(r.error()));
        } else {
            reload();
        }
    });
    connect(del, &QPushButton::clicked, this, [this] {
        auto* it = list_->currentItem();
        if (!it) {
            return;
        }
        const int page = it->data(Qt::UserRole).toInt();
        const int index = it->data(Qt::UserRole + 1).toInt();
        auto r = working_.apply([&](const auto& in, const auto& out) {
            return core::removeAnnotation(in, out, page, index);
        });
        if (!r) {
            banner_->showError(QString::fromStdString(r.error()));
        } else {
            reload();
        }
    });
    connect(save, &QPushButton::clicked, this, &AnnotateView::saveAs);

    root->addWidget(title);
    root->addWidget(hint);
    root->addWidget(drop_);
    root->addLayout(tools);
    root->addWidget(note_);
    root->addWidget(split, 1);
    root->addWidget(banner_);
    hl->setChecked(true);
    canvas_->setMode(PageCanvas::Mode::Rect);
}

void AnnotateView::loadFile(const QString& path) {
    if (!working_.load(path)) {
        banner_->showError(QStringLiteral("Could not open file."));
        return;
    }
    AppSettings::instance().addRecentFile(path);
    QPdfDocument doc;
    doc.load(working_.currentPath());
    page_->setMaximum(std::max(1, doc.pageCount()));
    page_->setValue(1);
    reload();
}

void AnnotateView::reload() {
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
    refreshList();
}

void AnnotateView::refreshList() {
    list_->clear();
    auto annots = core::listAnnotations(std::filesystem::path(working_.currentPath().toStdString()));
    if (!annots) {
        banner_->showError(QString::fromStdString(annots.error()));
        return;
    }
    for (const auto& a : annots.value()) {
        auto* it = new QListWidgetItem(
            QStringLiteral("p.%1  %2  %3")
                .arg(a.page + 1)
                .arg(QString::fromStdString(a.subtype))
                .arg(QString::fromStdString(a.contents)),
            list_);
        it->setData(Qt::UserRole, a.page);
        it->setData(Qt::UserRole + 1, a.index);
    }
}

void AnnotateView::saveAs() {
    if (!working_.isOpen()) {
        banner_->showError(QStringLiteral("Open a PDF first."));
        return;
    }
    const QString out = QFileDialog::getSaveFileName(
        this, QStringLiteral("Save annotated PDF"),
        AppSettings::instance().lastDirectory() + QStringLiteral("/annotated.pdf"),
        QStringLiteral("PDF files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }
    auto r = working_.saveAs(out);
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
        return;
    }
    emit exported(out);
    banner_->showInfo(QStringLiteral("Saved ") + out);
}

} // namespace drpdf

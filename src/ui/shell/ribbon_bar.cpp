#include "ribbon_bar.h"

#include "ui/icons/icons.h"
#include "ui/theme/theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

namespace drpdf {
namespace {

QToolButton* makeToolBtn(const QString& text, const QString& iconName, Tool tool) {
    auto* btn = new QToolButton();
    btn->setText(text);
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setIcon(Icons::named(iconName, Theme::instance().tokens().muted, 20));
    btn->setObjectName(QStringLiteral("ToolBtn"));
    btn->setProperty("tool", static_cast<int>(tool));
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

QToolButton* makeIconBtn(const QString& iconName, const QString& tip) {
    auto* btn = new QToolButton();
    btn->setIcon(Icons::named(iconName, Theme::instance().tokens().muted, 18));
    btn->setToolTip(tip);
    btn->setObjectName(QStringLiteral("IconBtn"));
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

} // namespace

RibbonBar::RibbonBar(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("Ribbon"));

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // Tab row
    auto* tabRow = new QWidget(this);
    tabRow->setObjectName(QStringLiteral("RibbonTabRow"));
    tabRow->setFixedHeight(36);
    tabs_ = new QHBoxLayout(tabRow);
    tabs_->setContentsMargins(12, 4, 12, 0);
    tabs_->setSpacing(2);

    addTab(QStringLiteral("Home"), Tool::Home);
    addTab(QStringLiteral("Edit & Annotate"), Tool::Edit);
    addTab(QStringLiteral("Organize Pages"), Tool::Organize);
    addTab(QStringLiteral("Protect & Sign"), Tool::Protect);
    addTab(QStringLiteral("OCR & Convert"), Tool::Ocr);
    addTab(QStringLiteral("Compress & Optimize"), Tool::Compress);

    // Quick Sign button on tab row
    quickSign_ = new QToolButton(this);
    quickSign_->setText(QStringLiteral("Quick Sign"));
    quickSign_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    quickSign_->setIcon(Icons::named(QStringLiteral("sign"), Theme::instance().tokens().gold, 16));
    quickSign_->setObjectName(QStringLiteral("GoldBtn"));
    quickSign_->setCursor(Qt::PointingHandCursor);
    quickSign_->setVisible(false);
    connect(quickSign_, &QToolButton::clicked, this, &RibbonBar::quickSignRequested);
    tabs_->addStretch();
    tabs_->addWidget(quickSign_);

    // Context bar host
    contextHost_ = new QWidget(this);
    contextHost_->setObjectName(QStringLiteral("ContextBar"));
    contextHost_->setFixedHeight(56);
    contextLay_ = new QHBoxLayout(contextHost_);
    contextLay_->setContentsMargins(12, 4, 12, 4);
    contextLay_->setSpacing(6);

    lay->addWidget(tabRow);
    lay->addWidget(contextHost_);

    rebuildContextBar();
}

QToolButton* RibbonBar::addTab(const QString& label, Tool tool) {
    auto* btn = new QToolButton();
    btn->setText(label);
    btn->setObjectName(QStringLiteral("RibbonTab"));
    btn->setCheckable(true);
    btn->setAutoExclusive(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setProperty("tool", static_cast<int>(tool));
    connect(btn, &QToolButton::clicked, this, [this, tool] {
        setCurrentTool(tool);
        emit toolChosen(tool);
    });
    tabs_->addWidget(btn);
    tabButtons_.append(btn);
    return btn;
}

void RibbonBar::setCurrentTool(Tool tool) {
    current_ = tool;
    for (auto* btn : tabButtons_) {
        btn->setChecked(btn->property("tool").toInt() == static_cast<int>(tool));
    }
    quickSign_->setVisible(tool != Tool::Home && docOpen_);
    rebuildContextBar();
}

void RibbonBar::setDocumentOpen(bool open) {
    docOpen_ = open;
    quickSign_->setVisible(current_ != Tool::Home && docOpen_);
}

void RibbonBar::rebuildContextBar() {
    // Clear existing context widgets
    while (auto* item = contextLay_->takeAt(0)) {
        if (auto* w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }

    QWidget* ctx = nullptr;
    switch (current_) {
    case Tool::Edit:
    case Tool::Annotate:
        ctx = buildEditContext();
        break;
    case Tool::Organize:
    case Tool::Merge:
    case Tool::Split:
        ctx = buildOrganizeContext();
        break;
    case Tool::Protect:
    case Tool::Sign:
        ctx = buildSecurityContext();
        break;
    case Tool::Ocr:
        ctx = buildOcrContext();
        break;
    case Tool::Compress:
        ctx = buildCompressContext();
        break;
    case Tool::Home:
    case Tool::Create:
    case Tool::Images:
    case Tool::Viewer:
    case Tool::Watermark:
    default:
        ctx = buildHomeContext();
        break;
    }
    if (ctx) {
        // Make every tool button in the context bar actually navigate.
        for (auto* b : ctx->findChildren<QToolButton*>()) {
            if (b->objectName() == QStringLiteral("ToolBtn")) {
                const Tool t = static_cast<Tool>(b->property("tool").toInt());
                connect(b, &QToolButton::clicked, this, [this, t] { emit toolChosen(t); });
            }
        }
        contextLay_->addWidget(ctx);
    }
    contextLay_->addStretch();
}

QWidget* RibbonBar::buildEditContext() {
    auto* w = new QWidget(this);
    auto* lay = new QHBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(6);

    auto* hand = makeIconBtn(QStringLiteral("hand"), QStringLiteral("Hand tool"));
    auto* select = makeIconBtn(QStringLiteral("arrow_selector_tool"), QStringLiteral("Select tool"));
    auto* snapshot = makeIconBtn(QStringLiteral("crop_free"), QStringLiteral("Snapshot"));
    auto* text = makeToolBtn(QStringLiteral("Text"), QStringLiteral("edit"), Tool::Edit);
    auto* highlight = makeToolBtn(QStringLiteral("Highlight"), QStringLiteral("annotate"), Tool::Annotate);
    auto* pencil = makeToolBtn(QStringLiteral("Pencil"), QStringLiteral("draw"), Tool::Annotate);
    auto* image = makeToolBtn(QStringLiteral("Image"), QStringLiteral("images"), Tool::Edit);

    lay->addWidget(hand);
    lay->addWidget(select);
    lay->addWidget(snapshot);
    lay->addSpacing(8);
    lay->addWidget(text);
    lay->addWidget(highlight);
    lay->addWidget(pencil);
    lay->addWidget(image);
    return w;
}

QWidget* RibbonBar::buildOrganizeContext() {
    auto* w = new QWidget(this);
    auto* lay = new QHBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(6);

    auto* insert = makeToolBtn(QStringLiteral("Insert Blank"), QStringLiteral("create"), Tool::Organize);
    auto* fromFile = makeToolBtn(QStringLiteral("From File"), QStringLiteral("open"), Tool::Merge);
    auto* extract = makeToolBtn(QStringLiteral("Extract"), QStringLiteral("split"), Tool::Split);
    auto* del = makeToolBtn(QStringLiteral("Delete"), QStringLiteral("trash"), Tool::Organize);
    auto* rotateL = makeIconBtn(QStringLiteral("rotate_left"), QStringLiteral("Rotate left 90°"));
    auto* rotateR = makeIconBtn(QStringLiteral("rotate_right"), QStringLiteral("Rotate right 90°"));
    auto* mergeBtn = makeToolBtn(QStringLiteral("Merge Files"), QStringLiteral("merge"), Tool::Merge);
    auto* splitBtn = makeToolBtn(QStringLiteral("Split Range"), QStringLiteral("split"), Tool::Split);
    auto* reverseBtn = makeToolBtn(QStringLiteral("Reverse"), QStringLiteral("swap"), Tool::Organize);

    lay->addWidget(insert);
    lay->addWidget(fromFile);
    lay->addWidget(extract);
    lay->addWidget(del);
    lay->addSpacing(8);
    lay->addWidget(rotateL);
    lay->addWidget(rotateR);
    lay->addSpacing(8);
    lay->addWidget(mergeBtn);
    lay->addWidget(splitBtn);
    lay->addWidget(reverseBtn);
    return w;
}

QWidget* RibbonBar::buildSecurityContext() {
    auto* w = new QWidget(this);
    auto* lay = new QHBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(6);

    auto* signBtn = makeToolBtn(QStringLiteral("Sign Document"), QStringLiteral("sign"), Tool::Sign);
    auto* validate = makeToolBtn(QStringLiteral("Validate All"), QStringLiteral("check"), Tool::Protect);
    auto* aes = makeToolBtn(QStringLiteral("AES-256"), QStringLiteral("protect"), Tool::Protect);
    auto* removeSec = makeToolBtn(QStringLiteral("Remove Security"), QStringLiteral("unlock"), Tool::Protect);
    auto* redact = makeToolBtn(QStringLiteral("Redact"), QStringLiteral("redact"), Tool::Edit);
    auto* scrub = makeToolBtn(QStringLiteral("Scrub Metadata"), QStringLiteral("clean"), Tool::Protect);

    lay->addWidget(signBtn);
    lay->addWidget(validate);
    lay->addSpacing(8);
    lay->addWidget(aes);
    lay->addWidget(removeSec);
    lay->addSpacing(8);
    lay->addWidget(redact);
    lay->addWidget(scrub);
    return w;
}

QWidget* RibbonBar::buildOcrContext() {
    auto* w = new QWidget(this);
    auto* lay = new QHBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(6);

    auto* currentPage = makeToolBtn(QStringLiteral("Current Page"), QStringLiteral("ocr"), Tool::Ocr);
    auto* batch = makeToolBtn(QStringLiteral("Batch Document"), QStringLiteral("batch"), Tool::Ocr);
    auto* lang = makeToolBtn(QStringLiteral("Eng + Math (TeX)"), QStringLiteral("translate"), Tool::Ocr);
    auto* accurate = makeToolBtn(QStringLiteral("Accurate (LSTM)"), QStringLiteral("neurology"), Tool::Ocr);
    auto* exportBtn = makeToolBtn(QStringLiteral("Export Searchable PDF"), QStringLiteral("export"), Tool::Ocr);

    lay->addWidget(currentPage);
    lay->addWidget(batch);
    lay->addWidget(lang);
    lay->addWidget(accurate);
    lay->addSpacing(8);
    lay->addWidget(exportBtn);
    return w;
}

QWidget* RibbonBar::buildCompressContext() {
    auto* w = new QWidget(this);
    auto* lay = new QHBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(6);

    auto* lossless = makeToolBtn(QStringLiteral("Lossless"), QStringLiteral("compress"), Tool::Compress);
    auto* web = makeToolBtn(QStringLiteral("Web 150 DPI"), QStringLiteral("compress"), Tool::Compress);
    auto* archive = makeToolBtn(QStringLiteral("Archive 300"), QStringLiteral("compress"), Tool::Compress);
    auto* compact = makeToolBtn(QStringLiteral("Compact 72"), QStringLiteral("compress"), Tool::Compress);
    auto* optimize = makeToolBtn(QStringLiteral("Optimize & Save"), QStringLiteral("save"), Tool::Compress);

    lay->addWidget(lossless);
    lay->addWidget(web);
    lay->addWidget(archive);
    lay->addWidget(compact);
    lay->addSpacing(8);
    lay->addWidget(optimize);
    return w;
}

QWidget* RibbonBar::buildHomeContext() {
    auto* w = new QWidget(this);
    auto* lay = new QHBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(6);

    auto* create = makeToolBtn(QStringLiteral("Create PDF"), QStringLiteral("create"), Tool::Create);
    auto* images = makeToolBtn(QStringLiteral("Images to PDF"), QStringLiteral("images"), Tool::Images);
    auto* merge = makeToolBtn(QStringLiteral("Merge"), QStringLiteral("merge"), Tool::Merge);
    auto* split = makeToolBtn(QStringLiteral("Split"), QStringLiteral("split"), Tool::Split);
    auto* organize = makeToolBtn(QStringLiteral("Organize"), QStringLiteral("organize"), Tool::Organize);
    auto* compress = makeToolBtn(QStringLiteral("Compress"), QStringLiteral("compress"), Tool::Compress);
    auto* protect = makeToolBtn(QStringLiteral("Protect"), QStringLiteral("protect"), Tool::Protect);
    auto* viewer = makeToolBtn(QStringLiteral("Viewer"), QStringLiteral("viewer"), Tool::Viewer);

    lay->addWidget(create);
    lay->addWidget(images);
    lay->addWidget(merge);
    lay->addWidget(split);
    lay->addWidget(organize);
    lay->addWidget(compress);
    lay->addWidget(protect);
    lay->addWidget(viewer);
    return w;
}

} // namespace drpdf
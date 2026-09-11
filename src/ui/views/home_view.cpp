#include "home_view.h"

#include "app/settings.h"
#include "ui/components/drop_zone.h"
#include "ui/components/tool_card.h"
#include "ui/theme/theme.h"

#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QVBoxLayout>

namespace drpdf {

HomeView::HomeView(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Everything below lives inside a scroll area so the tool grid and the recent
    // list are NEVER clipped or hidden behind each other when the window is small.
    auto* scroller = new QScrollArea(this);
    scroller->setWidgetResizable(true);
    scroller->setFrameShape(QFrame::NoFrame);
    scroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* content = new QWidget(scroller);
    auto* inner = new QVBoxLayout(content);
    inner->setContentsMargins(32, 24, 32, 24);
    inner->setSpacing(18);

    auto* kicker = new QLabel(QStringLiteral("LOCAL-FIRST PDF WORKSPACE"), this);
    kicker->setProperty("muted", true);
    QFont kf = kicker->font();
    kf.setPixelSize(11);
    kf.setWeight(QFont::DemiBold);
    kf.setLetterSpacing(QFont::AbsoluteSpacing, 1.4);
    kicker->setFont(kf);

    auto* title = new QLabel(QStringLiteral("Dr PDF"), this);
    title->setProperty("gold", true);

    QFont tf = title->font();
    tf.setPixelSize(34);
    tf.setWeight(QFont::Bold);
    title->setFont(tf);

    auto* sub = new QLabel(
        QStringLiteral("Everything a PDF tool should be. Nothing it shouldn't.\n"
                       "Offline. No accounts. No uploads."),
        this);
    sub->setProperty("muted", true);
    sub->setWordWrap(true);

    auto* drop = new DropZone(this);
    drop->setTitle(QStringLiteral("Drop PDFs or images anywhere"));
    drop->setSubtitle(QStringLiteral("Multiple PDFs → Merge   ·   Images → Images to PDF   ·   One PDF → Viewer"));
    drop->setAcceptImages(true);
    drop->setAcceptPdf(true);
    connect(drop, &DropZone::filesDropped, this, &HomeView::filesDropped);

    auto* grid = new QGridLayout();
    grid->setHorizontalSpacing(14);
    grid->setVerticalSpacing(14);

    struct Spec {
        Tool tool;
        const char* icon;
        const char* title;
        const char* sub;
        bool ready;
    };
    const Spec specs[] = {
        {Tool::Create, "create", "Create PDF", "Coming soon — being rebuilt", false},

        {Tool::Images, "images", "Images to PDF", "JPG, PNG, TIFF, WebP — one file", true},
        {Tool::Merge, "merge", "Merge", "Combine PDFs, drag to reorder", true},
        {Tool::Split, "split", "Split / Extract", "Ranges, every N, or pick pages", true},
        {Tool::Organize, "organize", "Organize", "Rotate, reorder, delete pages", true},
        {Tool::Compress, "compress", "Compress", "Lossless or downsample images", true},
        {Tool::Protect, "protect", "Protect", "AES-256 lock or remove a password", true},
        {Tool::Viewer, "viewer", "Viewer", "Open and read a PDF", true},
        {Tool::Edit, "edit", "Edit text", "Replace runs, add text/images, redact", true},
        {Tool::Annotate, "annotate", "Annotate", "Highlight, comment, ink, free text", true},
        {Tool::Watermark, "watermark", "Watermark", "Stamp, headers, page numbers", true},
        {Tool::Sign, "sign", "Sign", "Draw, type, stamp, optional PKCS#12", true},
        {Tool::Ocr, "ocr", "OCR", "Make scans searchable, fully offline", true},
    };

    int i = 0;


    for (const auto& s : specs) {
        auto* card = new ToolCard(s.tool, QString::fromUtf8(s.icon), QString::fromUtf8(s.title),
                                  QString::fromUtf8(s.sub), s.ready, this);
        connect(card, &ToolCard::activated, this, &HomeView::toolChosen);
        grid->addWidget(card, i / 4, i % 4);
        ++i;
    }

    auto* recentLabel = new QLabel(QStringLiteral("Recent"), this);
    QFont rf = recentLabel->font();
    rf.setPixelSize(13);
    rf.setWeight(QFont::DemiBold);
    recentLabel->setFont(rf);

    auto* recent = new QListWidget(this);
    recent->setMaximumHeight(120);
    for (const auto& p : AppSettings::instance().recentFiles()) {
        auto* it = new QListWidgetItem(QFileInfo(p).fileName(), recent);
        it->setToolTip(p);
        it->setData(Qt::UserRole, p);
    }
    connect(recent, &QListWidget::itemActivated, this, [this](QListWidgetItem* it) {
        emit filesDropped(QStringList{it->data(Qt::UserRole).toString()});
    });

    inner->addWidget(kicker);
    inner->addWidget(title);
    inner->addWidget(sub);
    inner->addWidget(drop);
    inner->addLayout(grid);
    inner->addSpacing(8);
    inner->addWidget(recentLabel);
    inner->addWidget(recent);
    inner->addStretch();

    scroller->setWidget(content);
    root->addWidget(scroller);
}

} // namespace drpdf

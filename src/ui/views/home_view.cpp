#include "home_view.h"

#include "app/settings.h"
#include "ui/components/drop_zone.h"
#include "ui/components/tool_card.h"
#include "ui/theme/theme.h"

#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

namespace drpdf {

HomeView::HomeView(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(32, 24, 32, 24);
    root->setSpacing(18);

    auto* kicker = new QLabel(QStringLiteral("LOCAL-FIRST PDF WORKSPACE"), this);
    kicker->setProperty("muted", true);
    QFont kf = kicker->font();
    kf.setPixelSize(11);
    kf.setWeight(QFont::DemiBold);
    kf.setLetterSpacing(QFont::AbsoluteSpacing, 1.4);
    kicker->setFont(kf);

    auto* title = new QLabel(QStringLiteral("Dr PDF"), this);
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
        {Tool::Create, "create", "Create PDF", "Write like a document, export as PDF", true},
        {Tool::Images, "images", "Images to PDF", "JPG, PNG, TIFF, WebP — one file", true},
        {Tool::Merge, "merge", "Merge", "Combine PDFs, drag to reorder", true},
        {Tool::Split, "split", "Split / Extract", "Ranges, every N, or pick pages", true},
        {Tool::Organize, "organize", "Organize", "Rotate, reorder, delete pages", true},
        {Tool::Compress, "compress", "Compress", "Optimize streams, shrink files", true},
        {Tool::Protect, "protect", "Protect", "AES-256 lock or remove a password", true},
        {Tool::Viewer, "viewer", "Viewer", "Open and read a PDF", true},
        {Tool::Edit, "edit", "Edit text", "Block-level text editing", false},
        {Tool::Sign, "sign", "Sign", "Draw, type, or stamp a signature", false},
        {Tool::Ocr, "ocr", "OCR", "Make scans searchable, fully offline", false},
        {Tool::Annotate, "annotate", "Annotate", "Highlight, comment, draw", false},
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

    root->addWidget(kicker);
    root->addWidget(title);
    root->addWidget(sub);
    root->addWidget(drop);
    root->addLayout(grid);
    root->addSpacing(8);
    root->addWidget(recentLabel);
    root->addWidget(recent);
    root->addStretch();
}

} // namespace drpdf

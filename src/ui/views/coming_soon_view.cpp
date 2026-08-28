#include "coming_soon_view.h"

#include "ui/theme/theme.h"

#include <QLabel>
#include <QVBoxLayout>

namespace drpdf {

ComingSoonView::ComingSoonView(Tool tool, const QString& milestone, const QString& blurb,
                               QWidget* parent)
    : QWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(48, 48, 48, 48);
    lay->setSpacing(12);
    auto* title = new QLabel(toolTitle(tool), this);
    QFont f = title->font();
    f.setPixelSize(28);
    f.setWeight(QFont::Bold);
    title->setFont(f);
    auto* m = new QLabel(milestone, this);
    m->setProperty("muted", true);
    auto* b = new QLabel(blurb, this);
    b->setWordWrap(true);
    b->setProperty("muted", true);
    auto* note = new QLabel(
        QStringLiteral("The engine API is in /core so this tool can be wired without UI rewrites. "
                       "Nothing here phones home."),
        this);
    note->setWordWrap(true);
    lay->addWidget(title);
    lay->addWidget(m);
    lay->addSpacing(8);
    lay->addWidget(b);
    lay->addSpacing(16);
    lay->addWidget(note);
    lay->addStretch();
}

} // namespace drpdf

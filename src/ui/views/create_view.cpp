#include "create_view.h"

/*
=============================================================================================
 CREATE PDF  -  TEMPORARILY DISABLED  (Coming Soon)
=============================================================================================
 WHY THIS FEATURE WAS DISABLED
 -------------------------------
 The Word-like "Create PDF" editor (QTextEdit nested inside QScrollArea > canvas > paper and
 given rich-text mode + setFontPointSize(12) + a stylesheet) crashed the app on the very first
 keystroke with an access violation (0xC0000005). This is REAL and reproducible.

 DIAGNOSIS (probe/main.cpp, target drpdf-probe)
 --------------------------------------------
   Stage A: bare QTextEdit + synthetic keystrokes  -> NO crash
   Stage B: CreateView           + synthetic keystrokes  -> crash on first key
   Stage C: CreateView           + currentCharFormat()  -> crash on FIRST DIRECT CALL

 Conclusion: the crash is NOT in the keystroke handler chain and NOT the syncToolbar()
 font/size combo recursion (that was already guarded with QSignalBlocker + syncing_ flag).
 The QTextEdit is constructed in a BROKEN state so that even querying currentCharFormat()
 dereferences an invalid internal object. Prime suspects, in order:
   1) The deeply nested container chain combined with rich-text accept + setFontPointSize(12)
      and a stylesheet that does `QTextEdit { background: transparent; }` applied AFTER widget
      setup (applyPageMetrics -> editor_->setStyleSheet).
   2) Setting paper_->setFixedWidth() plus the scroll-area widget-resizable interplay driving
      the editor through a zero/negative-height relayout before the first paint.

 HOW TO RE-ENABLE (do in this order, validating in the probe at each step)
 -----------------------------------------------------------------------
   1. Attach the QTextEdit to a PLAIN QWidget parent first (no scroll nesting).
   2. Do NOT call editor->setStyleSheet() in applyPageMetrics(); style a wrapping QFrame instead.
   3. After creation, call editor->currentCharFormat() once in the probe; if it crashes, fix
      construction before wiring any signals.
   4. Once stable, add the scroller/paper chrome back and re-connect toolbar handlers.

 Until then, this view renders a simple "Coming Soon" panel so the rest of the app is usable.
=============================================================================================
*/

#include "create_view.h"

#include "ui/icons/icons.h"
#include "ui/theme/theme.h"

#include <QLabel>
#include <QVBoxLayout>

namespace drpdf {

// Safe "Coming Soon" placeholder. Real editor construction is disabled -- see header comment.
CreateView::CreateView(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("CreateView"));

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(48, 48, 48, 48);
    lay->setSpacing(18);

    auto* icon = new QLabel(this);
    icon->setPixmap(Icons::pixmap(QStringLiteral("create"), Theme::instance().tokens().accent, 72));
    icon->setAlignment(Qt::AlignCenter);

    auto* heading = new QLabel(QStringLiteral("Create PDF"), this);
    QFont hf = heading->font();
    hf.setPixelSize(26);
    hf.setWeight(QFont::DemiBold);
    heading->setFont(hf);
    heading->setAlignment(Qt::AlignCenter);

    auto* sub = new QLabel(
        QStringLiteral("Coming soon. The Word-like editor is being rebuilt "
                       "(it previously crashed on first keystroke)."),
        this);
    sub->setWordWrap(true);
    sub->setAlignment(Qt::AlignCenter);
    sub->setProperty("muted", true);

    lay->addWidget(icon);
    lay->addWidget(heading);
    lay->addWidget(sub);
    lay->addStretch();
}

/*
=============================================================================================
 ORIGINAL (DISABLED) IMPLEMENTATION -- REFERENCE ONLY
 The code below is the crash-causing CreateView. It is NOT compiled. Re-enable only after
 applying the fix strategy described in the header comment.
=============================================================================================
void CreateView::exportPdf() {}
void CreateView::saveDraft() {}
void CreateView::openDraft() {}
void CreateView::autosave() {}
void CreateView::maybeRecover() {}
void CreateView::applyHeading(int) {}
void CreateView::insertTable() {}
void CreateView::insertPageBreak() {}
void CreateView::insertImage() {}
void CreateView::toggleList(bool) {}
void CreateView::setAlign(Qt::Alignment) {}
void CreateView::setColor(bool) {}
void CreateView::syncToolbar() {}
void CreateView::applyPageMetrics() {}
void CreateView::updateStats() {}
QString CreateView::autosavePath() const { return QString(); }
*/

} // namespace drpdf

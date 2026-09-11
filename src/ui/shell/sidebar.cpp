#include "sidebar.h"

#include "ui/icons/icons.h"
#include "ui/theme/theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>


namespace drpdf {

Sidebar::Sidebar(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("Sidebar"));
    setMinimumWidth(72);
    setMaximumWidth(240);
    setAccessibleName(QStringLiteral("Tools"));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(10, 14, 10, 14);
    root->setSpacing(4);

    auto* brandRow = new QHBoxLayout();
    auto* brand = new QLabel(QStringLiteral("Dr PDF"), this);
    brand->setProperty("gold", true);
    QFont bf = brand->font();
    bf.setPixelSize(15);
    bf.setWeight(QFont::DemiBold);
    brand->setFont(bf);
    auto* collapse = new QToolButton(this);
    collapse->setText(QStringLiteral("«"));
    collapse->setToolTip(QStringLiteral("Collapse sidebar"));
    collapse->setAccessibleName(QStringLiteral("Collapse sidebar"));
    connect(collapse, &QToolButton::clicked, this, [this, collapse] {
        setCollapsed(!collapsed_);
        collapse->setText(collapsed_ ? QStringLiteral("»") : QStringLiteral("«"));
        emit collapseToggled(collapsed_);
    });
    brandRow->addWidget(brand, 1);
    brandRow->addWidget(collapse);
    root->addLayout(brandRow);
    headers_.push_back(brand);

    auto addHeader = [&](const QString& text) {
        auto* h = new QLabel(text, this);
        h->setProperty("muted", true);
        QFont f = h->font();
        f.setPixelSize(10);
        f.setWeight(QFont::DemiBold);
        f.setLetterSpacing(QFont::AbsoluteSpacing, 1.1);
        h->setFont(f);
        root->addSpacing(10);
        root->addWidget(h);
        headers_.push_back(h);
    };

    nav_ = root;
    addNav(QStringLiteral("app"), QStringLiteral("Home"), Tool::Home);
    addHeader(QStringLiteral("CREATE"));

    addNav(QStringLiteral("create"), QStringLiteral("Create"), Tool::Create);
    addNav(QStringLiteral("images"), QStringLiteral("Images"), Tool::Images);

    addHeader(QStringLiteral("ORGANIZE"));
    addNav(QStringLiteral("merge"), QStringLiteral("Merge"), Tool::Merge);
    addNav(QStringLiteral("split"), QStringLiteral("Split"), Tool::Split);
    addNav(QStringLiteral("organize"), QStringLiteral("Pages"), Tool::Organize);

    addHeader(QStringLiteral("EDIT"));
    addNav(QStringLiteral("edit"), QStringLiteral("Edit"), Tool::Edit);
    addNav(QStringLiteral("annotate"), QStringLiteral("Annotate"), Tool::Annotate);
    addNav(QStringLiteral("watermark"), QStringLiteral("Watermark"), Tool::Watermark);

    addHeader(QStringLiteral("SECURE"));
    addNav(QStringLiteral("protect"), QStringLiteral("Protect"), Tool::Protect);
    addNav(QStringLiteral("sign"), QStringLiteral("Sign"), Tool::Sign);

    addHeader(QStringLiteral("TOOLS"));
    addNav(QStringLiteral("compress"), QStringLiteral("Compress"), Tool::Compress);
    addNav(QStringLiteral("ocr"), QStringLiteral("OCR"), Tool::Ocr);
    addNav(QStringLiteral("viewer"), QStringLiteral("Viewer"), Tool::Viewer);

    root->addStretch();
}

QToolButton* Sidebar::addNav(const QString& icon, const QString& text, Tool tool) {
    auto* b = new QToolButton(this);
    b->setText(text);
    b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    b->setIcon(Icons::named(icon, Theme::instance().tokens().accent, 18));
    b->setCheckable(true);
    b->setAutoExclusive(true);
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    b->setAccessibleName(text);
    b->setToolTip(text);
    b->setProperty("tool", static_cast<int>(tool));
    connect(b, &QToolButton::clicked, this, [this, tool] { emit toolChosen(tool); });
    nav_->addWidget(b);
    buttons_.push_back(b);
    return b;
}

void Sidebar::setCurrent(Tool tool) {
    for (auto* b : buttons_) {
        b->setChecked(b->property("tool").toInt() == static_cast<int>(tool));
    }
}

void Sidebar::setCollapsed(bool collapsed) {
    collapsed_ = collapsed;
    setFixedWidth(collapsed_ ? 72 : 220);
    relabel();
}

void Sidebar::relabel() {
    for (auto* h : headers_) {
        h->setVisible(!collapsed_);
    }
    for (auto* b : buttons_) {
        b->setToolButtonStyle(collapsed_ ? Qt::ToolButtonIconOnly : Qt::ToolButtonTextBesideIcon);
    }
}

} // namespace drpdf

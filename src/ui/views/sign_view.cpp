#include "sign_view.h"

#include "app/settings.h"
#include "core/overlay.h"
#include "services/pdf_sign.h"
#include "ui/components/banner.h"
#include "ui/components/drop_zone.h"
#include "ui/components/page_canvas.h"
#include "ui/components/signature_pad.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPdfDocument>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>

#include <algorithm>

namespace drpdf {

SignView::SignView(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    auto* title = new QLabel(QStringLiteral("Sign"), this);
    QFont f = title->font();
    f.setPixelSize(22);
    f.setWeight(QFont::DemiBold);
    title->setFont(f);
    auto* hint = new QLabel(
        QStringLiteral("Draw, type, or stamp a signature onto the page. Optional PKCS#12 "
                       "certificate creates a real PDF digital signature on this machine."),
        this);
    hint->setWordWrap(true);
    hint->setProperty("muted", true);

    drop_ = new DropZone(this);
    drop_->setTitle(QStringLiteral("Drop a PDF to sign"));
    drop_->setMaximumHeight(100);
    connect(drop_, &DropZone::filesDropped, this, [this](const QStringList& p) {
        if (!p.isEmpty()) {
            loadFile(p.first());
        }
    });

    page_ = new QSpinBox(this);
    page_->setMinimum(1);
    page_->setPrefix(QStringLiteral("Page "));
    auto* place = new QPushButton(QStringLiteral("Place on page"), this);
    place->setProperty("primary", true);
    auto* save = new QPushButton(QStringLiteral("Save as…"), this);

    canvas_ = new PageCanvas(this);
    pad_ = new SignaturePad(this);
    typed_ = new QLineEdit(this);
    typed_->setPlaceholderText(QStringLiteral("Or type your name"));
    auto* clear = new QPushButton(QStringLiteral("Clear drawing"), this);
    auto* fromFile = new QPushButton(QStringLiteral("Load PNG"), this);

    p12_ = new QLineEdit(this);
    p12_->setPlaceholderText(QStringLiteral("Optional: path to .p12 / .pfx"));
    p12_->setReadOnly(true);
    p12pass_ = new QLineEdit(this);
    p12pass_->setEchoMode(QLineEdit::Password);
    p12pass_->setPlaceholderText(QStringLiteral("PKCS#12 password"));
    crypto_ = new QCheckBox(QStringLiteral("Digitally sign with certificate after placing"), this);
    crypto_->setEnabled(opensslSigningAvailable());
    auto* browseP12 = new QPushButton(QStringLiteral("Certificate…"), this);
    if (!opensslSigningAvailable()) {
        browseP12->setEnabled(false);
        p12pass_->setEnabled(false);
    }

    auto* tools = new QHBoxLayout();
    tools->addWidget(page_);
    tools->addWidget(place);
    tools->addStretch();
    tools->addWidget(save);

    auto* certRow = new QHBoxLayout();
    certRow->addWidget(p12_, 1);
    certRow->addWidget(browseP12);
    certRow->addWidget(p12pass_);

    banner_ = new Banner(this);
    if (!opensslSigningAvailable()) {
        banner_->showInfo(
            QStringLiteral("Visual signatures ready. Rebuild with OpenSSL for PKCS#12 crypto sign."));
    }

    connect(page_, &QSpinBox::valueChanged, this, [this](int) {
        if (working_.isOpen()) {
            QPdfDocument doc;
            if (doc.load(working_.currentPath()) != QPdfDocument::Error::None) {
                return;
            }
            const int page = std::clamp(page_->value() - 1, 0, std::max(0, doc.pageCount() - 1));
            const QSizeF pts = doc.pagePointSize(page);
            const QSize px(std::max(400, int(pts.width() * 1.4)),
                           std::max(500, int(pts.height() * 1.4)));
            canvas_->setPage(doc.render(page, px), pts);
        }
    });
    connect(place, &QPushButton::clicked, this, [this] {
        canvas_->setMode(PageCanvas::Mode::Rect);
        banner_->showInfo(QStringLiteral("Drag a rectangle on the page to drop the signature."));
    });
    connect(canvas_, &PageCanvas::rectDrawn, this, &SignView::placeOnPage);
    connect(clear, &QPushButton::clicked, pad_, &SignaturePad::clear);
    connect(fromFile, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getOpenFileName(
            this, QStringLiteral("Signature image"), AppSettings::instance().lastDirectory(),
            QStringLiteral("Images (*.png *.jpg *.jpeg *.webp)"));
        if (path.isEmpty()) {
            return;
        }
        QImage img(path);
        if (img.isNull()) {
            banner_->showError(QStringLiteral("Could not read image."));
            return;
        }
        typed_->setProperty("image", img);
        banner_->showInfo(QStringLiteral("Image loaded. Place on page."));
    });
    connect(browseP12, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getOpenFileName(
            this, QStringLiteral("PKCS#12 certificate"), AppSettings::instance().lastDirectory(),
            QStringLiteral("PKCS#12 (*.p12 *.pfx)"));
        if (!path.isEmpty()) {
            p12_->setText(path);
        }
    });
    connect(save, &QPushButton::clicked, this, &SignView::saveAs);

    auto* padRow = new QHBoxLayout();
    padRow->addWidget(clear);
    padRow->addWidget(fromFile);
    padRow->addWidget(typed_, 1);

    root->addWidget(title);
    root->addWidget(hint);
    root->addWidget(drop_);
    root->addLayout(tools);
    root->addWidget(canvas_, 1);
    root->addWidget(pad_);
    root->addLayout(padRow);
    root->addWidget(crypto_);
    root->addLayout(certRow);
    root->addWidget(banner_);
}

QImage SignView::currentSignature() const {
    if (typed_->property("image").isValid()) {
        const QImage img = typed_->property("image").value<QImage>();
        if (!img.isNull()) {
            return img;
        }
    }
    if (!typed_->text().trimmed().isEmpty()) {
        QImage img(900, 220, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        QFont font(QStringLiteral("Segoe Script"));
        if (!font.exactMatch()) {
            font = QFont(QStringLiteral("Apple Chancery"));
        }
        if (!font.exactMatch()) {
            font = QFont(QStringLiteral("Comic Sans MS"));
        }
        font.setPointSize(48);
        font.setItalic(true);
        p.setFont(font);
        p.setPen(Qt::black);
        p.drawText(img.rect().adjusted(20, 10, -20, -10), Qt::AlignVCenter | Qt::AlignLeft,
                   typed_->text().trimmed());
        return img;
    }
    if (!pad_->isEmpty()) {
        return pad_->toImage();
    }
    return {};
}

void SignView::loadFile(const QString& path) {
    if (!working_.load(path)) {
        banner_->showError(QStringLiteral("Could not open file."));
        return;
    }
    AppSettings::instance().addRecentFile(path);
    QPdfDocument doc;
    doc.load(working_.currentPath());
    page_->setMaximum(std::max(1, doc.pageCount()));
    page_->setValue(1);
    page_->valueChanged(1);
}

void SignView::placeOnPage(QRectF pdf) {
    QImage img = currentSignature();
    if (img.isNull()) {
        banner_->showError(QStringLiteral("Draw, type, or load a signature first."));
        return;
    }
    lastRect_ = pdf;
    core::ImageStamp stamp;
    stamp.page = page_->value() - 1;
    stamp.x = pdf.x();
    stamp.y = pdf.y();
    stamp.width = std::max(8.0, pdf.width());
    stamp.height = std::max(8.0, pdf.height());
    stamp.image = rasterFromImage(img);
    auto r = working_.apply([&](const auto& in, const auto& out) { return core::stampImage(in, out, stamp); });
    if (!r) {
        banner_->showError(QString::fromStdString(r.error()));
        return;
    }
    if (crypto_->isChecked() && !p12_->text().isEmpty()) {
        auto cr = working_.apply([&](const auto& in, const auto& out) {
            return digitallySignPdf(QString::fromStdString(in.string()),
                                    QString::fromStdString(out.string()), p12_->text(),
                                    p12pass_->text(), stamp.page, stamp.x, stamp.y,
                                    stamp.x + stamp.width, stamp.y + stamp.height);
        });
        if (!cr) {
            banner_->showError(QString::fromStdString(cr.error()));
            page_->valueChanged(page_->value());
            return;
        }
        banner_->showInfo(QStringLiteral("Placed and digitally signed. Save as a new file."));
    } else {
        banner_->showInfo(QStringLiteral("Signature placed. Save as a new file."));
    }
    canvas_->setMode(PageCanvas::Mode::None);
    page_->valueChanged(page_->value());
}

void SignView::saveAs() {
    if (!working_.isOpen()) {
        banner_->showError(QStringLiteral("Open a PDF first."));
        return;
    }
    const QString out = QFileDialog::getSaveFileName(
        this, QStringLiteral("Save signed PDF"),
        AppSettings::instance().lastDirectory() + QStringLiteral("/signed.pdf"),
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

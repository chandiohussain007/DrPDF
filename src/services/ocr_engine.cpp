#include "ocr_engine.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPdfDocument>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <cmath>
#include <algorithm>


namespace drpdf {
namespace {

QStringList extraTesseractGuesses() {
    QStringList g;
#ifdef Q_OS_WIN
    g << QStringLiteral("C:/Program Files/Tesseract-OCR/tesseract.exe")
      << QStringLiteral("C:/Program Files (x86)/Tesseract-OCR/tesseract.exe");
#endif
#ifdef Q_OS_MAC
    g << QStringLiteral("/opt/homebrew/bin/tesseract") << QStringLiteral("/usr/local/bin/tesseract");
#endif
    g << QStringLiteral("/usr/bin/tesseract");
    return g;
}

} // namespace

QString findTesseract() {
    const QString env = qEnvironmentVariable("TESSERACT_PATH");
    if (!env.isEmpty() && QFileInfo::exists(env)) {
        return env;
    }
    const QString path = QStandardPaths::findExecutable(QStringLiteral("tesseract"));
    if (!path.isEmpty()) {
        return path;
    }
    for (const auto& g : extraTesseractGuesses()) {
        if (QFileInfo::exists(g)) {
            return g;
        }
    }
    return {};
}

QStringList tesseractLanguages() {
    QStringList langs;
    const QString exe = findTesseract();
    if (exe.isEmpty()) {
        return {QStringLiteral("eng")};
    }
    QProcess p;
    p.start(exe, {QStringLiteral("--list-langs")});
    if (!p.waitForFinished(8000)) {
        return {QStringLiteral("eng")};
    }
    const QString out = QString::fromUtf8(p.readAllStandardOutput() + p.readAllStandardError());
    for (const auto& line : out.split('\n')) {
        const QString s = line.trimmed();
        if (s.isEmpty() || s.contains(' ') || s.contains(':')) {
            continue;
        }
        langs << s;
    }
    if (langs.isEmpty()) {
        langs << QStringLiteral("eng");
    }
    return langs;
}

core::Result<void> ocrPdf(const QString& input, const QString& output, const OcrOptions& options,
                          const std::function<void(int page, int total)>& progress) {
    const QString exe = findTesseract();
    if (exe.isEmpty()) {
        return core::Error{
            "Tesseract was not found. Install tesseract-ocr (and a language pack, e.g. eng) "
            "or set TESSERACT_PATH to the binary. Nothing is uploaded."};
    }
    QPdfDocument doc;
    if (doc.load(input) != QPdfDocument::Error::None) {
        return core::Error{"could not render PDF for OCR"};
    }
    const int pages = doc.pageCount();
    if (pages <= 0) {
        return core::Error{"PDF has no pages"};
    }
    QTemporaryDir tmp;
    if (!tmp.isValid()) {
        return core::Error{"could not create temp directory"};
    }
    std::vector<core::SearchableWord> words;
    QString sidecar;
    for (int i = 0; i < pages; ++i) {
        if (progress) {
            progress(i + 1, pages);
        }
        const QSizeF pts = doc.pagePointSize(i);
        const double scale = std::max(1.0, options.dpi / 72.0);
        QSize px(std::max(1, int(pts.width() * scale)), std::max(1, int(pts.height() * scale)));
        if (px.width() > 2800 || px.height() > 2800) {
            px.scale(2800, 2800, Qt::KeepAspectRatio);
        }
        const QImage img = doc.render(i, px);
        if (img.isNull()) {
            continue;
        }
        const QString png = tmp.filePath(QStringLiteral("p%1.png").arg(i));
        if (!img.save(png, "PNG")) {
            return core::Error{"could not write OCR raster"};
        }
        QProcess p;
        p.start(exe, {png, QStringLiteral("stdout"), QStringLiteral("tsv"), QStringLiteral("-l"),
                      options.language, QStringLiteral("--dpi"),
                      QString::number(std::max(70, int(px.width() / std::max(1.0, pts.width() / 72.0))))});
        if (!p.waitForFinished(120000)) {
            p.kill();
            return core::Error{"tesseract timed out on page " + std::to_string(i + 1)};
        }
        if (p.exitCode() != 0) {
            const QString err = QString::fromUtf8(p.readAllStandardError());
            return core::Error{"tesseract failed: " + err.toStdString()};
        }
        const std::string tsv = p.readAllStandardOutput().toStdString();
        auto parsed = core::parseTesseractTsv(tsv, i);
        for (const auto& w : parsed) {
            core::SearchableWord sw;
            sw.page = i;
            sw.width = (w.width / double(px.width())) * pts.width();
            sw.height = (w.height / double(px.height())) * pts.height();
            sw.x = (w.left / double(px.width())) * pts.width();
            sw.y = pts.height() - ((w.top + w.height) / double(px.height())) * pts.height();
            sw.text = w.text;
            sidecar += QString::fromStdString(w.text);
            sidecar += QLatin1Char(' ');
            words.push_back(std::move(sw));
        }
        sidecar += QLatin1Char('\n');
    }
    auto r = core::overlaySearchableText(std::filesystem::path(input.toStdString()),
                                         std::filesystem::path(output.toStdString()), words);
    if (!r) {
        return r;
    }
    if (options.sidecarTxt) {
        QFile f(output + QStringLiteral(".txt"));
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            f.write(sidecar.toUtf8());
        }
    }
    return {};
}

} // namespace drpdf

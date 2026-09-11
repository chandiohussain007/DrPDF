// Crash prwobe: simulates typing into CreateView to reproduce the user-reported crash.
#include <QApplication>
#include <QEventLoop>
#include <QKeyEvent>
#include <QTextEdit>
#include <QTimer>

#include "ui/theme/theme.h"
#include "ui/views/create_view.h"

#include <cstdio>
#include <functional>

#define LOG(msg)                                                                                       \
    do {                                                                                               \
        std::fprintf(stderr, "[STAGE] %s\n", msg);                                                     \
        std::fflush(stderr);                                                                           \
    } while (0)

namespace {

void pump(QApplication& app, int ms) {
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, [&loop] { loop.quit(); });
    loop.exec();
}

void typeOne(QApplication& app, QTextEdit* editor, QChar ch) {
    const int key = ch.toUpper().unicode();
    QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier, QString(ch));
    QApplication::sendEvent(editor, &press);
    QKeyEvent release(QEvent::KeyRelease, key, Qt::NoModifier, QString(ch));
    QApplication::sendEvent(editor, &release);
    pump(app, 5);
}

} // namespace

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Dr PDF"));
    QApplication::setOrganizationName(QStringLiteral("DrPdf"));

    const bool noQss = argc > 1 && QString::fromLocal8Bit(argv[1]) == QStringLiteral("noqss");
    if (!noQss) {
        LOG("applying theme stylesheet...");
        auto& theme = drpdf::Theme::instance();
        theme.setDark(true);
        theme.apply(app);
        LOG("theme applied OK");
    }

    LOG("creating bare QTextEdit...");
    {
        QTextEdit probe;
        probe.setPlaceholderText(QStringLiteral("probe"));
        probe.resize(600, 400);
        LOG("showing bare QTextEdit...");
        probe.show();
        LOG("pumping 150ms...");
        pump(app, 150);
        LOG("typing 'A' into bare QTextEdit...");
        typeOne(app, &probe, QLatin1Char('A'));
        LOG("typed 'A' OK");
        typeOne(app, &probe, QLatin1Char('b'));
        LOG("typed 'b' OK");
        LOG("bare text =");
        std::fprintf(stderr, "  '%s'\n", probe.toPlainText().toStdString().c_str());
        std::fflush(stderr);
    }
    LOG("bare QTextEdit scope ended OK");

    LOG("creating CreateView...");
    drpdf::CreateView view;
    view.resize(1100, 720);
    LOG("showing CreateView...");
    view.show();
    pump(app, 200);
    LOG("CreateView shown OK");

    QTextEdit* editor = view.findChild<QTextEdit*>();
    if (!editor) {
        LOG("NO EDITOR FOUND INSIDE CreateView");
        return 2;
    }
    LOG("calling currentCharFormat() directly...");
    {
        auto f = editor->currentCharFormat();
        std::fprintf(stderr, "[STAGE] currentCharFormat OK family='%s' size=%f\n",
                     f.fontFamily().toStdString().c_str(), f.fontPointSize());
        std::fflush(stderr);
    }
    LOG("calling textCursor()...");
    {
        auto c = editor->textCursor();
        std::fprintf(stderr, "[STAGE] textCursor OK pos=%d\n", c.position());
        std::fflush(stderr);
    }
    editor->setFocus();
    pump(app, 50);
    LOG("typing into CreateView editor...");

    const QString text = QStringLiteral("Hello Dr PDF");
    for (const QChar ch : text) {
        typeOne(app, editor, ch);
    }
    LOG("typed plain text OK");

    LOG("pressing Enter...");
    {
        QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, QStringLiteral("\r"));
        QApplication::sendEvent(editor, &enter);
        pump(app, 20);
    }
    LOG("enter OK");

    LOG("pressing Backspace...");
    {
        QKeyEvent back(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier, QString());
        QApplication::sendEvent(editor, &back);
        pump(app, 20);
    }
    LOG("backspace OK");

    LOG("pressing Ctrl+B...");
    {
        QKeyEvent ctrlB(QEvent::KeyPress, Qt::Key_B, Qt::ControlModifier, QString());
        QApplication::sendEvent(editor, &ctrlB);
        QKeyEvent relB(QEvent::KeyRelease, Qt::Key_B, Qt::ControlModifier, QString());
        QApplication::sendEvent(editor, &relB);
        pump(app, 20);
        typeOne(app, editor, QLatin1Char('x'));
    }
    LOG("ctrl+b + typing OK");

    LOG("moving cursor...");
    {
        QTextCursor c = editor->textCursor();
        c.movePosition(QTextCursor::Start);
        editor->setTextCursor(c);
        pump(app, 20);
        c.movePosition(QTextCursor::End);
        editor->setTextCursor(c);
        pump(app, 20);
    }
    LOG("cursor moves OK");

    LOG("PROBE PASSED — no crash");
    return 0;
}
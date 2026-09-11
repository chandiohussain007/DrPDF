#include "app/settings.h"
#include "ui/icons/icons.h"
#include "ui/shell/main_window.h"
#include "ui/theme/theme.h"

#include <QApplication>
#include <QFileInfo>
#include <QIcon>

int main(int argc, char* argv[]) {
    QApplication::setApplicationName(QStringLiteral("Dr PDF"));
    QApplication::setOrganizationName(QStringLiteral("DrPdf"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    QApplication::setDesktopFileName(QStringLiteral("app.drpdf.desktop"));

    QApplication app(argc, argv);
    QIcon appIcon(QStringLiteral(":/logo.png"));
    if (appIcon.isNull()) {
        appIcon = drpdf::Icons::app(64);
    }
    app.setWindowIcon(appIcon);

    auto& theme = drpdf::Theme::instance();
    theme.setDark(drpdf::AppSettings::instance().darkTheme());
    theme.apply(app);

    drpdf::MainWindow window;
    QStringList paths;
    const auto args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        if (QFileInfo::exists(args[i])) {
            paths << args[i];
        }
    }
    if (!paths.isEmpty()) {
        window.openPaths(paths);
    }
    window.show();
    return app.exec();
}

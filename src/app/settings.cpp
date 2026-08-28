#include "settings.h"

#include <QSettings>
#include <QStandardPaths>

namespace drpdf {

AppSettings& AppSettings::instance() {
    static AppSettings s;
    return s;
}

AppSettings::AppSettings() {
    s_ = new QSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("DrPdf"),
                       QStringLiteral("DrPdf"));
}

bool AppSettings::darkTheme() const { return s_->value(QStringLiteral("ui/dark"), true).toBool(); }

void AppSettings::setDarkTheme(bool dark) { s_->setValue(QStringLiteral("ui/dark"), dark); }

QStringList AppSettings::recentFiles() const {
    return s_->value(QStringLiteral("files/recent")).toStringList();
}

void AppSettings::addRecentFile(const QString& path) {
    QStringList list = recentFiles();
    list.removeAll(path);
    list.prepend(path);
    while (list.size() > 12) {
        list.removeLast();
    }
    s_->setValue(QStringLiteral("files/recent"), list);
}

void AppSettings::clearRecent() { s_->remove(QStringLiteral("files/recent")); }

QString AppSettings::lastDirectory() const {
    return s_->value(QStringLiteral("files/lastDir"),
                     QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
        .toString();
}

void AppSettings::setLastDirectory(const QString& dir) {
    s_->setValue(QStringLiteral("files/lastDir"), dir);
}

} // namespace drpdf

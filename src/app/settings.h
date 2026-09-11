#pragma once

#include <QStringList>

class QSettings;

namespace drpdf {

class AppSettings {
public:
    static AppSettings& instance();

    bool darkTheme() const;
    void setDarkTheme(bool dark);

    QStringList recentFiles() const;
    void addRecentFile(const QString& path);
    void clearRecent();

    QString lastDirectory() const;
    void setLastDirectory(const QString& dir);

    bool sidebarCollapsed() const;
    void setSidebarCollapsed(bool collapsed);


private:
    AppSettings();
    QSettings* s_ = nullptr;
};

} // namespace drpdf

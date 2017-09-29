#ifndef APPLICATIONPATHS_H
#define APPLICATIONPATHS_H

#include <QDir>
#include <QStandardPaths>
#include <QStringList>

namespace QtGui {

class ApplicationPaths {
public:
    static QString settingsPath();
    static QString dowloadedFilesPath();

protected:
    static QString path(QStandardPaths::StandardLocation location);
};

inline QString ApplicationPaths::settingsPath()
{
    return path(QStandardPaths::DataLocation);
}

inline QString ApplicationPaths::dowloadedFilesPath()
{
    return path(QStandardPaths::CacheLocation);
}

inline QString ApplicationPaths::path(QStandardPaths::StandardLocation location)
{
    QString path = QStandardPaths::standardLocations(location).value(0);
    QDir dir(path);
    if (!dir.exists())
        dir.mkpath(path);
    if (!path.isEmpty() && !path.endsWith("/"))
        path += "/";
    return path;
}
} // namespace QtGui

#endif // APPLICATIONPATHS_H

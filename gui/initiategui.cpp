#include "./initiategui.h"

#include "./gui/mainwindow.h"

#include "resources/config.h"
#include "resources/qtconfig.h"

#include <qtutilities/resources/importplugin.h>
#include <qtutilities/resources/qtconfigarguments.h>
#include <qtutilities/resources/resources.h>
#include <qtutilities/settingsdialog/qtsettings.h>

#include <passwordfile/util/openssl.h>

#include <QApplication>
#include <QFile>
#include <QSettings>

using namespace CppUtilities;
using namespace QtUtilities;
using namespace Util;

namespace QtGui {

int runWidgetsGui(int argc, char *argv[], const QtConfigArguments &qtConfigArgs, const QString &file)
{
    SET_QT_APPLICATION_INFO;

    // init OpenSSL
    OpenSsl::init();

    // init application
    QApplication application(argc, argv);

    // restore Qt settings
    QtSettings qtSettings;
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral(PROJECT_NAME));

    // move old config to new location
    const QString oldConfig
        = QSettings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName()).fileName();
    QFile::rename(oldConfig, settings.fileName()) || QFile::remove(oldConfig);
    settings.sync();
    qtSettings.restore(settings);
    qtSettings.apply();

    // apply settings specified via command line args
    qtConfigArgs.applySettings(qtSettings.hasCustomFont());
    LOAD_QT_TRANSLATIONS;

    // init widgets GUI
    MainWindow w(settings, &qtSettings);
    w.show();
    if (!file.isEmpty()) {
        w.openFile(file);
    }

    // start event loop
    QObject::connect(&application, &QCoreApplication::aboutToQuit, &OpenSsl::clean);
    return application.exec();
}
} // namespace QtGui

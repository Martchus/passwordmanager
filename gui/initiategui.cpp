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
#include <QMessageBox>
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
    auto qtSettings = QtSettings();
    auto settings = QtUtilities::getSettings(QStringLiteral(PROJECT_NAME));
    auto settingsError = QtUtilities::errorMessageForSettings(*settings);
    qtSettings.disableNotices();
    qtSettings.restore(*settings);
    qtSettings.apply();

    // apply settings specified via command line args
    qtConfigArgs.applySettings(qtSettings.hasCustomFont());
    LOAD_QT_TRANSLATIONS;

    // init widgets GUI
    if (!settingsError.isEmpty()) {
        QMessageBox::critical(nullptr, QCoreApplication::applicationName(), settingsError);
    }
    auto w = MainWindow(*settings, &qtSettings);
    w.show();
    if (!file.isEmpty()) {
        w.openFile(file);
    }

    // start event loop
    QObject::connect(&application, &QCoreApplication::aboutToQuit, &OpenSsl::clean);
    auto res = application.exec();

    // save settings to disk
    settings->sync();
    if (settingsError.isEmpty()) {
        settingsError = QtUtilities::errorMessageForSettings(*settings);
        if (!settingsError.isEmpty()) {
            QMessageBox::critical(nullptr, QCoreApplication::applicationName(), settingsError);
        }
    }

    return res;
}
} // namespace QtGui

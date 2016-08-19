#include "./initiategui.h"

#include "./gui/mainwindow.h"

#include "resources/config.h"

#include <qtutilities/resources/qtconfigarguments.h>
#include <qtutilities/resources/resources.h>
#include <qtutilities/resources/importplugin.h>
#include <qtutilities/settingsdialog/qtsettings.h>

#include <QApplication>
#include <QSettings>

using namespace ApplicationUtilities;
using namespace Dialogs;

namespace QtGui {

int runWidgetsGui(int argc, char *argv[], const QtConfigArguments &qtConfigArgs, const QString &file)
{
    SET_QT_APPLICATION_INFO;
    // init application
    QApplication a(argc, argv);
    // restore Qt settings
    QtSettings qtSettings;
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,  QApplication::organizationName(), QApplication::applicationName());
    qtSettings.restore(settings);
    qtSettings.apply();
    // load resources needed by classes of qtutilities
    QtUtilitiesResources::init();
    // apply settings specified via command line args
    qtConfigArgs.applySettings();
    LOAD_QT_TRANSLATIONS;
    // init widgets GUI
    MainWindow w(settings, &qtSettings);
    w.show();
    if(!file.isEmpty()) {
        w.openFile(file);
    }
    // start event loop
    int res = a.exec();
    // cleanup resources
    QtUtilitiesResources::cleanup();
    // save Qt settings
    qtSettings.save(settings);
    return res;
}

}

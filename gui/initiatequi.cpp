#include "./initiategui.h"

#include "./gui/mainwindow.h"

// include configuration from separate header file when building with CMake
#ifndef APP_METADATA_AVAIL
#include "config.h"
#endif

#include <qtutilities/resources/qtconfigarguments.h>
#include <qtutilities/resources/resources.h>

#include <QTextCodec>
#include <QApplication>
#include <QFile>

using namespace ApplicationUtilities;

namespace QtGui {

int runWidgetsGui(int argc, char *argv[], const QtConfigArguments &qtConfigArgs, const QString &file)
{
    SET_QT_APPLICATION_INFO;
    // init application
    QApplication a(argc, argv);
    // load resources needed by classes of qtutilities
    QtUtilitiesResources::init();
    // apply settings specified via command line args
    qtConfigArgs.applySettings();
    LOAD_QT_TRANSLATIONS;
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    // init widgets GUI
    QtGui::MainWindow w;
    w.show();
    if(!file.isEmpty()) {
        w.openFile(file);
    }
    // start event loop
    int res = a.exec();
    // cleanup resources
    QtUtilitiesResources::cleanup();
    return res;
}

}

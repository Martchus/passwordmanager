#include "initiate.h"

# include "gui/mainwindow.h"

#include <qtutilities/resources/resources.h>

#include <QTextCodec>
#include <QApplication>
#include <QFile>

namespace QtGui {

int runWidgetsGui(int argc, char *argv[], const QString &file)
{
    SET_QT_APPLICATION_INFO;
    // init application
    QApplication a(argc, argv);
    LOAD_QT_TRANSLATIONS;
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    // load the other resources
    QtUtilitiesResources::init();
    Theme::setup();
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

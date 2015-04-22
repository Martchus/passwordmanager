#include "initiate.h"

# include "gui/mainwindow.h"

#include <qtutilities/resources/resources.h>

#include <QTextCodec>
#include <QApplication>
#include <QFile>


namespace QtGui {

int runWidgetsGui(int argc, char *argv[], const QString &file)
{
    // init application
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QGuiApplication::setOrganizationName(QStringLiteral("Martchus"));
    QGuiApplication::setOrganizationDomain(QStringLiteral("http://martchus.netai.net/"));
    QGuiApplication::setApplicationName(QStringLiteral("Password Manager"));
    QGuiApplication::setApplicationVersion(QStringLiteral("2.0.5"));
    // load translation files
    TranslationFiles::loadQtTranslationFile();
    TranslationFiles::loadApplicationTranslationFile(QStringLiteral("passwordmanager"));
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

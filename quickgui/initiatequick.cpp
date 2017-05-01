#include "./initiatequick.h"
#include "./applicationinfo.h"

#include "../model/entryfiltermodel.h"
#include "../model/entrymodel.h"
#include "../model/fieldmodel.h"

#include "resources/config.h"

#include <qtutilities/resources/qtconfigarguments.h>
#include <qtutilities/resources/resources.h>

#if defined(GUI_QTWIDGETS)
#include <QApplication>
#else
#include <QGuiApplication>
#endif
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTextCodec>
#include <QtQml>

using namespace ApplicationUtilities;

namespace QtGui {

static QObject *applicationInfo(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new ApplicationInfo();
}

int runQuickGui(int argc, char *argv[], const QtConfigArguments &qtConfigArgs)
{
    // init application
    SET_QT_APPLICATION_INFO;
#if defined(GUI_QTWIDGETS)
    QApplication a(argc, argv);
#else
    QGuiApplication a(argc, argv);
#endif
    // apply settings specified via command line args
    qtConfigArgs.applySettings();
    LOAD_QT_TRANSLATIONS;
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    // init quick GUI
    qmlRegisterSingletonType<QtGui::ApplicationInfo>("martchus.passwordmanager", 2, 0, "ApplicationInfo", applicationInfo);
    qmlRegisterType<QtGui::EntryFilterModel>("martchus.passwordmanager", 2, 0, "EntryFilterModel");
    qmlRegisterType<QtGui::EntryModel>("martchus.passwordmanager", 2, 0, "EntryModel");
    qmlRegisterType<QtGui::FieldModel>("martchus.passwordmanager", 2, 0, "FieldModel");
    QQmlApplicationEngine engine(QUrl("qrc:/qml/main.qml"));
    // start event loop
    int res = a.exec();
    return res;
}
}

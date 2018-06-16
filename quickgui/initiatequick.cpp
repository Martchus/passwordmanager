#include "./initiatequick.h"
#include "./controller.h"

#include "resources/config.h"

#define QT_UTILITIES_GUI_QTQUICK
#include <qtutilities/resources/qtconfigarguments.h>
#include <qtutilities/resources/resources.h>

#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTextCodec>
#include <QtQml>

#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS
#include <QApplication>
#endif

using namespace ApplicationUtilities;

namespace QtGui {

int runQuickGui(int argc, char *argv[], const QtConfigArguments &qtConfigArgs, const QString &file)
{
    // init application
    SET_QT_APPLICATION_INFO;
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS
    QApplication a(argc, argv);
#else
    QGuiApplication a(argc, argv);
#endif

    // apply settings specified via command line args
    qtConfigArgs.applySettings();
    qtConfigArgs.applySettingsForQuickGui();

    // load translations and enforce UTF-8 locale
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    LOAD_QT_TRANSLATIONS;

    // determine user paths
    const QVariantMap userPaths{
        { QStringLiteral("desktop"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) },
        { QStringLiteral("documents"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) },
        { QStringLiteral("music"), QStandardPaths::writableLocation(QStandardPaths::MusicLocation) },
        { QStringLiteral("movies"), QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) },
        { QStringLiteral("pictures"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) },
        { QStringLiteral("home"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation) },
    };

    // init Quick GUI
    QQmlApplicationEngine engine;
    Controller controller(file);
    QQmlContext *const context(engine.rootContext());
    context->setContextProperty(QStringLiteral("userPaths"), userPaths);
    context->setContextProperty(QStringLiteral("nativeInterface"), &controller);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    // run event loop
    return a.exec();
}
} // namespace QtGui

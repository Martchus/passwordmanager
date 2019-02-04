#include "./initiatequick.h"
#include "./controller.h"
#ifdef Q_OS_ANDROID
#include "./android.h"
#endif

#include "resources/config.h"

// enable inline helper functions for Qt Quick provided by qtutilities
#define QT_UTILITIES_GUI_QTQUICK

#include <qtutilities/resources/qtconfigarguments.h>
#include <qtutilities/resources/resources.h>

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QTextCodec>
#include <QtQml>
#ifdef Q_OS_ANDROID
#include <QDebug>
#include <QDirIterator>
#endif

#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS
#include <QApplication>
#endif

using namespace ApplicationUtilities;

namespace QtGui {

int runQuickGui(int argc, char *argv[], const QtConfigArguments &qtConfigArgs, const QString &file)
{
    // setup Android-specifics (logging, theming)
#ifdef Q_OS_ANDROID
    setupAndroidSpecifics();
#endif

    // work around kirigami plugin trying to be clever
    if (!qEnvironmentVariableIsSet("XDG_CURRENT_DESKTOP")) {
        qputenv("XDG_CURRENT_DESKTOP", QByteArray("please don't override my settings"));
    }

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

    // assume we're bundling breeze icons
    if (QIcon::themeName().isEmpty()) {
        QIcon::setThemeName(QStringLiteral("breeze"));
    }

    // log resource information
#if defined(Q_OS_ANDROID) && defined(DEBUG_BUILD)
    qDebug() << "Using icon theme" << QIcon::themeName();
    qDebug() << "Icon theme search paths" << QIcon::themeSearchPaths();
    qDebug() << "Resources:";
    QDirIterator it(QStringLiteral(":/"), QDirIterator::Subdirectories);
    while (it.hasNext()) {
        qDebug() << it.next();
    }
#endif

    // load settings from configuration file
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral(PROJECT_NAME));

    // load translations
    LOAD_QT_TRANSLATIONS;

    // init Quick GUI
    QQmlApplicationEngine engine;
    Controller controller(settings, file);
#ifdef Q_OS_ANDROID
    registerControllerForAndroid(&controller);
#endif
    auto *const context(engine.rootContext());
    context->setContextProperty(QStringLiteral("nativeInterface"), &controller);
    context->setContextProperty(QStringLiteral("app"), &a);
    context->setContextProperty(QStringLiteral("description"), QStringLiteral(APP_DESCRIPTION));
    context->setContextProperty(QStringLiteral("dependencyVersions"), QStringList(DEPENCENCY_VERSIONS));
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    // run event loop
    return a.exec();
}
} // namespace QtGui

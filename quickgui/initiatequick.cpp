#include "./initiatequick.h"
#include "./controller.h"
#ifdef Q_OS_ANDROID
#include "./android.h"
#endif

#include "resources/config.h"
#include "resources/qtconfig.h"

// enable inline helper functions for Qt Quick provided by qtutilities
#define QT_UTILITIES_GUI_QTQUICK

#include <qtutilities/misc/desktoputils.h>
#include <qtutilities/resources/qtconfigarguments.h>
#include <qtutilities/resources/resources.h>
#include <qtutilities/settingsdialog/qtsettings.h>

#include <passwordfile/util/openssl.h>

#include <QDebug>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QtQml>
#ifdef Q_OS_ANDROID
#include <QDirIterator>
#endif

#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS
#include <QApplication>
#endif

#include <cstdlib>

using namespace CppUtilities;
using namespace Util;

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

    // init OpenSSL
    OpenSsl::init();

    // init application
    SET_QT_APPLICATION_INFO;
#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS
    auto application = QApplication(argc, argv);
#else
    auto application = QGuiApplication(argc, argv);
#endif

    // restore Qt settings
    auto qtSettings = QtUtilities::QtSettings();
    auto settings = QtUtilities::getSettings(QStringLiteral(PROJECT_NAME));
    if (auto settingsError = QtUtilities::errorMessageForSettings(*settings); !settingsError.isEmpty()) {
        qDebug() << settingsError;
    }
    qtSettings.restore(*settings);
    qtSettings.apply();
#if defined(Q_OS_ANDROID)
    qtSettings.reapplyDefaultIconTheme(QtUtilities::isDarkModeEnabled().value_or(false));
#endif

    // apply settings specified via command line args
    qtConfigArgs.applySettings(qtSettings.hasCustomFont());
    qtConfigArgs.applySettingsForQuickGui();
    LOAD_QT_TRANSLATIONS;

    // init Quick GUI
    auto controller = Controller(*settings, file);
    auto engine = QQmlApplicationEngine();
#ifdef Q_OS_ANDROID
    registerControllerForAndroid(&controller);
#endif
    auto *const context = engine.rootContext();
    context->setContextProperty(QStringLiteral("nativeInterface"), &controller);
    context->setContextProperty(QStringLiteral("app"), &application);
    context->setContextProperty(QStringLiteral("description"), QStringLiteral(APP_DESCRIPTION));
    context->setContextProperty(QStringLiteral("dependencyVersions"), QStringList(DEPENCENCY_VERSIONS));
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    const auto importPaths = qEnvironmentVariable(PROJECT_VARNAME_UPPER "_QML_IMPORT_PATHS").split(QChar(':'));
    for (const auto &path : importPaths) {
        engine.addImportPath(path);
    }
#endif

    QObject::connect(&application, &QCoreApplication::aboutToQuit, &OpenSsl::clean);
    // load main QML file; run event loop or exit if it cannot be loaded
    const auto mainUrl = QUrl(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &application,
        [&mainUrl](QObject *obj, const QUrl &objUrl) {
            if (!obj && objUrl == mainUrl) {
                QCoreApplication::exit(EXIT_FAILURE);
            }
        },
        Qt::QueuedConnection);
    engine.load(mainUrl);
    return application.exec();
}
} // namespace QtGui

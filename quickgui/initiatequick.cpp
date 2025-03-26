#include "./initiatequick.h"
#include "./controller.h"
#ifdef Q_OS_ANDROID
#include "./android.h"
#endif

#include "resources/config.h"
#include "resources/qtconfig.h"

// enable inline helper functions for Qt Quick provided by qtutilities
#define QT_UTILITIES_GUI_QTQUICK

// ensure QGuiApplication is defined before resources.h for desktop file name
#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS
#include <QApplication>
using App = QApplication;
#else
#include <QGuiApplication>
using App = QGuiApplication;
#endif

#include <qtutilities/misc/desktoputils.h>
#include <qtutilities/resources/qtconfigarguments.h>
#include <qtutilities/resources/resources.h>
#include <qtutilities/settingsdialog/qtsettings.h>

#include <passwordfile/util/openssl.h>

#include <QDebug>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>

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

    // init OpenSSL
    OpenSsl::init();

    // init application
    SET_QT_APPLICATION_INFO;
    auto application = App(argc, argv);
    QObject::connect(&application, &QCoreApplication::aboutToQuit, &OpenSsl::clean);

    // restore Qt settings
    auto qtSettings = QtUtilities::QtSettings();
    auto settings = QtUtilities::getSettings(QStringLiteral(PROJECT_NAME));
    if (auto settingsError = QtUtilities::errorMessageForSettings(*settings); !settingsError.isEmpty()) {
        qDebug() << settingsError;
    }
    qtSettings.restore(*settings);
    qtSettings.apply();

    // create controller and handle dark mode
    // note: Not handling changes of the dark mode setting dynamically yet because it does not work with Kirigami.
    //       It looks like Kirigami does not follow the QCC2 theme (the Material.theme/Material.theme settings) but
    //       instead uses colors based on the initial palette. Not sure how to toggle Kirigami's palette in accordance
    //       with the QCC2 theme. Hence this code is disabled via APPLY_COLOR_SCHEME_DYNAMICALLY for now.
    auto controller = Controller(*settings, file);
#ifdef APPLY_COLOR_SCHEME_DYNAMICALLY
    QtUtilities::onDarkModeChanged(
        [&qtSettings, &controller](bool isDarkModeEnabled) {
            qtSettings.reapplyDefaultIconTheme(isDarkModeEnabled);
            controller.setDarkModeEnabled(isDarkModeEnabled);
        },
        &controller);
#else
    const auto isDarkModeEnabled = QtUtilities::isDarkModeEnabled().value_or(false);
    qtSettings.reapplyDefaultIconTheme(isDarkModeEnabled);
    controller.setDarkModeEnabled(isDarkModeEnabled);
#endif

    // apply settings specified via command line args
    qtConfigArgs.applySettings(qtSettings.hasCustomFont());
    qtConfigArgs.applySettingsForQuickGui();
    LOAD_QT_TRANSLATIONS;

    // init QML engine
    QtUtilities::deletePipelineCacheIfNeeded();
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

    // load main QML file; run event loop or exit if it cannot be loaded
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &application,
        [](QObject *obj, const QUrl &objUrl) {
            if (!obj) {
                std::cerr << "Unable to load " << objUrl.toString().toStdString() << '\n';
                QCoreApplication::exit(EXIT_FAILURE);
            }
        },
        Qt::QueuedConnection);
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &application, &QGuiApplication::quit);
    engine.loadFromModule("Main", "Main");
    return application.exec();
}
} // namespace QtGui

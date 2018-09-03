#include "./initiatequick.h"
#include "./controller.h"
#ifdef Q_OS_ANDROID
#include "./android.h"
#endif

#include "resources/config.h"

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

#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS
#include <QApplication>
#endif

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

using namespace ApplicationUtilities;

namespace QtGui {

#ifdef Q_OS_ANDROID
namespace Android {
namespace WindowManager {
namespace LayoutParams {
enum RelevantFlags {
    TranslucentStatus = 0x04000000,
    DrawsSystemBarBackgrounds = 0x80000000,
};
}
} // namespace WindowManager
} // namespace Android
#endif

int runQuickGui(int argc, char *argv[], const QtConfigArguments &qtConfigArgs, const QString &file)
{
    // setup logging for Android
#ifdef Q_OS_ANDROID
    qInstallMessageHandler(writeToAndroidLog);
#endif

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

#ifdef Q_OS_ANDROID
    // assume we're bundling breeze icons under Android
    if (QIcon::themeName().isEmpty()) {
        QIcon::setThemeName(QStringLiteral("breeze"));
    }
#endif

    // load settings from configuration file
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral(PROJECT_NAME));

    // load translations
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
    Controller controller(settings, file);
#ifdef Q_OS_ANDROID
    registerControllerForAndroid(&controller);
#endif
    auto *const context(engine.rootContext());
    context->setContextProperty(QStringLiteral("userPaths"), userPaths);
    context->setContextProperty(QStringLiteral("nativeInterface"), &controller);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

#ifdef Q_OS_ANDROID
    QtAndroid::runOnAndroidThread([=]() {
        QAndroidJniObject window = QtAndroid::androidActivity().callObjectMethod("getWindow", "()Landroid/view/Window;");
        window.callMethod<void>("addFlags", "(I)V", Android::WindowManager::LayoutParams::DrawsSystemBarBackgrounds);
        window.callMethod<void>("clearFlags", "(I)V", Android::WindowManager::LayoutParams::TranslucentStatus);
        window.callMethod<void>("setStatusBarColor", "(I)V", QColor("#2196f3").rgba());
        window.callMethod<void>("setNavigationBarColor", "(I)V", QColor("#2196f3").rgba());
    });
#endif

    // run event loop
    return a.exec();
}
} // namespace QtGui

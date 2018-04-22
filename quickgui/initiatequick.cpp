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

int runQuickGui(int argc, char *argv[], const QtConfigArguments &qtConfigArgs, const QString &file)
{
    // init application
#ifdef __ANDROID__
    qputenv("QT_QUICK_CONTROLS_STYLE", "material");
#endif
    SET_QT_APPLICATION_INFO;
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#if defined(GUI_QTWIDGETS)
    QApplication a(argc, argv);
#else
    QGuiApplication a(argc, argv);
#endif

    // apply settings specified via command line args
    qtConfigArgs.applySettings();

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
    qmlRegisterSingletonType<QtGui::ApplicationInfo>("martchus.passwordmanager", 2, 0, "ApplicationInfo", applicationInfo);
    qmlRegisterType<QtGui::EntryFilterModel>("martchus.passwordmanager", 2, 0, "EntryFilterModel");
    qmlRegisterType<QtGui::EntryModel>("martchus.passwordmanager", 2, 0, "EntryModel");
    qmlRegisterType<QtGui::FieldModel>("martchus.passwordmanager", 2, 0, "FieldModel");
    QQmlApplicationEngine engine(QUrl("qrc:/qml/main.qml"));
    engine.rootContext()->setContextProperty(QStringLiteral("userPaths"), userPaths);
    engine.rootContext()->setContextProperty(QStringLiteral("file"), file);

    // start event loop
    int res = a.exec();
    return res;
}
} // namespace QtGui

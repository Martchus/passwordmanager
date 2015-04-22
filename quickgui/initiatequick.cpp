#include "initiate.h"

# include "model/entryfiltermodel.h"
# include "model/entrymodel.h"
# include "model/fieldmodel.h"

# include "quickgui/applicationinfo.h"

#include <qtutilities/resources/resources.h>

#if defined(GUI_QTWIDGETS)
# include <QApplication>
#else
# include <QGuiApplication>
#endif
#include <QGuiApplication>
#include <QTextCodec>
#include <QtQml>
#include <QQmlApplicationEngine>
#include <QDebug>

namespace QtGui {

#if defined(GUI_QTQUICK)
static QObject *applicationInfo(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new ApplicationInfo();
}
#endif

int runQuickGui(int argc, char *argv[])
{
    // init application
#if defined(GUI_QTWIDGETS)
    QApplication a(argc, argv);
#else
    QGuiApplication a(argc, argv);
#endif
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QGuiApplication::setOrganizationName(QStringLiteral("Martchus"));
    QGuiApplication::setOrganizationDomain(QStringLiteral("http://martchus.netai.net/"));
    QGuiApplication::setApplicationName(QStringLiteral("Password Manager (mobile)"));
    QGuiApplication::setApplicationVersion(QStringLiteral("2.0.5"));
    // load translation files
    TranslationFiles::loadQtTranslationFile();
    TranslationFiles::loadApplicationTranslationFile(QStringLiteral("passwordmanager"));
    // load the other resources
    QtUtilitiesResources::init();
    Theme::setup();
    qDebug() << "test 2";
    // init quick GUI
    qmlRegisterSingletonType<QtGui::ApplicationInfo>("martchus.passwordmanager", 2, 0, "ApplicationInfo", applicationInfo);
    qmlRegisterType<QtGui::EntryFilterModel>("martchus.passwordmanager", 2, 0, "EntryFilterModel");
    qmlRegisterType<QtGui::EntryModel>("martchus.passwordmanager", 2, 0, "EntryModel");
    qmlRegisterType<QtGui::FieldModel>("martchus.passwordmanager", 2, 0, "FieldModel");
    QQmlApplicationEngine engine(QUrl("qrc:/qml/main.qml"));
    // start event loop
    int res = a.exec();
    // cleanup resources
    QtUtilitiesResources::cleanup();
    return res;
}

}

#include "./android.h"
#include "./controller.h"

#include "resources/config.h"

#include <c++utilities/conversion/stringbuilder.h>

#include <QColor>
#include <QCoreApplication>
#include <QJniObject>
#include <QMessageLogContext>
#include <QMetaObject>

#include <android/log.h>

#include <jni.h>

using namespace CppUtilities;

namespace QtGui {

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

static Controller *controllerForAndroid = nullptr;

void applyThemingForAndroid()
{
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
        const auto color = QColor(QLatin1String("#2c714a")).rgba();
        QJniObject window = QJniObject(QNativeInterface::QAndroidApplication::context()).callObjectMethod("getWindow", "()Landroid/view/Window;");
        window.callMethod<void>("addFlags", "(I)V", Android::WindowManager::LayoutParams::DrawsSystemBarBackgrounds);
        window.callMethod<void>("clearFlags", "(I)V", Android::WindowManager::LayoutParams::TranslucentStatus);
        window.callMethod<void>("setStatusBarColor", "(I)V", color);
        window.callMethod<void>("setNavigationBarColor", "(I)V", color);
    });
}

void registerControllerForAndroid(Controller *controller)
{
    controllerForAndroid = controller;
}

bool showAndroidFileDialog(bool existing, bool createNew)
{
    return QJniObject(QNativeInterface::QAndroidApplication::context()).callMethod<jboolean>("showAndroidFileDialog", "(ZZ)Z", existing, createNew);
}

int openFileDescriptorFromAndroidContentUrl(const QString &url, const QString &mode)
{
    return QJniObject(QNativeInterface::QAndroidApplication::context())
        .callMethod<jint>("openFileDescriptorFromAndroidContentUri", "(Ljava/lang/String;Ljava/lang/String;)I",
            QJniObject::fromString(url).object<jstring>(), QJniObject::fromString(mode).object<jstring>());
}

void writeToAndroidLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    constexpr auto tag = PROJECT_NAME "-" APP_VERSION;
    auto report = msg.toStdString();
    if (context.file && *context.file) {
        report += argsToString(" in file ", context.file, " line ", context.line);
    }
    if (context.function && !QString(context.function).isEmpty()) {
        report += argsToString(" function ", context.function);
    }

    switch (type) {
    case QtDebugMsg:
        __android_log_write(ANDROID_LOG_DEBUG, tag, report.data());
        break;
    case QtInfoMsg:
        __android_log_write(ANDROID_LOG_INFO, tag, report.data());
        break;
    case QtWarningMsg:
        __android_log_write(ANDROID_LOG_WARN, tag, report.data());
        break;
    case QtCriticalMsg:
        __android_log_write(ANDROID_LOG_ERROR, tag, report.data());
        break;
    case QtFatalMsg:
        __android_log_write(ANDROID_LOG_FATAL, tag, report.data());
        abort();
    }
}

void setupAndroidSpecifics()
{
    qInstallMessageHandler(writeToAndroidLog);
    applyThemingForAndroid();
}

} // namespace QtGui

static void onAndroidError(JNIEnv *, jobject, jstring message)
{
    QMetaObject::invokeMethod(QtGui::controllerForAndroid, "newNotification", Qt::QueuedConnection, Q_ARG(QString, QJniObject(message).toString()));
}

static void onAndroidFileDialogAccepted(JNIEnv *, jobject, jstring fileName, jboolean existing, jboolean createNew)
{
    QMetaObject::invokeMethod(QtGui::controllerForAndroid, "handleFileSelectionAccepted", Qt::QueuedConnection,
        Q_ARG(QString, QJniObject(fileName).toString()), Q_ARG(bool, existing), Q_ARG(bool, createNew));
}

static void onAndroidFileDialogAcceptedDescriptor(
    JNIEnv *, jobject, jstring nativeUrl, jstring fileName, jint fileHandle, jboolean existing, jboolean createNew)
{
    QMetaObject::invokeMethod(QtGui::controllerForAndroid, "handleFileSelectionAcceptedDescriptor", Qt::QueuedConnection,
        Q_ARG(QString, QJniObject(nativeUrl).toString()), Q_ARG(QString, QJniObject(fileName).toString()), Q_ARG(int, fileHandle),
        Q_ARG(bool, existing), Q_ARG(bool, createNew));
}

static void onAndroidFileDialogRejected(JNIEnv *, jobject)
{
    QMetaObject::invokeMethod(QtGui::controllerForAndroid, "handleFileSelectionCanceled", Qt::QueuedConnection);
}

/*!
 * \brief Registers the static functions declared above so they can be called from the Java-side.
 * \remarks This method is called automatically by Java after the .so file is loaded.
 */
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *)
{
    // get the JNIEnv pointer
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    // search for Java class which declares the native methods
    const auto javaClass = env->FindClass("org/martchus/passwordmanager/Activity");
    if (!javaClass) {
        return JNI_ERR;
    }

    // register native methods
    static const JNINativeMethod methods[] = {
        { "onAndroidError", "(Ljava/lang/String;)V", reinterpret_cast<void *>(onAndroidError) },
        { "onAndroidFileDialogAccepted", "(Ljava/lang/String;ZZ)V", reinterpret_cast<void *>(onAndroidFileDialogAccepted) },
        { "onAndroidFileDialogAcceptedDescriptor", "(Ljava/lang/String;Ljava/lang/String;IZZ)V",
            reinterpret_cast<void *>(onAndroidFileDialogAcceptedDescriptor) },
        { "onAndroidFileDialogRejected", "()V", reinterpret_cast<void *>(onAndroidFileDialogRejected) },
    };
    if (env->RegisterNatives(javaClass, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

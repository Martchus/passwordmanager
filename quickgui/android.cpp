#include "./android.h"
#include "./controller.h"

#include "resources/config.h"

#include <c++utilities/conversion/stringbuilder.h>

#include <QtAndroid>
#include <QAndroidJniObject>
#include <QCoreApplication>
#include <QMetaObject>
#include <QMessageLogContext>

#include <android/log.h>

#include <jni.h>

using namespace ConversionUtilities;

namespace QtGui {

static Controller *controllerForAndroid = nullptr;

void applyThemingForAndroid()
{
    QtAndroid::androidActivity().callObjectMethod("applyTheming", "()");
}

void registerControllerForAndroid(Controller *controller)
{
    controllerForAndroid = controller;
}

bool showAndroidFileDialog(bool existing)
{
    return QtAndroid::androidActivity().callMethod<jboolean>("showAndroidFileDialog", "(Z)Z", existing);
}

int openFileDescriptorFromAndroidContentUrl(const QString &url, const QString &mode)
{
    return QtAndroid::androidActivity().callMethod<jint>("openFileDescriptorFromAndroidContentUri", "(Ljava/lang/String;Ljava/lang/String;)I", QAndroidJniObject::fromString(url).object<jstring>(), QAndroidJniObject::fromString(mode).object<jstring>());
}

void writeToAndroidLog(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
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
    __android_log_write(ANDROID_LOG_INFO,tag,report.data());
    break;
  case QtWarningMsg:
    __android_log_write(ANDROID_LOG_WARN,tag,report.data());
    break;
  case QtCriticalMsg:
    __android_log_write(ANDROID_LOG_ERROR,tag,report.data());
    break;
  case QtFatalMsg:
    __android_log_write(ANDROID_LOG_FATAL,tag,report.data());
    abort();
  }
}

}

static void onAndroidError(JNIEnv *, jobject, jstring message)
{
    QMetaObject::invokeMethod(QtGui::controllerForAndroid, "newNotification", Qt::QueuedConnection, Q_ARG(QString, QAndroidJniObject::fromLocalRef(message).toString()));
}

static void onAndroidFileDialogAccepted(JNIEnv *, jobject, jstring fileName, jboolean existing)
{
    QMetaObject::invokeMethod(QtGui::controllerForAndroid, "handleFileSelectionAccepted", Qt::QueuedConnection, Q_ARG(QString, QAndroidJniObject::fromLocalRef(fileName).toString()), Q_ARG(bool, existing));
}

static void onAndroidFileDialogAcceptedDescriptor(JNIEnv *, jobject, jstring nativeUrl, jstring fileName, jint fileHandle, jboolean existing)
{
    QMetaObject::invokeMethod(QtGui::controllerForAndroid, "handleFileSelectionAcceptedDescriptor", Qt::QueuedConnection, Q_ARG(QString, QAndroidJniObject::fromLocalRef(nativeUrl).toString()), Q_ARG(QString, QAndroidJniObject::fromLocalRef(fileName).toString()), Q_ARG(int, fileHandle), Q_ARG(bool, existing));
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
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    // search for Java class which declares the native methods
    const auto javaClass = env->FindClass("org/martchus/passwordmanager/Activity");
    if (!javaClass) {
        return JNI_ERR;
    }

    // register native methods
    static const JNINativeMethod methods[] = {
        {"onAndroidError", "(Ljava/lang/String;)V", reinterpret_cast<void *>(onAndroidError)},
        {"onAndroidFileDialogAccepted", "(Ljava/lang/String;Z)V", reinterpret_cast<void *>(onAndroidFileDialogAccepted)},
        {"onAndroidFileDialogAcceptedDescriptor", "(Ljava/lang/String;Ljava/lang/String;IZ)V", reinterpret_cast<void *>(onAndroidFileDialogAcceptedDescriptor)},
        {"onAndroidFileDialogRejected", "()V", reinterpret_cast<void *>(onAndroidFileDialogRejected)},
    };
    if (env->RegisterNatives(javaClass, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

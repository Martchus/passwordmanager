#include "./android.h"
#include "./controller.h"

#include <QtAndroid>
#include <QAndroidJniObject>
#include <QMetaObject>

#include <jni.h>

namespace QtGui {

static Controller *controllerForAndroid = nullptr;

void registerControllerForAndroid(Controller *controller)
{
    controllerForAndroid = controller;
}

bool showAndroidFileDialog(bool existing)
{
    return QtAndroid::androidActivity().callMethod<jboolean>("showAndroidFileDialog", "(Z)Z", existing);
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

static void onAndroidFileDialogAcceptedHandle(JNIEnv *, jobject, jstring fileName, jint fileHandle, jboolean existing)
{
    QMetaObject::invokeMethod(QtGui::controllerForAndroid, "handleFileSelectionAcceptedDescriptor", Qt::QueuedConnection, Q_ARG(QString, QAndroidJniObject::fromLocalRef(fileName).toString()), Q_ARG(int, fileHandle), Q_ARG(bool, existing));
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
        {"onAndroidFileDialogAcceptedHandle", "(Ljava/lang/String;IZ)V", reinterpret_cast<void *>(onAndroidFileDialogAcceptedHandle)},
        {"onAndroidFileDialogRejected", "()V", reinterpret_cast<void *>(onAndroidFileDialogRejected)},
    };
    if (env->RegisterNatives(javaClass, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

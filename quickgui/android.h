#ifndef QT_QUICK_GUI_ANDROID_H
#define QT_QUICK_GUI_ANDROID_H

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QMessageLogContext)
QT_FORWARD_DECLARE_CLASS(QString)

namespace QtGui {

class Controller;

void applyThemingForAndroid();
void registerControllerForAndroid(Controller *controller);
bool showAndroidFileDialog(bool existing);
int openFileDescriptorFromAndroidContentUrl(const QString &url, const QString &mode);
void writeToAndroidLog(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void setupAndroidSpecifics();
} // namespace QtGui

#endif // QT_QUICK_GUI_ANDROID_H

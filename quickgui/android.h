#ifndef QT_QUICK_GUI_ANDROID_H
#define QT_QUICK_GUI_ANDROID_H

#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QMessageLogContext;
class QString;
QT_END_NAMESPACE

namespace QtGui {

class Controller;

void applyThemingForAndroid();
void registerControllerForAndroid(Controller *controller);
bool showAndroidFileDialog(bool existing);
int openFileDescriptorFromAndroidContentUrl(const QString &url, const QString &mode);
void writeToAndroidLog(QtMsgType type, const QMessageLogContext &context, const QString &msg);

}

#endif // QT_QUICK_GUI_ANDROID_H

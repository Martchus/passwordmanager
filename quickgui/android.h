#ifndef QT_QUICK_GUI_ANDROID_H
#define QT_QUICK_GUI_ANDROID_H

namespace QtGui {

class Controller;
void registerControllerForAndroid(Controller *controller);
bool showAndroidFileDialog(bool existing);

}

#endif // QT_QUICK_GUI_ANDROID_H

#ifndef QT_QUICK_GUI_INITIATE_H
#define QT_QUICK_GUI_INITIATE_H

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QString)

namespace ApplicationUtilities {
class QtConfigArguments;
}

namespace QtGui {

int runQuickGui(int argc, char *argv[], const ApplicationUtilities::QtConfigArguments &qtConfigArgs);

}

#endif // QT_QUICK_GUI_INITIATE_H

#ifndef QT_QUICK_GUI_INITIATE_H
#define QT_QUICK_GUI_INITIATE_H

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QString)

namespace CppUtilities {
class QtConfigArguments;
}

namespace QtGui {

int runQuickGui(int argc, char *argv[], const CppUtilities::QtConfigArguments &qtConfigArgs, const QString &file);
}

#endif // QT_QUICK_GUI_INITIATE_H

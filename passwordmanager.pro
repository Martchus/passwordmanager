projectname = passwordmanager
appname = "Password Manager"
appauthor = Martchus
appurl = "https://github.com/$${appauthor}/$${projectname}"
QMAKE_TARGET_DESCRIPTION = "A simple password manager."
VERSION = 2.0.7

# include ../../common.pri when building as part of a subdirs project; otherwise include general.pri
!include(../../common.pri) {
    !include(./general.pri) {
        error("Couldn't find the common.pri or the general.pri file!")
    }
}

TEMPLATE = app

no-gui {
    CONFIG -= qt # Qt is only required for GUI
} else {
    QT += core gui
    android {
        OTHER_FILES += android/AndroidManifest.xml
        include(../../deployment.pri)
    }
}

guiqtquick {
    QML_IMPORT_PATH += ./quick ./qml ./qml/pages ./qml/touch
}

# files
SOURCES += main.cpp\
    model/entrymodel.cpp \
    model/fieldmodel.cpp \
    model/entryfiltermodel.cpp \
    util/testroutines.cpp \
    cli/cli.cpp


guiqtwidgets {
    SOURCES += gui/mainwindow.cpp \
        gui/passwordgeneratordialog.cpp \
        gui/undocommands.cpp \
        gui/stacksupport.cpp \
        gui/initiatequi.cpp

    FORMS += gui/mainwindow.ui \
        gui/passwordgeneratordialog.ui
}

guiqtquick {
    SOURCES += quickgui/applicationinfo.cpp \
        quickgui/initiatequick.cpp
}

HEADERS  += model/entrymodel.h \
    model/fieldmodel.h \
    model/entryfiltermodel.h \
    util/testroutines.h \
    cli/cli.h \
    gui/initiategui.h

guiqtwidgets {
    HEADERS += gui/mainwindow.h \
        gui/passwordgeneratordialog.h \
        gui/undocommands.h \
        gui/stacksupport.h
}

guiqtquick {
    HEADERS += quickgui/applicationinfo.h \
        quickgui/applicationpaths.h \
        quickgui/initiatequick.h
}

# resources and translations
!no-gui {
    RESOURCES += resources/icons.qrc
    contains(DEFINES, GUI_QTQUICK) {
        RESOURCES += resources/qml.qrc
    }
    TRANSLATIONS = translations/passwordmanager_en_US.ts \
        translations/passwordmanager_de_DE.ts
}
include(translations.pri)

win32:include(windowsicon.pri)

OTHER_FILES += \
    README.md \
    LICENSE

# libs and includepath
CONFIG(debug, debug|release) {
    LIBS += -lc++utilitiesd -lpasswordfiled
    !no-gui {
        LIBS += -lqtutilitiesd
    }
} else {
    LIBS += -lc++utilities -lpasswordfile
    !no-gui {
        LIBS += -lqtutilities
    }
}

# installs
mingw-w64-install {
    target.path = $$(INSTALL_ROOT)
    target.extra = install -m755 -D $${OUT_PWD}/release/$(TARGET) $$(INSTALL_ROOT)/bin/$(TARGET)
    INSTALLS += target
} else {
    target.path = $$(INSTALL_ROOT)/bin
    INSTALLS += target
}
!mingw-w64-install {
    icon.path = $$(INSTALL_ROOT)/share/icons/hicolor/scalable/apps/
    icon.files = $${PWD}/resources/icons/hicolor/scalable/apps/$${projectname}.svg
    INSTALLS += icon
    menu.path = $$(INSTALL_ROOT)/share/applications/
    menu.files = $${PWD}/resources/desktop/applications/$${projectname}.desktop
    INSTALLS += menu
}

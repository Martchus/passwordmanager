# meta data
projectname = passwordmanager
appname = "Password Manager"
appauthor = Martchus
appurl = "https://github.com/$${appauthor}/$${projectname}"
QMAKE_TARGET_DESCRIPTION = "A simple password manager."
VERSION = 2.0.9

# include ../../common.pri when building as part of a subdirs project; otherwise include general.pri
!include(../../common.pri) {
    !include(./general.pri) {
        error("Couldn't find the common.pri or the general.pri file!")
    }
}

# basic configuration: application, might be configured with no Qt
TEMPLATE = app
no-gui {
    # Qt is only required for GUI
    CONFIG -= qt
} else {
    QT += core gui
}

guiqtquick {
    QML_IMPORT_PATH += ./quick ./qml ./qml/pages ./qml/touch
}

# add project files
HEADERS  += \
    model/entrymodel.h \
    model/fieldmodel.h \
    model/entryfiltermodel.h \
    cli/cli.h

SOURCES += \
    main.cpp \
    model/entrymodel.cpp \
    model/fieldmodel.cpp \
    model/entryfiltermodel.cpp \
    cli/cli.cpp


testing {
    HEADERS += \
        util/testroutines.h

    SOURCES += \
        util/testroutines.cpp
}

guiqtwidgets {
    HEADERS += \
        gui/mainwindow.h \
        gui/passwordgeneratordialog.h \
        gui/undocommands.h \
        gui/stacksupport.h \
        gui/initiategui.h \
        gui/fielddelegate.h

    SOURCES += \
        gui/mainwindow.cpp \
        gui/passwordgeneratordialog.cpp \
        gui/undocommands.cpp \
        gui/stacksupport.cpp \
        gui/initiatequi.cpp \
        gui/fielddelegate.cpp

    FORMS += \
        gui/mainwindow.ui \
        gui/passwordgeneratordialog.ui
}

guiqtquick {
    HEADERS += \
        quickgui/applicationinfo.h \
        quickgui/applicationpaths.h \
        quickgui/initiatequick.h

    SOURCES += \
        quickgui/applicationinfo.cpp \
        quickgui/initiatequick.cpp

    RESOURCES += \
        resources/qml.qrc
}

!no-gui {
    RESOURCES += \
        resources/icons.qrc

    TRANSLATIONS += \
        translations/passwordmanager_en_US.ts \
        translations/passwordmanager_de_DE.ts
}

OTHER_FILES += \
    README.md \
    LICENSE \
    CMakeLists.txt \
    resources/config.h.in \
    resources/windows.rc.in \
    android/AndroidManifest.xml

# add deployment for android
android:include(../../deployment.pri)

# release translations
include(translations.pri)

# make windows icon
win32:include(windowsicon.pri)

# add libs
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

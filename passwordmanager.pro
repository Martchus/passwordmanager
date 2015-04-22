projectname = passwordmanager

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
    cli/cli.h

guiqtwidgets {
    HEADERS += gui/mainwindow.h \
        gui/passwordgeneratordialog.h \
        gui/undocommands.h \
        gui/stacksupport.h \
        gui/initiate.h
}

guiqtquick {
    HEADERS += quickgui/applicationinfo.h \
        quickgui/applicationpaths.h
        quickgui/initiate.h
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
updateqm.commands = lrelease $${projectname}.pro
updateqm.target = updateqm
QMAKE_EXTRA_TARGETS += updateqm

OTHER_FILES += \
    pkgbuild/default/PKGBUILD \
    pkgbuild/mingw-w64/PKGBUILD

# libs and includepath
CONFIG(debug, debug|release) {
    LIBS += -L../../ -lc++utilitiesd -lpasswordfiled
    !no-gui {
        LIBS += -lqtutilitiesd
    }
} else {
    LIBS += -L../../ -lc++utilities -lpasswordfile
    !no-gui {
        LIBS += -lqtutilities
    }
}

# requried when building in subdirs project
INCLUDEPATH += ../

# installs
target.path = $$(INSTALL_ROOT)/bin
INSTALLS += target
icon.path = $$(INSTALL_ROOT)/share/icons/hicolor/scalable/apps/
icon.files = ./resources/icons/hicolor/scalable/apps/$${projectname}.svg
INSTALLS += icon
menu.path = $$(INSTALL_ROOT)/share/applications/
menu.files = ./resources/desktop/applications/$${projectname}.desktop
INSTALLS += menu
translations.path = $$(INSTALL_ROOT)/share/$${projectname}/translations/
translations.files = ./translations/*.qm
INSTALLS += translations

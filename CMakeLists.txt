cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# meta data
set(META_PROJECT_NAME passwordmanager)
set(META_APP_NAME "Password Manager")
set(META_APP_CATEGORIES "Utility")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "A simple password store using AES-256-CBC encryption via OpenSSL.")
set(META_VERSION_MAJOR 2)
set(META_VERSION_MINOR 1)
set(META_VERSION_PATCH 1)
set(META_APP_VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH})

# add project files
set(HEADER_FILES
    cli/cli.h
    model/entryfiltermodel.h
    model/entrymodel.h
    model/fieldmodel.h
)
set(SRC_FILES
    cli/cli.cpp
    main.cpp
    model/entryfiltermodel.cpp
    model/entrymodel.cpp
    model/fieldmodel.cpp
)

set(WIDGETS_HEADER_FILES
    gui/fielddelegate.h
    gui/initiategui.h
    gui/mainwindow.h
    gui/passwordgeneratordialog.h
    gui/stacksupport.h
    gui/undocommands.h
)
set(WIDGETS_SRC_FILES
    gui/fielddelegate.cpp
    gui/initiatequi.cpp
    gui/mainwindow.cpp
    gui/passwordgeneratordialog.cpp
    gui/stacksupport.cpp
    gui/undocommands.cpp
    resources/icons.qrc
)
set(WIDGETS_UI_FILES
    gui/mainwindow.ui
    gui/passwordgeneratordialog.ui
)

set(QML_HEADER_FILES
    quickgui/applicationinfo.h
    quickgui/applicationpaths.h
    quickgui/initiatequick.h
)
set(QML_SRC_FILES
    quickgui/applicationinfo.cpp
    quickgui/initiatequick.cpp
    resources/icons.qrc
    resources/qml.qrc
)

set(TS_FILES
    translations/${META_PROJECT_NAME}_de_DE.ts
    translations/${META_PROJECT_NAME}_en_US.ts
)

set(ICON_FILES
    resources/icons/hicolor/scalable/apps/${META_PROJECT_NAME}.svg
)

# find c++utilities
find_package(c++utilities REQUIRED)
use_cpp_utilities()

# find qtutilities
find_package(qtutilities REQUIRED)
use_qt_utilities()

# find passwordfile
find_package(passwordfile REQUIRED)
use_password_file()

# include modules to apply configuration
include(BasicConfig)
include(QtGuiConfig)
include(QtConfig)
include(WindowsResources)
include(AppTarget)

# create desktop file using previously defined meta data
add_desktop_file()

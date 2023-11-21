cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

# set meta data
project(passwordmanager)
set(META_PROJECT_NAME ${PROJECT_NAME})
set(META_PROJECT_VARNAME password_manager)
set(META_PROJECT_TYPE application)
set(META_APP_NAME "Password Manager")
set(META_APP_CATEGORIES "Utility;Security;")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "A simple password store using AES-256-CBC encryption via OpenSSL")
set(META_GUI_OPTIONAL YES)
set(META_USE_QQC2 ON)
set(META_ANDROID_PACKAGE_NAME "org.martchus.passwordmanager")
set(META_VERSION_MAJOR 4)
set(META_VERSION_MINOR 1)
set(META_VERSION_PATCH 13)
set(META_RELEASE_DATE "2023-11-21")

# add project files
set(HEADER_FILES cli/cli.h model/entryfiltermodel.h model/entrymodel.h model/fieldmodel.h)
set(SRC_FILES cli/cli.cpp main.cpp)

set(GUI_HEADER_FILES model/entryfiltermodel.cpp model/entrymodel.cpp model/fieldmodel.cpp)
set(GUI_SRC_FILES model/entryfiltermodel.cpp model/entrymodel.cpp model/fieldmodel.cpp)

set(WIDGETS_HEADER_FILES gui/fielddelegate.h gui/initiategui.h gui/mainwindow.h gui/passwordgeneratordialog.h
                         gui/stacksupport.h gui/undocommands.h)
set(WIDGETS_SRC_FILES
    gui/fielddelegate.cpp
    gui/initiategui.cpp
    gui/mainwindow.cpp
    gui/passwordgeneratordialog.cpp
    gui/stacksupport.cpp
    gui/undocommands.cpp
    resources/icons.qrc)
set(WIDGETS_UI_FILES gui/mainwindow.ui gui/passwordgeneratordialog.ui)

set(QML_HEADER_FILES quickgui/controller.h quickgui/initiatequick.h)
set(QML_SRC_FILES quickgui/controller.cpp quickgui/initiatequick.cpp resources/icons.qrc resources/qml.qrc)
if (ANDROID)
    list(APPEND QML_HEADER_FILES quickgui/android.h)
    list(APPEND QML_SRC_FILES quickgui/android.cpp)
endif ()

set(TS_FILES translations/${META_PROJECT_NAME}_de_DE.ts translations/${META_PROJECT_NAME}_en_US.ts)

set(ICON_FILES resources/icons/hicolor/scalable/apps/${META_PROJECT_NAME}.svg)

set(DOC_FILES README.md)

set(REQUIRED_ICONS
    application-exit
    applications-internet
    dialog-cancel
    dialog-ok
    dialog-ok-apply
    dialog-password
    document-close
    document-encrypt
    document-export
    document-new
    document-open
    document-open-recent
    document-properties
    document-save
    document-save-as
    edit-clear
    edit-copy
    edit-cut
    edit-delete
    edit-paste
    edit-redo
    edit-rename
    edit-undo
    emblem-warning
    flag-black
    flag-blue
    folder
    folder-add
    folder-symbolic
    go-next
    go-next-symbolic
    go-next-symbolic-rtl
    go-previous
    go-previous-symbolic
    go-previous-symbolic-rtl
    handle-right
    handle-sort
    overflow-menu-right
    overflow-menu-left
    help-about
    insert-text
    list-add
    list-remove
    password-copy
    password-generate
    password-show-off
    password-show-on
    preferences-desktop-icons
    preferences-desktop-locale
    qtcreator
    search
    story-editor
    system-file-manager
    system-run
    system-search
    username-copy
    window-close)

# find c++utilities
set(CONFIGURATION_PACKAGE_SUFFIX
    ""
    CACHE STRING "sets the suffix for find_package() calls to packages configured via c++utilities")
find_package(c++utilities${CONFIGURATION_PACKAGE_SUFFIX} 5.6.0 REQUIRED)
use_cpp_utilities()

# apply basic configuration
include(BasicConfig)

# find qtutilities
set(CONFIGURATION_PACKAGE_SUFFIX_QTUTILITIES
    "${CONFIGURATION_PACKAGE_SUFFIX}"
    CACHE STRING "sets the suffix for qtutilities")
find_package(qtutilities${CONFIGURATION_PACKAGE_SUFFIX_QTUTILITIES} 6.12.0 REQUIRED)
use_qt_utilities()

# find passwordfile
find_package(passwordfile${CONFIGURATION_PACKAGE_SUFFIX} 5.0.0 REQUIRED)
use_password_file()

# require at least Qt 5.8 for the Qt Quick GUI
if (QUICK_GUI)
    set(META_QT5_VERSION 5.8)
endif ()

# allow to enable undo support from the widgets GUI in the quick GUI as well (so the quick GUI will depend on Qt Widgets as
# well)
if (QUICK_GUI AND NOT WIDGETS_GUI)
    option(ENABLE_UNDO_SUPPORT_FOR_QUICK_GUI "enables with undo/redo support for the Qt Quick GUI (requires Qt Widgets)" ON)
    if (ENABLE_UNDO_SUPPORT_FOR_QUICK_GUI)
        list(APPEND ADDITIONAL_QT_MODULES Widgets)
        list(APPEND META_PUBLIC_COMPILE_DEFINITIONS ${META_PROJECT_VARNAME_UPPER}_ENABLE_UNDO_SUPPORT_FOR_QUICK_GUI)
        list(APPEND QML_HEADER_FILES gui/stacksupport.h gui/undocommands.h)
        list(APPEND QML_SRC_FILES gui/stacksupport.cpp gui/undocommands.cpp)
    endif ()
endif ()

# add further Qt/KF modules required by the Qt Quick GUI under Android
if (ANDROID)
    list(APPEND ADDITIONAL_QT_MODULES AndroidExtras)
endif ()
if (QUICK_GUI)
    list(APPEND ADDITIONAL_KF_MODULES Kirigami2)
endif ()

# add Qt-version-specific QML files
unset(QML_FILE)
if (NOT QT_PACKAGE_PREFIX)
    set(QML_FILE "resources/qml5.qrc")
elseif (QT_PACKAGE_PREFIX MATCHES ".*Qt([0-9]+).*")
    set(QML_FILE "resources/qml${CMAKE_MATCH_1}.qrc")
endif ()
if (NOT QML_FILE)
    message(FATAL_ERROR "Unable to add Qt-version-specific resource file for QT_PACKAGE_PREFIX \"${QT_PACKAGE_PREFIX}\".")
endif ()
message(STATUS "Adding Qt-version-specific resource file \"${QML_FILE}\" to build")
list(APPEND QML_SRC_FILES "${QML_FILE}")

# add other QML files to "EXCLUDED_FILES" so they're still considered for translations in any case
file(GLOB OTHER_QML_FILES "resources/qml*.qrc")
list(REMOVE_ITEM OTHER_QML_FILES ${QML_SRC_FILES})
list(APPEND EXCLUDED_FILES ${OTHER_QML_FILES})

# apply further configuration
if (WIDGETS_GUI OR QUICK_GUI)
    include(QtGuiConfig)
    include(QtConfig)
endif ()
include(WindowsResources)
include(AppTarget)
include(AndroidApk)
include(ShellCompletion)
include(ConfigHeader)

# create desktop file using previously defined meta data
add_desktop_file()

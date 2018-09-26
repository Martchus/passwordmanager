cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# meta data
set(META_PROJECT_NAME passwordmanager)
set(META_PROJECT_VARNAME password_manager)
set(META_PROJECT_TYPE application)
set(META_APP_NAME "Password Manager")
set(META_APP_CATEGORIES "Utility;Security;")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "A simple password store using AES-256-CBC encryption via OpenSSL")
set(META_GUI_OPTIONAL YES)
set(META_USE_QQC2 ON)
set(META_VERSION_MAJOR 3)
set(META_VERSION_MINOR 2)
set(META_VERSION_PATCH 0)
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
)

set(GUI_HEADER_FILES
    model/entryfiltermodel.cpp
    model/entrymodel.cpp
    model/fieldmodel.cpp
)
set(GUI_SRC_FILES
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
    quickgui/controller.h
    quickgui/initiatequick.h
)
set(QML_SRC_FILES
    quickgui/controller.cpp
    quickgui/initiatequick.cpp
    resources/icons.qrc
    resources/qml.qrc
)
if(ANDROID)
    list(APPEND QML_HEADER_FILES
        quickgui/android.h
    )
    list(APPEND QML_SRC_FILES
        quickgui/android.cpp
    )
endif()

set(TS_FILES
    translations/${META_PROJECT_NAME}_de_DE.ts
    translations/${META_PROJECT_NAME}_en_US.ts
)

set(ICON_FILES
    resources/icons/hicolor/scalable/apps/${META_PROJECT_NAME}.svg
)

set(DOC_FILES
    README.md
)

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
    password-generate
    password-show-off
    password-show-on
    preferences-desktop-icons
    preferences-desktop-locale
    qtcreator
    story-editor
    system-file-manager
    system-run
    system-search
    window-close
)

# find c++utilities
find_package(c++utilities 4.10.0 REQUIRED)
use_cpp_utilities()
include(BasicConfig)

# find qtutilities
find_package(qtutilities 5.11.0 REQUIRED)
use_qt_utilities()

# find passwordfile
find_package(passwordfile 3.1.0 REQUIRED)
use_password_file()

# require Qt AndroidExtras
if(ANDROID AND QUICK_GUI)
    list(APPEND ADDITIONAL_QT_MODULES AndroidExtras)
    list(APPEND ADDITIONAL_KF_MODULES Kirigami2)
endif()

# include modules to apply configuration
if(WIDGETS_GUI OR QUICK_GUI)
    include(QtGuiConfig)
    include(QtConfig)
endif()

include(WindowsResources)
include(AppTarget)
include(AndroidApk)
include(ShellCompletion)
include(ConfigHeader)

# create desktop file using previously defined meta data
add_desktop_file()

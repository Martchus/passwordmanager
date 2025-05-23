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
set(META_APP_DOMAIN "${META_APP_URL}")
set(META_APP_DESCRIPTION "A simple password store using AES-256-CBC encryption via OpenSSL")
set(META_GUI_OPTIONAL YES)
set(META_USE_QQC2 ON)
set(META_ANDROID_PACKAGE_NAME "org.martchus.passwordmanager")
set(META_VERSION_MAJOR 4)
set(META_VERSION_MINOR 2)
set(META_VERSION_PATCH 4)
set(META_RELEASE_DATE "2025-04-03")

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
set(QML_MODULE_FILES
    qml/AboutDialog.qml
    qml/BasicDialog.qml
    qml/EntriesPage.qml
    qml/EntryDelegate.qml
    qml/FieldsDelegate.qml
    qml/FieldsPage.qml
    qml/FileDialog.qml
    qml/Main.qml
    qml/PasswordDialog.qml)
list(APPEND EXCLUDED_FILES ${QML_MODULE_FILES}) # enable translations for qml files
if (ANDROID)
    list(APPEND QML_HEADER_FILES quickgui/android.h)
    list(APPEND QML_SRC_FILES quickgui/android.cpp)
else ()
    list(APPEND EXCLUDED_FILES quickgui/android.h quickgui/android.cpp)
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
    view-list-details-symbolic
    system-file-manager
    system-run
    system-search
    username-copy
    window-close)

# find c++utilities
set(CONFIGURATION_PACKAGE_SUFFIX
    ""
    CACHE STRING "sets the suffix for find_package() calls to packages configured via c++utilities")
find_package(c++utilities${CONFIGURATION_PACKAGE_SUFFIX} 5.28.0 REQUIRED)
use_cpp_utilities()

# apply basic configuration
set(BUILD_QUICK_GUI_BY_DEFAULT ON)
if (WIN32 OR APPLE)
    set(BUILD_QUICK_GUI_BY_DEFAULT OFF)
endif ()
option(QUICK_GUI "enables/disables building the Qt Quick GUI using Kirigami" "${BUILD_QUICK_GUI_BY_DEFAULT}")
include(BasicConfig)

# find qtutilities
set(CONFIGURATION_PACKAGE_SUFFIX_QTUTILITIES
    "${CONFIGURATION_PACKAGE_SUFFIX}"
    CACHE STRING "sets the suffix for qtutilities")
find_package(qtutilities${CONFIGURATION_PACKAGE_SUFFIX_QTUTILITIES} 6.15.0 REQUIRED)
use_qt_utilities()

# find passwordfile
find_package(passwordfile${CONFIGURATION_PACKAGE_SUFFIX} 5.0.0 REQUIRED)
use_password_file()

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

# deduce major Qt version from package prefix
if (NOT QT_PACKAGE_PREFIX)
    set(MAJOR_QT_VERSION "5")
elseif (QT_PACKAGE_PREFIX MATCHES ".*Qt([0-9]+).*")
    set(MAJOR_QT_VERSION "${CMAKE_MATCH_1}")
endif ()

# require Qt 6 for the Qt Quick GUI
if (QUICK_GUI AND (MAJOR_QT_VERSION VERSION_LESS 6 OR MAJOR_QT_VERSION VERSION_GREATER_EQUAL 7))
    message(FATAL_ERROR "The Qt Quick GUI is only compatible with Qt 6 (but Qt ${MAJOR_QT_VERSION} was found).")
endif ()

# workaround "ld: error: undefined symbol: qt_resourceFeatureZstd" when Qt 6 is not configured with zstd support
if (MAJOR_QT_VERSION GREATER_EQUAL 6 AND NOT QT_FEATURE_zstd)
    set(CMAKE_AUTORCC_OPTIONS "--no-zstd")
endif ()

# add further Qt/KF modules required by Qt Quick GUI
if (QUICK_GUI)
    list(APPEND ADDITIONAL_KF_MODULES Kirigami)
endif ()

# apply further configuration
if (WIDGETS_GUI OR QUICK_GUI)
    include(QtGuiConfig)
    include(QtConfig)
endif ()
include(WindowsResources)
include(AppTarget)
include(ShellCompletion)
include(ConfigHeader)

# add Qml module
if (QUICK_GUI)
    qt_add_qml_module(
        ${META_TARGET_NAME}
        URI
        "Main"
        VERSION
        1.0
        NO_PLUGIN
        QML_FILES
        ${QML_MODULE_FILES}
        RESOURCE_PREFIX
        "/qt/qml/")
endif ()

# configure creating an Android package using androiddeployqt
if (ANDROID)
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    set_target_properties(
        ${META_TARGET_NAME}
        PROPERTIES QT_ANDROID_PACKAGE_NAME "${META_PROJECT_RDNS}"
                   QT_ANDROID_VERSION_NAME "${META_APP_VERSION}"
                   QT_ANDROID_VERSION_CODE "1"
                   QT_ANDROID_PACKAGE_SOURCE_DIR "${ANDROID_PACKAGE_SOURCE_DIR}"
                   QT_ANDROID_TARGET_SDK_VERSION "34")

    set(ANDROID_MANIFEST_PATH "${ANDROID_PACKAGE_SOURCE_DIR}/AndroidManifest.xml")
    configure_file("resources/AndroidManifest.xml.in" "${ANDROID_MANIFEST_PATH}" @ONLY)

    # specify required plugins so androiddeployqt knows what we need and don't need
    # cmake-format: off
    qt_import_plugins(${META_TARGET_NAME}
        INCLUDE_BY_TYPE imageformats Qt::QSvgPlugin
        INCLUDE_BY_TYPE iconengines Qt::QSvgIconPlugin
        INCLUDE_BY_TYPE tls
        INCLUDE_BY_TYPE networkinformation
        INCLUDE_BY_TYPE platforms Qt::QAndroidIntegrationPlugin
    )
    # cmake-format: on
    if (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
        qt_import_plugins(${META_TARGET_NAME} EXCLUDE_BY_TYPE qmltooling)
    endif ()

    # bundle OpenMP (used by Kirigami) explicitly as it is otherwise not bundled
    find_package(OpenMP)
    if (OpenMP_CXX_FOUND)
        message(STATUS "Bundling OpenMP library for Kirigami: ${OpenMP_omp_LIBRARY}")
        set_target_properties(${META_TARGET_NAME} PROPERTIES QT_ANDROID_EXTRA_LIBS "${OpenMP_omp_LIBRARY}")
    endif ()

    set(QT_ANDROID_SIGN_APK ON)
    if (COMMAND _qt_internal_configure_android_multiabi_target)
        _qt_internal_configure_android_multiabi_target(${META_TARGET_NAME}) # required as of Qt 6.9.0 (probably Qt bug)
    endif ()
    qt_android_generate_deployment_settings(${META_TARGET_NAME})
    qt_android_add_apk_target(${META_TARGET_NAME})
endif ()

# create desktop file using previously defined meta data
add_desktop_file()

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# meta data
set(META_PROJECT_NAME passwordmanager)
set(META_APP_NAME "Password Manager")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "A simple password manager.")
set(META_VERSION_MAJOR 2)
set(META_VERSION_MINOR 0)
set(META_VERSION_PATCH 9)
set(META_APP_VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH})

# stringification of meta data
set(META_PROJECT_NAME_STR "\"${META_PROJECT_NAME}\"")
set(META_APP_NAME_STR "\"${META_APP_NAME}\"")
set(META_APP_AUTHOR_STR "\"${META_APP_AUTHOR}\"")
set(META_APP_URL_STR "\"${META_APP_URL}\"")
set(META_APP_DESCRIPTION_STR "\"${META_APP_DESCRIPTION}\"")
set(META_APP_VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH})
set(META_APP_VERSION_STR "\"${META_APP_VERSION}\"")

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
set(QUICK_HEADER_FILES
    quickgui/applicationinfo.h
    quickgui/applicationpaths.h
    quickgui/initiatequick.h
)
set(QUICK_SRC_FILES
    quickgui/applicationinfo.cpp
    quickgui/initiatequick.cpp
    resources/icons.qrc
    resources/qml.qrc
)
set(TS_FILES
    translations/${META_PROJECT_NAME}_de_DE.ts
    translations/${META_PROJECT_NAME}_en_US.ts
)

# define project
project(${META_PROJECT_NAME})

# add configuration header
configure_file(
    "${PROJECT_SOURCE_DIR}/resources/config.h.in"
    "${PROJECT_BINARY_DIR}/resources/config.h"
)
include_directories("${PROJECT_BINARY_DIR}")

# set "GUI-type" to WIN32 to hide console under windows
if(WIN32)
    set(GUI_TYPE WIN32)
endif(WIN32)

# add windows resource file
if(MINGW)
    # creation windows icon from png with ffmpeg if available
    find_program(FFMPEG_BIN ffmpeg avconv)
    if(FFMPEG_BIN)
        set(PNG_ICON_PATH "${PROJECT_SOURCE_DIR}/resources/icons/hicolor/128x128/apps/${META_PROJECT_NAME}.png")
        if(EXISTS ${PNG_ICON_PATH})
            set(WINDOWS_ICON_PATH "${PROJECT_BINARY_DIR}/${META_PROJECT_NAME}.ico")
            set(WINDOWS_ICON_RC_ENTRY "IDI_ICON1   ICON    DISCARDABLE \"${WINDOWS_ICON_PATH}\"")
            add_custom_command(
                OUTPUT "${WINDOWS_ICON_PATH}"
                COMMAND ${FFMPEG_BIN} -y -i "${PNG_ICON_PATH}" -vf crop=iw-20:ih-20:10:10,scale=64:64 "${WINDOWS_ICON_PATH}"
            )
        endif()
    endif(FFMPEG_BIN)
    # create windows rc file from template
    set(WINDOWS_EXT "exe")
    configure_file(
        "${PROJECT_SOURCE_DIR}/resources/windows.rc.in"
        "${PROJECT_BINARY_DIR}/resources/windows.rc"
    )
    # set windres as resource compiler
    set(RES_FILES "${PROJECT_BINARY_DIR}/resources/windows.rc")
    set(CMAKE_RC_COMPILER_INIT windres)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
    enable_language(RC)
endif(MINGW)

# enable Qt Widgets GUI
add_definitions(
    -DGUI_QTWIDGETS
    -DMODEL_UNDO_SUPPORT
)

# check required Qt 5 modules
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5LinguistTools REQUIRED)

# enable moc, uic and rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# enable lrelease
qt5_add_translation(QM_FILES ${TS_FILES})
ADD_CUSTOM_TARGET(translations ALL DEPENDS ${QM_FILES})

# executable and linking
add_executable(${META_PROJECT_NAME} ${GUI_TYPE} ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_HEADER_FILES} ${WIDGETS_SRC_FILES} ${QM_FILES} ${RES_FILES} ${WINDOWS_ICON_PATH})
target_link_libraries(${META_PROJECT_NAME} c++utilities qtutilities passwordfile Qt5::Core Qt5::Widgets)
set_target_properties(${META_PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
)

# add install target
install(TARGETS ${META_PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT binary
)
install(FILES resources/icons/hicolor/scalable/apps/${META_PROJECT_NAME}.svg
    DESTINATION share/icons/hicolor/scalable/apps
    COMPONENT desktop
)
install(FILES resources/desktop/applications/${META_PROJECT_NAME}.desktop
    DESTINATION share/applications
    COMPONENT desktop
)
install(FILES ${QM_FILES}
    DESTINATION share/${META_PROJECT_NAME}/translations
    COMPONENT localization
)
add_custom_target(install-binary
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(install-desktop
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=desktop -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(install-localization
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=localization -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(strip-mingw-w64
    DEPENDS install-binary install-localization
)
add_custom_target(install-binary-strip
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_DO_STRIP=1 -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(install-mingw-w64-strip
    DEPENDS install-binary-strip install-localization
)

# manual stripping


# ------------------------------------------------------------------------------
# CMake
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

# ------------------------------------------------------------------------------
# Project
# ------------------------------------------------------------------------------
project(yarock)
set(YAROCK_MAJOR_VERSION 1)
set(YAROCK_MINOR_VERSION 5)
set(YAROCK_PATCH_VERSION 0)
set(YAROCK_VERSION "${YAROCK_MAJOR_VERSION}.${YAROCK_MINOR_VERSION}.${YAROCK_PATCH_VERSION}")

# ------------------------------------------------------------------------------
# Configuration
# ------------------------------------------------------------------------------
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# ------------------------------------------------------------------------------
# Definitions
# ------------------------------------------------------------------------------
#add_definitions(-DTEST_FLAG)
add_definitions(-Wall -Wextra)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------

option(ENABLE_MPV "Enable/disable MPV audio engine" ON)
option(ENABLE_VLC "Enable/disable VLC audio engine" ON)
option(ENABLE_PHONON "Enable/disable Phonon audio engine" ON)
option(ENABLE_QTMULTIMEDIA "Enable/disable QtMultimedia audio engine" ON)
option(ENABLE_SHORTCUT "Enable/disable global shortcut manager" OFF)

# ------------------------------------------------------------------------------
# Qt
# ------------------------------------------------------------------------------
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt6Widgets_EXECUTABLE_COMPILE_FLAGS}")
find_package(Qt6Gui REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6Xml REQUIRED)
find_package(Qt6Network REQUIRED)
find_package(Qt6Concurrent REQUIRED)
find_package(Qt6DBus REQUIRED)
find_package(Qt6Sql REQUIRED)
find_package(Qt6LinguistTools REQUIRED)
set(QT_LIBRARIES  ${Qt6Core_LIBRARIES}
                  ${Qt6Gui_LIBRARIES}
                  ${Qt6Widgets_LIBRARIES}
                  ${Qt6Network_LIBRARIES}
                  ${Qt6Xml_LIBRARIES}
                  ${Qt6Concurrent_LIBRARIES}
                  ${Qt6DBus_LIBRARIES}
                  ${Qt6Sql_LIBRARIES})
set(QT_INCLUDES   ${Qt6Core_INCLUDE_DIRS}
                  ${Qt6Gui_INCLUDE_DIRS}
                  ${Qt6Widgets_INCLUDE_DIRS}
                  ${Qt6Network_INCLUDE_DIRS}
                  ${Qt6Xml_INCLUDE_DIRS}
                  ${Qt6Concurrent_INCLUDE_DIRS}
                  ${Qt6DBus_INCLUDE_DIRS}
                  ${Qt6Sql_INCLUDE_DIRS})
add_definitions(  ${Qt6Core_DEFINITIONS}
                  ${Qt6Widgets_DEFINITIONS}
                  ${Qt6Network_DEFINITIONS}
                  ${Qt6Xml_DEFINITIONS}
                  ${Qt6Concurrent_DEFINITIONS})
include_directories(${QT_INCLUDES})
qt_standard_project_setup()

# ------------------------------------------------------------------------------
# Taglib
# ------------------------------------------------------------------------------
find_package(Taglib 1.6.0 REQUIRED)
if(TAGLIB_FOUND)
  include(CheckTagLibFileName)
  check_taglib_filename(COMPLEX_TAGLIB_FILENAME)
endif(TAGLIB_FOUND)
include_directories(${TAGLIB_INCLUDES})

# ------------------------------------------------------------------------------
# HtmlCxx
# ------------------------------------------------------------------------------
find_package(HtmlCxx)
include_directories(${HTMLCXX_INCLUDES})

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------
add_subdirectory(src)

message(STATUS "Audio Engines")
message(STATUS "Audio Engine Vlc: ${ENABLE_VLC} [use cmake .. -DENABLE_VLC=ON/OFF]" )
if(ENABLE_VLC)
  add_definitions(-DENABLE_VLC)
  add_subdirectory(src/core/player/vlc)
endif(ENABLE_VLC)

message(STATUS "Audio Engine Mpv: ${ENABLE_MPV} [use cmake .. -DENABLE_MPV=ON/OFF]")
if(ENABLE_MPV)
  add_definitions(-DENABLE_MPV)
  add_subdirectory(src/core/player/mpv)
endif(ENABLE_MPV)

message(STATUS "Audio Engine Phonon: ${ENABLE_PHONON} [use cmake .. -DENABLE_PHONON=ON/OFF]")
if(ENABLE_PHONON)
  add_definitions(-DENABLE_PHONON)
  add_subdirectory(src/core/player/phonon)
endif(ENABLE_PHONON)

message(STATUS "Audio Engine QtMultimedia: ${ENABLE_QTMULTIMEDIA} [use cmake .. -DENABLE_QTMULTIMEDIA=ON/OFF]")
if(ENABLE_QTMULTIMEDIA)
  add_definitions(-DENABLE_QTMULTIMEDIA)
  add_subdirectory(src/core/player/qtmultimedia)
endif (ENABLE_QTMULTIMEDIA)

if(ENABLE_SHORTCUT)
  add_definitions(-DENABLE_SHORTCUT)
  add_subdirectory(src/shortcuts)
  include_directories(${CMAKE_SOURCE_DIR}/src/shortcuts)
  set(LIB_INTERNAL_STATIC_SHORTCUT yarock_shortchut)
endif(ENABLE_SHORTCUT)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})
include_directories(${YAROCK_INCLUDES})

# ------------------------------------------------------------------------------
# Resources
# ------------------------------------------------------------------------------
set(YAROCK_RESOURCES yarock.qrc)
qt_add_resources(YAROCK_RESOURCES_RCC ${YAROCK_RESOURCES})

# ------------------------------------------------------------------------------
# MPRIS 2.0 DBUS interfaces
# ------------------------------------------------------------------------------
qt_add_dbus_adaptor(YAROCK_SOURCES src/dbus/org.mpris.MediaPlayer2.Player.xml src/dbus/mpris2.h Mpris2 mpris2Player Mpris2Player)
qt_add_dbus_adaptor(YAROCK_SOURCES src/dbus/org.mpris.MediaPlayer2.xml src/dbus/mpris2.h Mpris2 mpris2Root Mpris2Root)

# ------------------------------------------------------------------------------
#  Translations
# ------------------------------------------------------------------------------
file(GLOB TRANS_FILES ${CMAKE_SOURCE_DIR}/translation/yarock_*.ts)
set(TS_FILES ${TRANS_FILES})

set(UPDATE_TRANSLATIONS OFF) # no ts update
#set(UPDATE_TRANSLATIONS ON) # ts update

option(UPDATE_TRANSLATIONS "Update source translation ../translation/*.ts files")
set(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${YAROCK_SOURCES})

if(UPDATE_TRANSLATIONS)
  qt_create_translation(YAROCK_QM_FILES ${FILES_TO_TRANSLATE} ${TS_FILES})
else(UPDATE_TRANSLATIONS)
  qt_add_translation(YAROCK_QM_FILES ${TS_FILES})
endif(UPDATE_TRANSLATIONS)

# ------------------------------------------------------------------------------
# Building
# ------------------------------------------------------------------------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wall -Wno-sign-compare")

add_executable(yarock ${YAROCK_SOURCES} ${YAROCK_RESOURCES_RCC} ${YAROCK_QM_FILES})

set_target_properties(yarock PROPERTIES AUTOMOC TRUE ENABLE_EXPORTS TRUE)

set_property(TARGET ${LIB_INTERNAL_STATIC_SHORTCUT} PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/shortcuts)

if(ENABLE_SHORTCUT)
  target_link_libraries(yarock ${LIB_INTERNAL_STATIC_SHORTCUT})
endif(ENABLE_SHORTCUT)

target_link_libraries(yarock
  X11
  ${QT_LIBRARIES}
  ${TAGLIB_LIBRARIES}
  ${HTMLCXX_LIBRARIES}
)

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "default install path" FORCE)
endif()

set(CMAKE_INSTALL_BIN   "${CMAKE_INSTALL_PREFIX}/bin"                       CACHE PATH "binaries install path"     FORCE)
set(CMAKE_INSTALL_LIB   "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/yarock"   CACHE PATH "libraries install path"    FORCE)
set(CMAKE_INSTALL_TRANS "${CMAKE_INSTALL_PREFIX}/share/yarock/translations" CACHE PATH "translations install path" FORCE)

# -- install binary file
install(TARGETS yarock DESTINATION ${CMAKE_INSTALL_BIN})

# -- install translations files
install(FILES ${YAROCK_QM_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/yarock/translations")

# -- install destop file
install(FILES data/yarock.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")

# -- install appdata file
install(FILES data/yarock.metainfo.xml  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/metainfo")

# -- install icons files
install(FILES ${CMAKE_SOURCE_DIR}/icon/yarock_16x16.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/16x16/apps" RENAME application-x-yarock.png)
install(FILES ${CMAKE_SOURCE_DIR}/icon/yarock_32x32.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/32x32/apps" RENAME application-x-yarock.png)
install(FILES ${CMAKE_SOURCE_DIR}/icon/yarock_48x48.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps" RENAME application-x-yarock.png)
install(FILES ${CMAKE_SOURCE_DIR}/icon/yarock_64x64.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/64x64/apps" RENAME application-x-yarock.png)
install(FILES ${CMAKE_SOURCE_DIR}/icon/yarock_128x128.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/128x128/apps" RENAME application-x-yarock.png)
install(FILES ${CMAKE_SOURCE_DIR}/icon/yarock.svg DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps" RENAME application-x-yarock.svg)
install(FILES ${CMAKE_SOURCE_DIR}/icon/yarock_48x48.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/pixmaps/" RENAME application-x-yarock.png)

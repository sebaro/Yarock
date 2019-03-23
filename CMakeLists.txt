# ------------------------------------------------------------------------------
#  YAROCK CMAKE FILE
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.6)
set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

PROJECT( yarock )
set(YAROCK_MAJOR_VERSION 1)
set(YAROCK_MINOR_VERSION 4)
set(YAROCK_PATCH_VERSION 0)
set(YAROCK_VERSION "${YAROCK_MAJOR_VERSION}.${YAROCK_MINOR_VERSION}.${YAROCK_PATCH_VERSION}")

# ------------------------------------------------------------------------------
#  build def
# ------------------------------------------------------------------------------
#ADD_DEFINITIONS( -DTEST_FLAG )
ADD_DEFINITIONS(-Wall -Wextra )

# ------------------------------------------------------------------------------
#  OPTION
# ------------------------------------------------------------------------------
# Since yarock 1.1.5 change default audio engine to VLC
OPTION(ENABLE_PHONON "Enable/disable Phonon audio engine" OFF)
OPTION(ENABLE_VLC "Enable/disable VLC audio engine" ON)
OPTION(ENABLE_MPV "Enable/disable MPV audio engine" OFF)
# OPTION(ENABLE_QTMULTIMEDIA "Enable/disable QtMultimedia audio engine" OFF)

# Since yarock 1.1.5 change default qt5 
OPTION(ENABLE_QT5 "Enable/disable Qt5 build" ON)
OPTION(ENABLE_SHORTCUT "Enable/disable global shortcut manager" ON)

# ------------------------------------------------------------------------------
#  EXTERNAL PACKAGE
# ------------------------------------------------------------------------------


#---- Qt ----
if (ENABLE_QT5)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
    
    FIND_PACKAGE(Qt5Gui REQUIRED)
    FIND_PACKAGE(Qt5Widgets REQUIRED)
    FIND_PACKAGE(Qt5Xml REQUIRED)
    FIND_PACKAGE(Qt5Network REQUIRED)
    FIND_PACKAGE(Qt5Concurrent REQUIRED)
    FIND_PACKAGE(Qt5DBus REQUIRED)
    FIND_PACKAGE(Qt5Sql REQUIRED)
    FIND_PACKAGE(Qt5X11Extras REQUIRED)
    FIND_PACKAGE(Qt5LinguistTools REQUIRED)
    
    set (QTLIBS ${Qt5Core_LIBRARIES} 
                ${Qt5Widgets_LIBRARIES} 
                ${Qt5Network_LIBRARIES} 
                ${Qt5Xml_LIBRARIES} 
                ${Qt5Concurrent_LIBRARIES} 
                ${Qt5DBus_LIBRARIES} 
                ${Qt5Sql_LIBRARIES}
                ${Qt5X11Extras_LIBRARIES})
    
    set (QTINCLUDES ${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Widgets_INCLUDE_DIRS}
                    ${Qt5Network_INCLUDE_DIRS}
                    ${Qt5Xml_INCLUDE_DIRS}
                    ${Qt5Concurrent_INCLUDE_DIRS} 
                    ${Qt5DBus_INCLUDE_DIRS} 
                    ${Qt5Sql_INCLUDE_DIRS})

    add_definitions( ${Qt5Core_DEFINITIONS} 
                     ${Qt5Widgets_DEFINITIONS} 
                     ${Qt5Network_DEFINITIONS} 
                     ${Qt5Xml_DEFINITIONS} 
                     ${Qt5Concurrent_DEFINITIONS})
else (ENABLE_QT5)
    FIND_PACKAGE(Qt4 4.7.0 REQUIRED QtCore QtGui QtXml QtNetwork QtSql QtDbus)

    set (QTLIBS ${QT_QTCORE_LIBRARY} 
                ${QT_QTGUI_LIBRARY} 
                ${QT_QTNETWORK_LIBRARY} 
                ${QT_QTXML_LIBRARY} 
                ${QT_QTDBUS_LIBRARY} 
                ${QT_QTSQL_LIBRARY})

    set (QTINCLUDES ${QT_INCLUDES})
    
    add_definitions(${QT_DEFINITIONS})    

endif (ENABLE_QT5)

 
# ---- QJson ----
if(NOT ENABLE_QT5)
    FIND_PACKAGE( QJSON REQUIRED )
endif (NOT ENABLE_QT5)

# ---- Taglib ----
FIND_PACKAGE( Taglib 1.6.0 REQUIRED )

if(TAGLIB_FOUND)
    include(CheckTagLibFileName)
    check_taglib_filename(COMPLEX_TAGLIB_FILENAME)
endif (TAGLIB_FOUND)

# ---- Htmlcxx ----
FIND_PACKAGE( HtmlCxx )

        
# ------------------------------------------------------------------------------
#  INCLUDES
# ------------------------------------------------------------------------------
INCLUDE_DIRECTORIES( ${QTINCLUDES} )
INCLUDE_DIRECTORIES( ${TAGLIB_INCLUDES} )
if(NOT ENABLE_QT5)
  INCLUDE_DIRECTORIES( ${QJSON_INCLUDE_DIR} )
endif (NOT ENABLE_QT5)
INCLUDE_DIRECTORIES( ${HTMLCXX_INCLUDE_DIRS} )
 
# ------------------------------------------------------------------------------
#  SUBDIRECTORIES
# ------------------------------------------------------------------------------
# global shortcut
if(ENABLE_SHORTCUT)
  add_definitions(-DENABLE_SHORTCUT)

  add_subdirectory(src/shortcuts)

  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/shortcuts)
  
  set(LIB_INTERNAL_STATIC_SHORTCUT yarock_shortchut)

endif(ENABLE_SHORTCUT)

# ------------------------------------------------------------------------------
#  INSTALL DIRECTORIES
# ------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   set (CMAKE_INSTALL_PREFIX /usr CACHE PATH "default install path" FORCE )
endif()

set(CMAKE_INSTALL_BIN          "${CMAKE_INSTALL_PREFIX}/bin"         CACHE PATH "binaries install path" FORCE)
set(CMAKE_INSTALL_LIB          "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/yarock"  CACHE PATH "libraries install path" FORCE)
set(CMAKE_INSTALL_TRANS        "${CMAKE_INSTALL_PREFIX}/share/yarock/translations"  CACHE PATH "translations install path" FORCE)

# ------------------------------------------------------------------------------
#  configure file
# ------------------------------------------------------------------------------
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)
               
# ------------------------------------------------------------------------------
#  SOURCES
# ------------------------------------------------------------------------------
add_subdirectory(src)

MESSAGE(STATUS "Audio Engine Vlc.................................................  " ${ENABLE_VLC} " [use cmake .. -DENABLE_VLC=ON/OFF]" )
if (ENABLE_VLC)
  add_definitions(-DENABLE_VLC)
  add_subdirectory(src/core/player/vlc)
endif (ENABLE_VLC)

MESSAGE(STATUS "Audio Engine Mpv.................................................  " ${ENABLE_MPV} " [use cmake .. -DENABLE_MPV=ON/OFF]")
if (ENABLE_MPV)
  add_definitions(-DENABLE_MPV)
  add_subdirectory(src/core/player/mpv)
endif (ENABLE_MPV)

MESSAGE(STATUS "Audio Engine Phonon..............................................  " ${ENABLE_PHONON} " [use cmake .. -DENABLE_PHONON=ON/OFF]")
if (ENABLE_PHONON)
  add_definitions(-DENABLE_PHONON)
  add_subdirectory(src/core/player/phonon)
endif (ENABLE_PHONON)

# MESSAGE(STATUS "Audio Engine QtMultimedia........................................  " ${ENABLE_QTMULTIMEDIA} " [use cmake .. -DENABLE_QTMULTIMEDIA=ON/OFF]")
# if (ENABLE_QTMULTIMEDIA)
#     if (ENABLE_QT5)
#       add_definitions(-DENABLE_QTMULTIMEDIA)
#       add_subdirectory(src/core/player/qtmultimedia)
#     else (ENABLE_QT5)
#       MESSAGE(WARNING "QtMultimedia can be set with QT5")
#     endif (ENABLE_QT5)
# endif (ENABLE_QTMULTIMEDIA)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

INCLUDE_DIRECTORIES(${YAROCK_INCLUDES})

# ------------------------------------------------------------------------------
#  RESSOURCE
# ------------------------------------------------------------------------------
SET(YAROCK_RESOURCES yarock.qrc)


if (ENABLE_QT5)
  QT5_ADD_RESOURCES(YAROCK_RESOURCES_RCC ${YAROCK_RESOURCES})
else (ENABLE_QT5)
  QT4_ADD_RESOURCES(YAROCK_RESOURCES_RCC ${YAROCK_RESOURCES})
endif (ENABLE_QT5)


# ------------------------------------------------------------------------------
#  DBUS
# ------------------------------------------------------------------------------
# MPRIS 2.0 DBUS interfaces
if (ENABLE_QT5)
  qt5_add_dbus_adaptor(YAROCK_SOURCES
        src/dbus/org.mpris.MediaPlayer2.Player.xml
        src/dbus/mpris2.h Mpris2 mpris2Player Mpris2Player)
  qt5_add_dbus_adaptor(YAROCK_SOURCES
        src/dbus/org.mpris.MediaPlayer2.xml
        src/dbus/mpris2.h Mpris2 mpris2Root Mpris2Root)
else (ENABLE_QT5)
  qt4_add_dbus_adaptor(YAROCK_SOURCES
        src/dbus/org.mpris.MediaPlayer2.Player.xml
        src/dbus/mpris2.h Mpris2 mpris2Player Mpris2Player)
  qt4_add_dbus_adaptor(YAROCK_SOURCES
        src/dbus/org.mpris.MediaPlayer2.xml
        src/dbus/mpris2.h Mpris2 mpris2Root Mpris2Root)
endif (ENABLE_QT5)


# ------------------------------------------------------------------------------
#  TRANSLATION
# ------------------------------------------------------------------------------
# Handle Translations, take all * files from trans directory.
file( GLOB TRANS_FILES ${CMAKE_SOURCE_DIR}/translation/yarock_*.ts)
set(TS_FILES ${TRANS_FILES})

SET(UPDATE_TRANSLATIONS OFF) # no ts update
#SET(UPDATE_TRANSLATIONS ON) # ts update


option (UPDATE_TRANSLATIONS "Update source translation ../translation/*.ts files")
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${YAROCK_SOURCES})

if (ENABLE_QT5)
  if (UPDATE_TRANSLATIONS)
    QT5_CREATE_TRANSLATION(YAROCK_QM_FILES ${FILES_TO_TRANSLATE} ${TS_FILES})
  else (UPDATE_TRANSLATIONS)
    QT5_ADD_TRANSLATION(YAROCK_QM_FILES ${TS_FILES})
  endif (UPDATE_TRANSLATIONS)
else (ENABLE_QT5)
  if (UPDATE_TRANSLATIONS)
    QT4_CREATE_TRANSLATION(YAROCK_QM_FILES ${FILES_TO_TRANSLATE} ${TS_FILES})
  else (UPDATE_TRANSLATIONS)
    QT4_ADD_TRANSLATION(YAROCK_QM_FILES ${TS_FILES})
  endif (UPDATE_TRANSLATIONS)
endif (ENABLE_QT5)

# ------------------------------------------------------------------------------
#  BUILDING
# ------------------------------------------------------------------------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wall -Wno-sign-compare")
    
ADD_EXECUTABLE(yarock ${YAROCK_SOURCES} ${YAROCK_RESOURCES_RCC} ${YAROCK_QM_FILES})

SET_TARGET_PROPERTIES(yarock PROPERTIES AUTOMOC TRUE)

SET_PROPERTY(TARGET  ${LIB_INTERNAL_STATIC_SHORTCUT} PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/shortcuts)

if(ENABLE_SHORTCUT)
  TARGET_LINK_LIBRARIES(yarock ${LIB_INTERNAL_STATIC_SHORTCUT})
endif(ENABLE_SHORTCUT)


TARGET_LINK_LIBRARIES(yarock
  X11
  ${QTLIBS}
  ${TAGLIB_LIBRARIES}
  ${HTMLCXX_LIBRARIES}
)

if(NOT ENABLE_QT5)  
  TARGET_LINK_LIBRARIES(yarock ${QJSON_LIBRARIES})
endif(NOT ENABLE_QT5)

 
# ------------------------------------------------------------------------------
#  INSTALL
# ------------------------------------------------------------------------------
# -- install binary file
install( TARGETS yarock DESTINATION ${CMAKE_INSTALL_BIN} )

# -- install translations files
install(FILES ${YAROCK_QM_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/yarock/translations")

# -- install destop file
install(FILES data/org.yarock.desktop  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications" )

# -- install appdata file
install(FILES data/org.yarock.appdata.xml  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/appdata" )

# -- install icons files
INSTALL( FILES ${CMAKE_SOURCE_DIR}/icon/yarock_16x16.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/16x16/apps" RENAME application-x-yarock.png)
INSTALL( FILES ${CMAKE_SOURCE_DIR}/icon/yarock_32x32.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/32x32/apps" RENAME application-x-yarock.png)
INSTALL( FILES ${CMAKE_SOURCE_DIR}/icon/yarock_48x48.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps" RENAME application-x-yarock.png)
INSTALL( FILES ${CMAKE_SOURCE_DIR}/icon/yarock_64x64.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/64x64/apps" RENAME application-x-yarock.png)
INSTALL( FILES ${CMAKE_SOURCE_DIR}/icon/yarock_128x128.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/128x128/apps" RENAME application-x-yarock.png)
INSTALL( FILES ${CMAKE_SOURCE_DIR}/icon/yarock.svg DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps" RENAME application-x-yarock.svg)
INSTALL( FILES ${CMAKE_SOURCE_DIR}/icon/yarock_48x48.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/pixmaps/" RENAME application-x-yarock.png)

# ------------------------------------------------------------------------------
#  UN-INSTALL SUPPORT
# ------------------------------------------------------------------------------

 

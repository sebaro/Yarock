# ------------------------------------------------------------------------------
#  YAROCK CMAKE FILE
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.6)
set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

PROJECT( yarock )
set(YAROCK_MAJOR_VERSION 1)
set(YAROCK_MINOR_VERSION 0)
set(YAROCK_PATCH_VERSION 1)
set(YAROCK_VERSION ${YAROCK_MAJOR_VERSION}.${YAROCK_MINOR_VERSION}.${YAROCK_PATCH_VERSION})

# ------------------------------------------------------------------------------
#  build def
# ------------------------------------------------------------------------------
#ADD_DEFINITIONS( -DTEST_FLAG )
ADD_DEFINITIONS(-Wall -Wextra )

# ------------------------------------------------------------------------------
#  OPTION
# ------------------------------------------------------------------------------
OPTION(ENABLE_PHONON "Enable Phonon library" ON)
OPTION(ENABLE_VLC "Enable VLC library" OFF)

if (ENABLE_VLC)
    add_definitions(-DENABLE_VLC)
endif (ENABLE_VLC)

if (ENABLE_PHONON)
    add_definitions(-DENABLE_PHONON)
endif (ENABLE_PHONON)

MESSAGE( STATUS "Phonon system enable:" ${ENABLE_PHONON} " (use cmake .. -DENABLE_PHONON=ON/OFF)")
MESSAGE( STATUS "Vlc audio engine enable:" ${ENABLE_VLC} " (use cmake .. -DENABLE_VLC=ON/OFF)")

# ------------------------------------------------------------------------------
#  EXTERNAL PACKAGE
# ------------------------------------------------------------------------------
OPTION(ENABLE_QT5 "Enable build with Qt5" OFF)

#-- Qt
if (ENABLE_QT5)
    set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
    FIND_PACKAGE(Qt5Gui REQUIRED)
    FIND_PACKAGE(Qt5Widgets REQUIRED)
    FIND_PACKAGE(Qt5Xml REQUIRED)
    FIND_PACKAGE(Qt5Network REQUIRED)
    FIND_PACKAGE(Qt5Concurrent REQUIRED)
    FIND_PACKAGE(Qt5DBus REQUIRED)
    find_package(Qt5Sql REQUIRED)

    set (QTLIBS ${Qt5Core_LIBRARIES} 
                ${Qt5Widgets_LIBRARIES} 
                ${Qt5Network_LIBRARIES} 
                ${Qt5Xml_LIBRARIES} 
                ${Qt5Concurrent_LIBRARIES} 
                ${Qt5DBus_LIBRARIES} 
                ${Qt5Sql_LIBRARIES})
    
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
    add_definitions( -DQT_NO_STL -DQT_NO_CAST_TO_ASCII)
endif (ENABLE_QT5)

if (ENABLE_VLC)
    FIND_PACKAGE( LIBVLC REQUIRED)
endif (ENABLE_VLC)

if (ENABLE_PHONON)
if(ENABLE_QT5)
    find_package(Phonon4Qt5 REQUIRED)
else(ENABLE_QT5)
    FIND_PACKAGE( Phonon REQUIRED )
endif (ENABLE_QT5)    
endif (ENABLE_PHONON)
 
FIND_PACKAGE( Taglib 1.6.0 REQUIRED )

if(NOT ENABLE_QT5)
    FIND_PACKAGE( QJSON REQUIRED )
endif (NOT ENABLE_QT5)

if(TAGLIB_FOUND)
    include(CheckTagLibFileName)
    check_taglib_filename(COMPLEX_TAGLIB_FILENAME)
endif (TAGLIB_FOUND)

        
# ------------------------------------------------------------------------------
#  INCLUDES
# ------------------------------------------------------------------------------
#INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES( ${QTINCLUDES} )

if (ENABLE_VLC)
  INCLUDE_DIRECTORIES( ${LIBVLC_INCLUDE_DIR} )
endif (ENABLE_VLC)

if (ENABLE_PHONON)
  INCLUDE_DIRECTORIES( ${PHONON_INCLUDES} )
endif (ENABLE_PHONON)

INCLUDE_DIRECTORIES( ${TAGLIB_INCLUDES} )
INCLUDE_DIRECTORIES( ${QJSON_INCLUDE_DIR} )
 
# ------------------------------------------------------------------------------
#  SUBDIRECTORIES
# ------------------------------------------------------------------------------
option(USE_SYSTEM_QTSINGLEAPPLICATION "Use system QtSingleApplication library." OFF)
if(USE_SYSTEM_QTSINGLEAPPLICATION)
  find_path(QTSINGLEAPPLICATION_INCLUDE_DIRS qtsingleapplication.h PATH_SUFFIXES QtSolutions)
  find_library(QTSINGLEAPPLICATION_LIBRARIES QtSolutions_SingleApplication-2.6)
  find_library(QTSINGLECOREAPPLICATION_LIBRARIES QtSolutions_SingleCoreApplication-2.6)
else(USE_SYSTEM_QTSINGLEAPPLICATION)
  add_subdirectory(src3party/qtsingleapplication)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src3party/qtsingleapplication)
  set(QTSINGLEAPPLICATION_LIBRARIES qtsingleapplication)
endif(USE_SYSTEM_QTSINGLEAPPLICATION)

option(USE_SYSTEM_QXT "Use system Qxt library" OFF)
if (USE_SYSTEM_QXT)
  find_path(QXTCORE_INCLUDE_DIRS qxtglobal.h PATH_SUFFIXES QxtCore)
  find_path(QXTGUI_INCLUDE_DIRS qxtglobalshortcut.h PATH_SUFFIXES QxtGui)
  set(QXT_INCLUDE_DIRS ${QXTCORE_INCLUDE_DIRS} ${QXTGUI_INCLUDE_DIRS})
  # We only need its header. We don't need to link to QxtCore.
  find_library(QXT_LIBRARIES QxtGui)
else (USE_SYSTEM_QXT)
  add_subdirectory(src3party/qxt)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src3party/qxt)
  set(QXT_LIBRARIES qxt) 
endif (USE_SYSTEM_QXT)


# ------------------------------------------------------------------------------
#  SOURCES
# ------------------------------------------------------------------------------
add_subdirectory(src)
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

#SET(UPDATE_TRANSLATIONS OFF) # no ts update
SET(UPDATE_TRANSLATIONS ON) # ts update


option (UPDATE_TRANSLATIONS "Update source translation ../translation/*.ts files")
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${YAROCK_SOURCES})

if (ENABLE_QT5)
# TODO
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
add_definitions(${QT_DEFINITIONS})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wall -Wno-sign-compare")
    
ADD_EXECUTABLE(yarock ${YAROCK_SOURCES} ${YAROCK_RESOURCES_RCC} ${YAROCK_QM_FILES})

set_target_properties(yarock PROPERTIES AUTOMOC TRUE)

TARGET_LINK_LIBRARIES(yarock
  X11
  ${QTLIBS}
  ${TAGLIB_LIBRARIES}
  ${QTSINGLECOREAPPLICATION_LIBRARIES} ${QTSINGLEAPPLICATION_LIBRARIES}
  ${QXT_LIBRARIES}  
)

if(NOT ENABLE_QT5)  
  TARGET_LINK_LIBRARIES(yarock ${QJSON_LIBRARIES})
endif(NOT ENABLE_QT5)

  
if (ENABLE_VLC)
  TARGET_LINK_LIBRARIES(yarock ${LIBVLC_LIBRARY})
endif (ENABLE_VLC)  

if (ENABLE_PHONON)
  if(ENABLE_QT5)  
    TARGET_LINK_LIBRARIES(yarock ${PHONON4QT5_LIBRARY})
  else(ENABLE_QT5)
    TARGET_LINK_LIBRARIES(yarock ${PHONON_LIBS})
  endif(ENABLE_QT5)
endif (ENABLE_PHONON)

# ------------------------------------------------------------------------------
#  INSTALL
# ------------------------------------------------------------------------------
# -- install binary file
install( TARGETS yarock DESTINATION /usr/bin/ )

# -- install translations files
install(FILES ${YAROCK_QM_FILES} DESTINATION /usr/share/yarock/translations/)

# -- install destop file
install(FILES data/yarock.desktop  DESTINATION /usr/share/applications/)

# -- install appdata file
install(FILES data/yarock.appdata.xml  DESTINATION /usr/share/appdata/)

# -- install icons files
install(FILES icon/yarock_16x16.png DESTINATION share/icons/hicolor/16x16/apps/ RENAME application-x-yarock.png)
install(FILES icon/yarock_32x32.png DESTINATION share/icons/hicolor/32x32/apps/ RENAME application-x-yarock.png)
install(FILES icon/yarock_48x48.png DESTINATION share/icons/hicolor/48x48/apps/ RENAME application-x-yarock.png)
install(FILES icon/yarock_64x64.png DESTINATION share/icons/hicolor/64x64/apps/ RENAME application-x-yarock.png)
install(FILES icon/yarock_128x128.png DESTINATION share/icons/hicolor/128x128/apps/ RENAME application-x-yarock.png)


# ------------------------------------------------------------------------------
#  UN-INSTALL SUPPORT
# ------------------------------------------------------------------------------

 
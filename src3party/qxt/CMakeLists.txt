cmake_minimum_required(VERSION 2.6)

if (ENABLE_QT5)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
endif (ENABLE_QT5)

set(QXT-SOURCES
  qxtglobal.cpp
  qxtglobalshortcut.cpp
  qxtglobalshortcut_x11.cpp
)

set(QXT-MOC-HEADERS
  qxtglobalshortcut.h
)

find_package(X11)
include_directories(${X11_INCLUDE_DIR})
include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QTINCLUDES} )


if (ENABLE_QT5)
  FIND_PACKAGE(Qt5X11Extras REQUIRED)
  include_directories(${Qt5X11Extras_INCLUDE_DIRS})
  add_definitions( ${Qt5X11Extras_DEFINITIONS})
  MESSAGE(STATUS "  Found qt5x11extra include-dir path: ${Qt5X11Extras_INCLUDE_DIRS}")
endif (ENABLE_QT5)
  

if (ENABLE_QT5)
  qt5_wrap_cpp(QXT-SOURCES-MOC ${QXT-MOC-HEADERS})
else (ENABLE_QT5)
  qt4_wrap_cpp(QXT-SOURCES-MOC ${QXT-MOC-HEADERS})
endif (ENABLE_QT5)


ADD_LIBRARY(qxt
  ${QXT-SOURCES}
  ${QXT-SOURCES-MOC}
)

if (ENABLE_QT5)
  target_link_libraries (qxt "Qt5::X11Extras")
endif (ENABLE_QT5)

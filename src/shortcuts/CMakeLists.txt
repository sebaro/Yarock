cmake_minimum_required(VERSION 2.6)

ADD_DEFINITIONS( -DENABLE_SHORTCUT )

if (ENABLE_QT5)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
endif (ENABLE_QT5)

set(QXT-SOURCES
  globalshortcut.cpp
  globalshortcut_x11.cpp  
)

set(QXT-MOC-HEADERS
  globalshortcut.h
)

find_package(X11)
include_directories(${X11_INCLUDE_DIR})
include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QTINCLUDES} )


if (ENABLE_QT5)

  FIND_PACKAGE(Qt5X11Extras REQUIRED)
  FIND_PACKAGE(Qt5Core COMPONENTS PRIVATE_HEADERS REQUIRED)
  include_directories(${Qt5X11Extras_INCLUDE_DIRS})
  include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
  
  add_definitions( ${Qt5X11Extras_DEFINITIONS})
  MESSAGE(STATUS "  Found qt5x11extra include-dir path: ${Qt5X11Extras_INCLUDE_DIRS}")
  MESSAGE(STATUS "  Found qt5 private header include-dir path: ${Qt5Gui_PRIVATE_INCLUDE_DIRS}")
endif (ENABLE_QT5)
  

if (ENABLE_QT5)
  qt5_wrap_cpp(QXT-SOURCES-MOC ${QXT-MOC-HEADERS})
else (ENABLE_QT5)
  qt4_wrap_cpp(QXT-SOURCES-MOC ${QXT-MOC-HEADERS})
endif (ENABLE_QT5)


ADD_LIBRARY(yarock_shortchut STATIC
  ${QXT-SOURCES}
  ${QXT-SOURCES-MOC}
)

if (ENABLE_QT5)
  target_link_libraries (yarock_shortchut
  "Qt5::X11Extras")
endif (ENABLE_QT5)

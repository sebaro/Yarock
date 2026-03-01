
# ------------------------------------------------------------------------------
# CMake
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)

# ------------------------------------------------------------------------------
# Definitions
# ------------------------------------------------------------------------------
add_definitions(-DENABLE_SHORTCUTS)

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------
set(QXT-SOURCES
  globalshortcut.cpp
  globalshortcut_x11.cpp
)

set(QXT-MOC-HEADERS
  globalshortcut.h
)

# ------------------------------------------------------------------------------
# X11
# ------------------------------------------------------------------------------
find_package(X11)
include_directories(${X11_INCLUDE_DIR})

# ------------------------------------------------------------------------------
# Qt
# ------------------------------------------------------------------------------
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt6Widgets_EXECUTABLE_COMPILE_FLAGS}")
find_package(Qt6Core COMPONENTS PRIVATE_HEADERS REQUIRED)
find_package(Qt6Gui REQUIRED COMPONENTS GuiPrivate)
include_directories(${Qt6Gui_PRIVATE_INCLUDE_DIRS})
include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QTINCLUDES} )
qt6_wrap_cpp(QXT-SOURCES-MOC ${QXT-MOC-HEADERS})

# ------------------------------------------------------------------------------
# Building
# ------------------------------------------------------------------------------
add_library(yarock_shortcuts STATIC ${QXT-SOURCES} ${QXT-SOURCES-MOC})

target_link_libraries(yarock_shortcuts "Qt6::GuiPrivate")


# ------------------------------------------------------------------------------
# CMake
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Project
# ------------------------------------------------------------------------------
project(enginevlc)

# ------------------------------------------------------------------------------
# Definitions
# ------------------------------------------------------------------------------
add_definitions(-Wall)
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_NO_DEBUG)
add_definitions(-DQT_SHARED)
add_definitions(-DQT_THREAD)

add_definitions(-DENABLE_VLC)

# ------------------------------------------------------------------------------
# LibVLC
# ------------------------------------------------------------------------------
find_package(LibVLC REQUIRED)

include_directories(${LIBVLC_INCLUDE_DIR})

# ------------------------------------------------------------------------------
# Qt
# ------------------------------------------------------------------------------
set(QT_LIBRARIES ${Qt6Core_LIBRARIES})
set(QT_INCLUDES ${Qt6Core_INCLUDE_DIRS})
add_definitions(${Qt6Core_DEFINITIONS})
qt_standard_project_setup()

include_directories(${QT_INCLUDES})

# ------------------------------------------------------------------------------
# Configuration
# ------------------------------------------------------------------------------
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/vlc/config.h)

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/vlc
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/mediaitem
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/vlc/engine_vlc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/vlc/vlc_lib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/vlc/vlc_media.cpp)

# ------------------------------------------------------------------------------
# Building
# ------------------------------------------------------------------------------
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_library(enginevlc SHARED ${SOURCES})

set_target_properties(enginevlc PROPERTIES AUTOMOC TRUE)

add_dependencies(enginevlc yarock)

target_link_libraries(enginevlc ${LIBVLC_LIBRARY} ${QT_LIBRARIES})

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "default install path" FORCE)
endif()

set(CMAKE_INSTALL_LIB "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/yarock"  CACHE PATH "libraries installation path" FORCE)

install(TARGETS enginevlc DESTINATION ${CMAKE_INSTALL_LIB})

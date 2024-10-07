
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
project(engineqtmultimedia)

# ------------------------------------------------------------------------------
# Definitions
# ------------------------------------------------------------------------------
add_definitions(-Wall)
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_NO_DEBUG)
add_definitions(-DQT_SHARED)
add_definitions(-DQT_THREAD)

add_definitions(-DENABLE_QTMULTIMEDIA)

# ------------------------------------------------------------------------------
# QtMultimedia
# ------------------------------------------------------------------------------
find_package(Qt6Multimedia REQUIRED)

# ------------------------------------------------------------------------------
# Qt
# ------------------------------------------------------------------------------
set(QT_LIBRARIES ${Qt6Core_LIBRARIES}
                 ${Qt6Multimedia_LIBRARIES})
set(QT_INCLUDES ${Qt6Core_INCLUDE_DIRS}
                ${Qt6Multimedia_INCLUDE_DIRS})
add_definitions(${Qt6Core_DEFINITIONS}
                ${Qt6Multimedia_DEFINITIONS})
qt_standard_project_setup()

include_directories(${QT_INCLUDES})

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/qtmultimedia
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/mediaitem
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/qtmultimedia/engine_qtmultimedia.cpp)

# ------------------------------------------------------------------------------
# Building
# ------------------------------------------------------------------------------
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_library(engineqtmultimedia SHARED ${SOURCES} )

set_target_properties(engineqtmultimedia PROPERTIES AUTOMOC TRUE)

add_dependencies(engineqtmultimedia yarock)

target_link_libraries(engineqtmultimedia ${QT_LIBRARIES})

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "default install path" FORCE)
endif()

set(CMAKE_INSTALL_LIB "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/yarock"  CACHE PATH "libraries installation path" FORCE)

install(TARGETS engineqtmultimedia DESTINATION ${CMAKE_INSTALL_LIB})

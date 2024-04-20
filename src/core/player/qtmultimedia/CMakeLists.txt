cmake_minimum_required(VERSION 2.8.3)

project(engineqtmultimedia)
# ------------------------------------------------------------------------------
#  build def (QT PLUGIN)
# ------------------------------------------------------------------------------
ADD_DEFINITIONS( -Wall )
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_THREAD)


ADD_DEFINITIONS( -DENABLE_QTMULTIMEDIA )

# ------------------------------------------------------------------------------
#  EXTERNAL LIB
# ------------------------------------------------------------------------------
if(ENABLE_QT5)
  FIND_PACKAGE( Qt5Multimedia REQUIRED)
endif (ENABLE_QT5)    
 

# ------------------------------------------------------------------------------
#  INCLUDES
# ------------------------------------------------------------------------------
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/qtmultimedia
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/mediaitem
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src
)

if (ENABLE_QT5)
    set (QTLIBS ${Qt5Core_LIBRARIES} 
                ${Qt5Multimedia_LIBRARIES})
    
    set (QTINCLUDES ${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Multimedia_INCLUDE_DIRS} )

    add_definitions( ${Qt5Core_DEFINITIONS} 
                     ${Qt5Multimedia_DEFINITIONS} )

    INCLUDE_DIRECTORIES( ${QTINCLUDES} )
    
endif(ENABLE_QT5)

# ------------------------------------------------------------------------------
#  SOURCES
# ------------------------------------------------------------------------------
SET(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/qtmultimedia/engine_qtmultimedia.cpp
)

# ------------------------------------------------------------------------------
#  BUILDING
# ------------------------------------------------------------------------------
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

ADD_LIBRARY( engineqtmultimedia SHARED ${SOURCES} )

SET_TARGET_PROPERTIES(engineqtmultimedia PROPERTIES AUTOMOC TRUE)

ADD_DEPENDENCIES(engineqtmultimedia yarock)

if(ENABLE_QT5)  
    TARGET_LINK_LIBRARIES(engineqtmultimedia ${QTLIBS})
endif(ENABLE_QT5)

# ------------------------------------------------------------------------------
#  INSTALL
# ------------------------------------------------------------------------------  
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   set (CMAKE_INSTALL_PREFIX /usr CACHE PATH "default install path" FORCE )
endif()

set(CMAKE_INSTALL_LIB "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/yarock"  CACHE PATH "libraries installation path" FORCE)

install(TARGETS engineqtmultimedia DESTINATION ${CMAKE_INSTALL_LIB} )




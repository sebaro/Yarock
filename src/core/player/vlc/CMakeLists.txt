cmake_minimum_required(VERSION 2.6)

project(enginevlc)
# ------------------------------------------------------------------------------
#  build def (QT PLUGIN)
# ------------------------------------------------------------------------------
ADD_DEFINITIONS( -Wall )
ADD_DEFINITIONS( -DQT_PLUGIN )
ADD_DEFINITIONS( -DQT_NO_DEBUG )
ADD_DEFINITIONS( -DQT_SHARED )
ADD_DEFINITIONS( -DQT_THREAD )


ADD_DEFINITIONS( -DENABLE_VLC )

# ------------------------------------------------------------------------------
#  EXTERNAL LIB
# ------------------------------------------------------------------------------
FIND_PACKAGE( LIBVLC REQUIRED )


# ------------------------------------------------------------------------------
#  INCLUDES
# ------------------------------------------------------------------------------
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/vlc
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/mediaitem
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src
)

INCLUDE_DIRECTORIES( ${LIBVLC_INCLUDE_DIR} )

if (ENABLE_QT5)
    set (QTLIBS ${Qt5Core_LIBRARIES})
    
    set (QTINCLUDES ${Qt5Core_INCLUDE_DIRS})

    add_definitions( ${Qt5Core_DEFINITIONS})

else (ENABLE_QT5)
    set (QTLIBS ${QT_QTCORE_LIBRARY})

    set (QTINCLUDES ${QT_INCLUDES})
    
    add_definitions(${QT_DEFINITIONS})    
endif (ENABLE_QT5)

INCLUDE_DIRECTORIES( ${QTINCLUDES} )


# ------------------------------------------------------------------------------
#  configure file
# ------------------------------------------------------------------------------
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/vlc/config.h)
               
# ------------------------------------------------------------------------------
#  SOURCES
# ------------------------------------------------------------------------------
SET(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/vlc/engine_vlc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/vlc/vlc_lib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/vlc/vlc_media.cpp
)


               
# ------------------------------------------------------------------------------
#  BUILDING
# ------------------------------------------------------------------------------
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

ADD_LIBRARY( enginevlc SHARED ${SOURCES} )

SET_TARGET_PROPERTIES(enginevlc PROPERTIES AUTOMOC TRUE)

ADD_DEPENDENCIES(enginevlc yarock)

TARGET_LINK_LIBRARIES(enginevlc ${LIBVLC_LIBRARY} ${QTLIBS})

# ------------------------------------------------------------------------------
#  INSTALL
# ------------------------------------------------------------------------------  
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   set (CMAKE_INSTALL_PREFIX /usr CACHE PATH "default install path" FORCE )
endif()

set(CMAKE_INSTALL_LIB "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/yarock"  CACHE PATH "libraries installation path" FORCE)

install(TARGETS enginevlc DESTINATION ${CMAKE_INSTALL_LIB} )


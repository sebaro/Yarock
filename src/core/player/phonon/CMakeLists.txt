cmake_minimum_required(VERSION 3.16)

project(enginephonon)
# ------------------------------------------------------------------------------
#  build def (QT PLUGIN)
# ------------------------------------------------------------------------------
ADD_DEFINITIONS( -Wall )
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_THREAD)


ADD_DEFINITIONS( -DENABLE_PHONON )

# ------------------------------------------------------------------------------
#  EXTERNAL LIB
# ------------------------------------------------------------------------------
if(ENABLE_QT6)
    FIND_PACKAGE( Phonon4Qt6 REQUIRED )
else(ENABLE_QT6)
    FIND_PACKAGE( Phonon4Qt5 REQUIRED )
endif (ENABLE_QT6)


# ------------------------------------------------------------------------------
#  INCLUDES
# ------------------------------------------------------------------------------
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/phonon
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/mediaitem
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src
)


if (ENABLE_QT6)
    INCLUDE_DIRECTORIES( ${PHONON4QT6_INCLUDE_DIR}/phonon4qt6 )
else (ENABLE_QT6)
    INCLUDE_DIRECTORIES( ${PHONON4QT5_INCLUDE_DIR}/phonon4qt5 )
endif (ENABLE_QT6)


if (ENABLE_QT6)
    set (QTLIBS ${Qt6Core_LIBRARIES})

    set (QTINCLUDES ${Qt6Core_INCLUDE_DIRS})

    add_definitions( ${Qt6Core_DEFINITIONS})

else (ENABLE_QT6)
    set (QTLIBS ${Qt5Core_LIBRARIES})

    set (QTINCLUDES ${Qt5Core_INCLUDE_DIRS})

    add_definitions( ${Qt5Core_DEFINITIONS})
endif (ENABLE_QT6)

INCLUDE_DIRECTORIES( ${QTINCLUDES} )

# ------------------------------------------------------------------------------
#  SOURCES
# ------------------------------------------------------------------------------
SET(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/core/player/phonon/engine_phonon.cpp
)

# ------------------------------------------------------------------------------
#  BUILDING
# ------------------------------------------------------------------------------
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

ADD_LIBRARY( enginephonon SHARED ${SOURCES} )

SET_TARGET_PROPERTIES(enginephonon PROPERTIES AUTOMOC TRUE)

ADD_DEPENDENCIES(enginephonon yarock)

if(ENABLE_QT6)
    TARGET_LINK_LIBRARIES(enginephonon ${PHONON4QT6_LIBRARY} ${QTLIBS})
else(ENABLE_QT6)
    TARGET_LINK_LIBRARIES(enginephonon ${PHONON4QT5_LIBRARY} ${QTLIBS})
endif(ENABLE_QT6)

# ------------------------------------------------------------------------------
#  INSTALL
# ------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   set (CMAKE_INSTALL_PREFIX /usr CACHE PATH "default install path" FORCE )
endif()

set(CMAKE_INSTALL_LIB "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/yarock"  CACHE PATH "libraries installation path" FORCE)

install(TARGETS enginephonon DESTINATION ${CMAKE_INSTALL_LIB} )




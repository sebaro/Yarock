
# Find htmlcxx headers and libraries.
#
#  HTMLCXX_INCLUDES - where to find htmlcxx/html/tree.h, etc.
#  HTMLCXX_LIBRARIES    - List of libraries when using HTMLCXX.
#  HTMLCXX_FOUND        - True if HTMLCXX found.

IF (HTMLCXX_INCLUDES AND HTMLCXX_LIBRARIES)
    SET(HTMLCXX_FIND_QUIETLY TRUE)
ENDIF (HTMLCXX_INCLUDES AND HTMLCXX_LIBRARIES)

FIND_PACKAGE (PkgConfig QUIET)
IF (PKGCONFIG_FOUND)
    PKG_CHECK_MODULES(PC_HTMLCXX QUIET htmlcxx)
ENDIF (PKGCONFIG_FOUND)

FIND_PATH(HTMLCXX_INCLUDES
    NAMES
        htmlcxx/html/tree.h
    PATHS
        ${PC_HTMLCXX_INCLUDES}
        /usr/local/include
        /usr/include
        $ENV{HTMLCXX}
        $ENV{HTMLCXX}/include
    )

FIND_LIBRARY(HTMLCXX_LIBRARIES
    NAMES
        htmlcxx
    PATHS
        ${PC_HTMLCXX_LIBRARY_DIRS}
        /usr/local/lib
        /usr/lib
        $ENV{HTMLCXX}
        $ENV{HTMLCXX}/lib
    )

IF (HTMLCXX_INCLUDES AND HTMLCXX_LIBRARIES)
    SET(HTMLCXX_FOUND TRUE)
ENDIF (HTMLCXX_INCLUDES AND HTMLCXX_LIBRARIES)

IF (HTMLCXX_FOUND)
    MESSAGE(STATUS "Found Htmlcxx include-dir path: ${HTMLCXX_INCLUDES}")
    MESSAGE(STATUS "Found Htmlcxx library path:${HTMLCXX_LIBRARIES}")
ELSE (HTMLCXX_FOUND)
    MESSAGE(FATAL_ERROR "Could not find Htmlcxx")
ENDIF (HTMLCXX_FOUND)



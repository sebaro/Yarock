
# Find htmlcxx headers and libraries.
#
#  HTMLCXX_INCLUDE_DIRS - where to find htmlcxx/html/tree.h, etc.
#  HTMLCXX_LIBRARIES    - List of libraries when using HTMLCXX.
#  HTMLCXX_FOUND        - True if HTMLCXX found.

IF (HTMLCXX_INCLUDE_DIRS AND HTMLCXX_LIBRARIES)
    SET(HTMLCXX_FIND_QUIETLY TRUE)
ENDIF (HTMLCXX_INCLUDE_DIRS AND HTMLCXX_LIBRARIES)

FIND_PACKAGE (PkgConfig QUIET)
IF (PKGCONFIG_FOUND)
    PKG_CHECK_MODULES(PC_HTMLCXX QUIET htmlcxx)
ENDIF (PKGCONFIG_FOUND)

FIND_PATH(HTMLCXX_INCLUDE_DIRS
    NAMES
        htmlcxx/html/tree.h
    PATHS
        ${PC_HTMLCXX_INCLUDE_DIRS}
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

IF (HTMLCXX_INCLUDE_DIRS AND HTMLCXX_LIBRARIES)
    SET(HTMLCXX_FOUND TRUE)
ENDIF (HTMLCXX_INCLUDE_DIRS AND HTMLCXX_LIBRARIES)

IF (HTMLCXX_FOUND)
    MESSAGE(STATUS "  Found Htmlcxx include-dir path: ${HTMLCXX_INCLUDE_DIRS}")
    MESSAGE(STATUS "  Found Htmlcxx library path:${HTMLCXX_LIBRARIES}")
ELSE (HTMLCXX_FOUND)
    MESSAGE(FATAL_ERROR "Could not find Htmlcxx")
ENDIF (HTMLCXX_FOUND)



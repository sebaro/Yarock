
#Find includes header (client.h) from libmpv-dev
FIND_PATH(LIBMPV_INCLUDE_DIR mpv/client.h
    "/usr/include"
    "/usr/include/mpv"
    "/usr/local/include"
    "/usr/local/include/mpv"
)

FIND_PATH(LIBMPV_INCLUDE_DIR client.h)

#Find library libmpv
FIND_LIBRARY(LIBMPV_LIBRARY NAMES mpv libmpv PATHS
    "$ENV{LIBVLC_LIBRARY_PATH}"
    "$ENV{LIB_DIR}/lib"
    #mingw
    c:/msys/local/lib
    NO_DEFAULT_PATH
)

FIND_LIBRARY(LIBMPV_LIBRARY NAMES mpv libmpv)


IF (LIBMPV_INCLUDE_DIR AND LIBMPV_LIBRARY)
    SET(LIBMPV_FOUND TRUE)
ENDIF (LIBMPV_INCLUDE_DIR AND LIBMPV_LIBRARY)


IF (LIBMPV_FOUND)
    MESSAGE(STATUS "  Found LibMPV include path: ${LIBMPV_INCLUDE_DIR}")
    MESSAGE(STATUS "  Found LibMPV library path: ${LIBMPV_LIBRARY}")
ELSE (LIBMPV_FOUND)
    MESSAGE(FATAL_ERROR "Could not find LibMPV")
ENDIF (LIBMPV_FOUND)


set(LIBMPV_VERSION ${PC_LIBMPV_VERSION})
if (NOT LIBMPV_VERSION)
    file(READ "${LIBMPV_INCLUDE_DIR}/mpv/client.h" _client_h)

    string(REGEX MATCH "MPV_MAKE_VERSION[ ]*\\([ ]*([0-9]+)[ ]*,[ ]*([0-9]+)[ ]*\\)" _dummy "${_client_h}")
    set(_version_major "${CMAKE_MATCH_1}")
    set(_version_minor "${CMAKE_MATCH_2}")

    set(LIBMPV_VERSION "${_version_major}.${_version_minor}")
endif (NOT LIBMPV_VERSION)

message(STATUS "  Found LibMPV version: ${LIBMPV_VERSION}")

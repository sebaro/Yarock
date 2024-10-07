# Find Phonon
#
# This module defines
#  PHONON4QT6_FOUND       - whether the Phonon library was found
#  PHONON4QT6_INCLUDE_DIR - the include path of the Phonon library
#  PHONON4QT6_LIBRARY     - the Phonon library
#
# Copyright (c) 2010, Pino Toscano, <toscano.pino@tiscali.it>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (PHONON4QT6_INCLUDE_DIR AND PHONON4QT6_LIBRARY)

  # in cache already
  set(PHONON4QT6_FOUND TRUE)

else (PHONON4QT6_INCLUDE_DIR AND PHONON4QT6_LIBRARY)
  if (NOT WIN32)
    find_package(PkgConfig)
    pkg_check_modules(PC_PHONON4QT6 phonon4qt6)
  endif(NOT WIN32)

  find_path(PHONON4QT6_INCLUDE_DIR phonon4qt6/phonon/phononnamespace.h
    HINTS
    ${PC_PHONON4QT6_INCLUDE_DIRS}
  )

  find_library(PHONON4QT6_LIBRARY NAMES phonon4qt6
    HINTS
    ${PC_PHONON4QT6_LIBRARY_DIRS}
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Phonon4Qt6 DEFAULT_MSG PHONON4QT6_LIBRARY PHONON4QT6_INCLUDE_DIR)

  mark_as_advanced(PHONON4QT6_INCLUDE_DIR PHONON4QT6_LIBRARY)

endif (PHONON4QT6_INCLUDE_DIR AND PHONON4QT6_LIBRARY)

IF (PHONON4QT6_FOUND)
    MESSAGE(STATUS "  Found LibPhonon4qt6 include path: ${PHONON4QT6_INCLUDE_DIR}")
    MESSAGE(STATUS "  Found LibPhonon4qt6 library path: ${PHONON4QT6_LIBRARY}")
ELSE (PHONON4QT6_FOUND)
    MESSAGE(FATAL_ERROR "  Could not find LibPhonon4qt6")
ENDIF (PHONON4QT6_FOUND)

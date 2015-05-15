cmake_minimum_required(VERSION 2.6)

if (ENABLE_QT5)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
endif (ENABLE_QT5)

set(SINGLEAPP-SOURCES
    qtlocalpeer.cpp
    qtlockedfile.cpp
    qtsingleapplication.cpp
    qtsinglecoreapplication.cpp
)

set(SINGLEAPP-MOC-HEADERS
    qtlockedfile.h
    qtlocalpeer.h
    qtsingleapplication.h
    qtsinglecoreapplication.h
)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QTINCLUDES} )

if(WIN32)
  set(SINGLEAPP-SOURCES ${SINGLEAPP-SOURCES} qtlockedfile_win.cpp)
elseif(WIN32)
  set(SINGLEAPP-SOURCES ${SINGLEAPP-SOURCES} qtlockedfile_unix.cpp)
endif(WIN32)

if (ENABLE_QT5)
  qt5_wrap_cpp(SINGLEAPP-SOURCES-MOC ${SINGLEAPP-MOC-HEADERS})
else (ENABLE_QT5)
  qt4_wrap_cpp(SINGLEAPP-SOURCES-MOC ${SINGLEAPP-MOC-HEADERS})
endif (ENABLE_QT5)

ADD_LIBRARY(qtsingleapplication STATIC
    ${SINGLEAPP-SOURCES}
    ${SINGLEAPP-SOURCES-MOC}
)

if (ENABLE_QT5)
   TARGET_LINK_LIBRARIES( qtsingleapplication ${Qt5Network_LIBRARIES}  ${Qt5Core_LIBRARIES}  )
endif (ENABLE_QT5)
   

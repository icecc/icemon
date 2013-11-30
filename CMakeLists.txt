project(icemon)

cmake_minimum_required(VERSION 2.8.5)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(GNUInstallDirs)
include(KDE4Macros-Icecream)
include(CheckIncludeFileCXX)
include(MacroLogFeature)

# version info
set(ICEMON_VERSION_MAJOR "2")
set(ICEMON_VERSION_MINOR "9")
set(ICEMON_VERSION_PATCH "90")
set(ICEMON_VERSION "${ICEMON_VERSION_MAJOR}.${ICEMON_VERSION_MINOR}.${ICEMON_VERSION_PATCH}")
set(ICEMON_VERSION_STRING "${ICEMON_VERSION}")

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  find_package(Git)
  if(GIT_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE _git_revision
    )
    string(REGEX REPLACE "\n" "" _git_revision "${_git_revision}")
    set(ICEMON_VERSION_STRING "${ICEMON_VERSION_STRING} (revision: ${_git_revision})")
  endif()
endif()

find_package(Qt4 REQUIRED)
find_package(Icecream REQUIRED)
find_package(Docbook2X QUIET)

macro_log_feature(
  DOCBOOK_TO_MAN_EXECUTABLE
  "docbook2X"
  "docbook2X converts DocBook documents into the traditional Unix man page format"
  "http://docbook2x.sourceforge.net/"
  FALSE
  ""
  "Required for man-page generation"
)

add_definitions(
  ${QT_DEFINITIONS}
)

include_directories(
    ${LIBICECREAM_INCLUDE_DIR}
    ${QT_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR} # config-icemon.h
)

check_include_file_cxx(icecc/logging.h ICECC_HAVE_LOGGING_H)

set(
  INSTALL_TARGETS_DEFAULT_ARGS
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Devel
)

set(XDG_APPS_INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/applications)
set(ICON_INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/icons)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config-icemon.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config-icemon.h
)

add_subdirectory(src)
add_subdirectory(doc)

macro_display_feature_log()

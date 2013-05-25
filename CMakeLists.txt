project(icemon)

cmake_minimum_required(VERSION 2.8.5)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(GNUInstallDirs)
include(KDE4Macros-Icecream)

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

add_definitions(
  ${QT_DEFINITIONS}
)

include_directories(
    ${LIBICECREAM_INCLUDE_DIR}
    ${QT_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR} # config-icemon.h
)

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

# It's not possible to install the docbook right now (Qt-only project)
#add_subdirectory(doc)

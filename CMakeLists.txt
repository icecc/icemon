project(icemon)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

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

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
set(LIB_INSTALL_DIR "lib${LIB_SUFFIX}")
set(PLUGIN_INSTALL_DIR "${LIB_INSTALL_DIR}/qt4/plugins")

set(
  INSTALL_TARGETS_DEFAULT_ARGS
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIB_INSTALL_DIR} COMPONENT Devel
)

set(XDG_APPS_INSTALL_DIR share/applications)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config-icemon.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config-icemon.h
)

add_subdirectory(src)

# It's not possible to install the docbook right now (Qt-only project)
#add_subdirectory(doc)

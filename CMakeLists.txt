project(icemon)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

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

add_subdirectory(src)

# It's not possible to install the docbook right now (Qt-only project)
#add_subdirectory(doc)

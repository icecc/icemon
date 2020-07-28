cmake_minimum_required(VERSION 3.1.0)
project(icemon VERSION 3.3)

find_package(ECM REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(GNUInstallDirs)
include(CheckIncludeFileCXX)
include(FeatureSummary)
include(ECMInstallIcons)

set(ICEMON_VERSION_STRING "${PROJECT_VERSION}")

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

set(QT_MIN_VERSION "5.4.0") # Because of QByteArray::fromStdString
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets)
find_package(Icecream)
set_package_properties(Icecream PROPERTIES
  DESCRIPTION "Package providing API for accessing icecc information. Provides 'icecc/comm.h' header"
  URL "http://en.opensuse.org/Icecream"
  TYPE REQUIRED
)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  check_cxx_compiler_flag(-Wunused-but-set-variable HAVE_GCC_UNUSED_BUT_SET)
  check_cxx_compiler_flag(-Wlogical-op HAVE_GCC_LOGICAL_OP)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wall -Wextra -Woverloaded-virtual -Winit-self -Wmissing-include-dirs -Wunused -Wno-div-by-zero -Wundef -Wpointer-arith -Wcast-qual -Wcast-align -Wmissing-noreturn -Werror=return-type -pedantic")
  if(HAVE_GCC_UNUSED_BUT_SET)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-but-set-variable")
  endif()
  if(HAVE_GCC_LOGICAL_OP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op")
  endif()
endif()

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050400
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_STRICT_ITERATORS
    -DQT_USE_FAST_CONCATENATION
    -DQT_USE_FAST_OPERATOR_PLUS
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR} # config-icemon.h
)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if (Icecream_FOUND)
    check_include_file_cxx(icecc/logging.h ICECC_HAVE_LOGGING_H)

    # Check whether icecc was compiled against old CXXABI
    # Work-around for: https://github.com/icecc/icemon/issues/24
    # See: https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_dual_abi.html
    set(USE_OLDCXXABI_DEFINE -D_GLIBCXX_USE_CXX11_ABI=0)
    set(CMAKE_REQUIRED_LIBRARIES Icecream)
    set(CMAKE_REQUIRED_DEFINITIONS ${USE_OLDCXXABI_DEFINE})
    check_cxx_source_compiles("
    #include <icecc/comm.h>

    int main() { DiscoverSched sched(\"foo\"); }
    " ICECC_TEST_USE_OLDABI)

    if (ICECC_TEST_USE_OLDABI)
        message(STATUS "Tests indicated icecc was linked against old GCC ABI, adding ${USE_OLDCXXABI_DEFINE} define")
        add_definitions(${USE_OLDCXXABI_DEFINE})
    endif()
endif()

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

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PROJECT_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | xz -T0 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.xz
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

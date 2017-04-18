# Try to find the Icecream library
#
# Once done this defines:
#  Icecream_FOUND - system has libicecream
#  Icecream_INCLUDE_DIR - the libicecream include directory
#  Icecream_LIBRARIES - Link these to use libicecream
#  Icecream_VERSION - the libicecream version, if available

#=============================================================================
# Copyright 2015 Kevin Funk <kfunk@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

if (Icecream_INCLUDE_DIR AND Icecream_LIBRARY AND Icecream_LIB_EXTRA)
  # in cache already
  set(Icecream_FOUND TRUE)
else ()
  set(extraLibs)

  if(NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PC_ICECC icecc)
    # The icecream lib may optionally need linking to -lcap-ng, so dig it out
    # of pkg-config data.
    # Somewhat hackish, but I can't find a simpler way to do this with CMake.
    foreach(lib ${PC_ICECC_STATIC_LIBRARIES})
      if(NOT ${lib} STREQUAL "icecc")
        list(APPEND extraLibs "-l${lib}")
      endif()
    endforeach()
    set(Icecream_VERSION "${PC_ICECC_VERSION}")
  endif()

  list(APPEND extraLibs ${CMAKE_DL_LIBS})

  set(Icecream_LIB_EXTRA ${extraLibs} CACHE INTERNAL "The library dependencies for libicecream")

  find_path(Icecream_INCLUDE_DIR icecc/comm.h
    HINTS
    ${PC_ICECC_INCLUDEDIR}
    ${PC_ICECC_INCLUDE_DIRS}
    /opt/icecream/include
  )

  find_library(Icecream_LIBRARY NAMES icecc
    HINTS
    ${PC_ICECC_LIBDIR}
    ${PC_ICECC_LIBRARY_DIRS}
    /opt/icecream/lib
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Icecream
    REQUIRED_VARS Icecream_LIBRARY Icecream_INCLUDE_DIR
    VERSION_VAR Icecream_VERSION
  )

  mark_as_advanced(
    Icecream_INCLUDE_DIR Icecream_LIBRARIES Icecream_LIB_EXTRA
  )
  set(Icecream_FOUND ${ICECREAM_FOUND}) # CMake bug: Does not set Icecream_FOUND
endif()

if (Icecream_FOUND)
  set(Icecream_LIBRARIES ${Icecream_LIBRARY} ${Icecream_LIB_EXTRA})

  add_library(Icecream UNKNOWN IMPORTED)
  set_target_properties(Icecream PROPERTIES
      IMPORTED_LOCATION ${Icecream_LIBRARY}
      INTERFACE_INCLUDE_DIRECTORIES ${Icecream_INCLUDE_DIR}
      INTERFACE_LINK_LIBRARIES "${Icecream_LIB_EXTRA}"
  )
endif()

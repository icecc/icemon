# - Try to find the icecream library
# Once done this defines
#
#  LIBICECREAM_FOUND - system has libicecream
#  LIBICECREAM_INCLUDE_DIR - the libicecream include directory
#  LIBICECREAM_LIBRARIES - Link these to use libicecream

if (LIBICECREAM_INCLUDE_DIR AND LIBICECREAM_LIBRARIES)

  # in cache already
  set(LIBICECREAM_FOUND TRUE)

else (LIBICECREAM_INCLUDE_DIR AND LIBICECREAM_LIBRARIES)

  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  INCLUDE(UsePkgConfig)

  PKGCONFIG(icecream _icecreamIncDir _icecreamLinkDir _icecreamLinkFlags _icecreamCflags)

  FIND_PATH(LIBICECREAM_INCLUDE_DIR icecream/comm.h
    /usr/include
    /usr/local/include
    /opt/icecream/include
  )

  FIND_LIBRARY(LIBICECREAM_LIBRARY NAMES icecream
    PATHS
    ${_icecreamLinkDir}
    /usr/lib
    /usr/local/lib
    /opt/icecream/lib
  )

  set( LIBICECREAM_LIBRARIES ${LIBICECREAM_LIBRARY} CACHE INTERNAL "The libraries for libicecream" )

  if (LIBICECREAM_INCLUDE_DIR AND LIBICECREAM_LIBRARIES)
     set( LIBICECREAM_FOUND TRUE)
  endif (LIBICECREAM_INCLUDE_DIR AND LIBICECREAM_LIBRARIES)

  if (LIBICECREAM_FOUND)
    if (NOT icecream_FIND_QUIETLY)
      message(STATUS "Found LIBICECREAM: ${LIBICECREAM_LIBRARIES}")
    endif (NOT icecream_FIND_QUIETLY)
  else (LIBICECREAM_FOUND)
    if (icecream_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find LIBICECREAM")
    endif (icecream_FIND_REQUIRED)
  endif (LIBICECREAM_FOUND)

  MARK_AS_ADVANCED(
     LIBICECREAM_INCLUDE_DIR LIBICECREAM_LIBRARIES 
  )

endif (LIBICECREAM_INCLUDE_DIR AND LIBICECREAM_LIBRARIES)

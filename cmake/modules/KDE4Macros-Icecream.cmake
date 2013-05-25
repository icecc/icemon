# for documentation look at FindKDE4Internal.cmake

# this file contains the following macros (or functions):
# KDE4_UPDATE_ICONCACHE
# KDE4_INSTALL_ICONS

# Copyright (c) 2006-2009 Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro (KDE4_UPDATE_ICONCACHE)
    # Update mtime of hicolor icon theme dir.
    # We don't always have touch command (e.g. on Windows), so instead create
    #  and delete a temporary file in the theme dir.
   install(CODE "
    set(DESTDIR_VALUE \"\$ENV{DESTDIR}\")
    if (NOT DESTDIR_VALUE)
        file(WRITE \"${ICON_INSTALL_DIR}/hicolor/temp.txt\" \"update\")
        file(REMOVE \"${ICON_INSTALL_DIR}/hicolor/temp.txt\")
    endif (NOT DESTDIR_VALUE)
    ")
endmacro (KDE4_UPDATE_ICONCACHE)

# a "map" of short type names to the directories
# unknown names should give empty results
# KDE 3 compatibility
set(_KDE4_ICON_GROUP_mime       "mimetypes")
set(_KDE4_ICON_GROUP_filesys    "places")
set(_KDE4_ICON_GROUP_device     "devices")
set(_KDE4_ICON_GROUP_app        "apps")
set(_KDE4_ICON_GROUP_action     "actions")
# KDE 4 / icon naming specification compatibility
set(_KDE4_ICON_GROUP_mimetypes  "mimetypes")
set(_KDE4_ICON_GROUP_places     "places")
set(_KDE4_ICON_GROUP_devices    "devices")
set(_KDE4_ICON_GROUP_apps       "apps")
set(_KDE4_ICON_GROUP_actions    "actions")
set(_KDE4_ICON_GROUP_categories "categories")
set(_KDE4_ICON_GROUP_status     "status")
set(_KDE4_ICON_GROUP_emblems    "emblems")
set(_KDE4_ICON_GROUP_emotes     "emotes")
set(_KDE4_ICON_GROUP_animations "animations")
set(_KDE4_ICON_GROUP_intl       "intl")

# a "map" of short theme names to the theme directory
set(_KDE4_ICON_THEME_ox "oxygen")
set(_KDE4_ICON_THEME_cr "crystalsvg")
set(_KDE4_ICON_THEME_lo "locolor")
set(_KDE4_ICON_THEME_hi "hicolor")


# only used internally by KDE4_INSTALL_ICONS
macro (_KDE4_ADD_ICON_INSTALL_RULE _install_SCRIPT _install_PATH _group _orig_NAME _install_NAME _l10n_SUBDIR)

   # if the string doesn't match the pattern, the result is the full string, so all three have the same content
   if (NOT ${_group} STREQUAL ${_install_NAME} )
      set(_icon_GROUP  ${_KDE4_ICON_GROUP_${_group}})
      if(NOT _icon_GROUP)
         set(_icon_GROUP "actions")
      endif(NOT _icon_GROUP)
#      message(STATUS "icon: ${_current_ICON} size: ${_size} group: ${_group} name: ${_name} l10n: ${_l10n_SUBDIR}")
      install(FILES ${_orig_NAME} DESTINATION ${_install_PATH}/${_icon_GROUP}/${_l10n_SUBDIR}/ RENAME ${_install_NAME} )
   endif (NOT ${_group} STREQUAL ${_install_NAME} )

endmacro (_KDE4_ADD_ICON_INSTALL_RULE)


macro (KDE4_INSTALL_ICONS _defaultpath )

   # the l10n-subdir if language given as second argument (localized icon)
   set(_lang ${ARGV1})
   if(_lang)
      set(_l10n_SUBDIR l10n/${_lang})
   else(_lang)
      set(_l10n_SUBDIR ".")
   endif(_lang)

   # first the png icons
   file(GLOB _icons *.png)
   foreach (_current_ICON ${_icons} )
      # since CMake 2.6 regex matches are stored in special variables CMAKE_MATCH_x, if it didn't match, they are empty
      string(REGEX MATCH "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" _dummy  "${_current_ICON}")
      set(_type  "${CMAKE_MATCH_1}")
      set(_size  "${CMAKE_MATCH_2}")
      set(_group "${CMAKE_MATCH_3}")
      set(_name  "${CMAKE_MATCH_4}")

      set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
      if( _theme_GROUP)
         _KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                    ${_defaultpath}/${_theme_GROUP}/${_size}x${_size}
                    ${_group} ${_current_ICON} ${_name} ${_l10n_SUBDIR})
      endif( _theme_GROUP)
   endforeach (_current_ICON)

   # mng icons
   file(GLOB _icons *.mng)
   foreach (_current_ICON ${_icons} )
      # since CMake 2.6 regex matches are stored in special variables CMAKE_MATCH_x, if it didn't match, they are empty
      string(REGEX MATCH "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.mng)$" _dummy  "${_current_ICON}")
      set(_type  "${CMAKE_MATCH_1}")
      set(_size  "${CMAKE_MATCH_2}")
      set(_group "${CMAKE_MATCH_3}")
      set(_name  "${CMAKE_MATCH_4}")

      set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
      if( _theme_GROUP)
         _KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                ${_defaultpath}/${_theme_GROUP}/${_size}x${_size}
                ${_group} ${_current_ICON} ${_name} ${_l10n_SUBDIR})
      endif( _theme_GROUP)
   endforeach (_current_ICON)

   # and now the svg icons
   file(GLOB _icons *.svgz)
   foreach (_current_ICON ${_icons} )
      # since CMake 2.6 regex matches are stored in special variables CMAKE_MATCH_x, if it didn't match, they are empty
      string(REGEX MATCH "^.*/([a-zA-Z]+)sc\\-([a-z]+)\\-(.+\\.svgz)$" _dummy "${_current_ICON}")
      set(_type  "${CMAKE_MATCH_1}")
      set(_group "${CMAKE_MATCH_2}")
      set(_name  "${CMAKE_MATCH_3}")

      set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
      if( _theme_GROUP)
          _KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                            ${_defaultpath}/${_theme_GROUP}/scalable
                            ${_group} ${_current_ICON} ${_name} ${_l10n_SUBDIR})
      endif( _theme_GROUP)
   endforeach (_current_ICON)

   kde4_update_iconcache()

endmacro (KDE4_INSTALL_ICONS)

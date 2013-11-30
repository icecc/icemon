# Attempt to find docbook-to-man binary from docbook2x package
#
# This module defines:
# - DOCBOOK_TO_MAN_EXECUTABLE, path to docbook2x-man binary
#
# Note that the binary docbook-to-man in debian systems is a different application
# than in other distributions.
#
# Debian systems
# * docbook-to-man comes from the package docbook-to-man
# * docbook2man comes from the package docbook-utils
# * docbook2x-man comes from the package docbook2x
# Suse
# * docbook-to-man comes from the package docbook2x
# * docbook2man comes from the package docbook-utils-minimal
#
# We actually want the binary from docbook2x, which supports XML

#=============================================================================
# Copyright 2013 Kevin Funk <kfunk@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

find_program(_docbook_to_man_executable
  NAMES docbook2x-man docbook-to-man
)

# make sure we've found the correct binary which supports XML input
# e.g. on Ubuntu docbook-to-man just eats SGML
if (_docbook_to_man_executable)
    execute_process(
        COMMAND ${_docbook_to_man_executable} --version
        OUTPUT_VARIABLE _output
        ERROR_QUIET
    )
    if("${_output}" MATCHES "docbook2x")
        set(DOCBOOK_TO_MAN_EXECUTABLE ${_docbook_to_man_executable})
    else()
        if (NOT Docbook2X_FIND_QUIETLY)
            message(STATUS "Could not find either docbook2x-man or docbook-to-man binary from docbook2x package")
        endif()
    endif()
endif()

if (DOCBOOK_TO_MAN_EXECUTABLE)
    macro(install_docbook_man_page name section)
        set(inputfn "man-${name}.${section}.xml")
        set(input "${CMAKE_CURRENT_SOURCE_DIR}/${inputfn}")
        set(outputfn "${name}.${section}")
        set(output "${CMAKE_CURRENT_BINARY_DIR}/${outputfn}")
        set(target "manpage-${outputfn}")

        add_custom_command(
            OUTPUT ${output}
            COMMAND ${DOCBOOK_TO_MAN_EXECUTABLE} ${input}
            DEPENDS ${input}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        add_custom_target(${target} ALL
            DEPENDS "${output}"
        )
        install(
            FILES ${output}
            DESTINATION ${CMAKE_INSTALL_MANDIR}/man${section}
        )
    endmacro()
endif()

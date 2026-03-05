# It's not possible to install the docbook right now (Qt-only project)
#add_subdirectory(icemon)

find_program(pandoc_EXECUTABLE pandoc)
find_package_handle_standard_args(pandoc
    REQUIRED_VARS
        pandoc_EXECUTABLE
)
mark_as_advanced(pandoc_EXECUTABLE)
set_package_properties(pandoc PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "A universal document converter. Used for generating manpages."
    URL "https://pandoc.org/"
)

if (pandoc_FOUND)
    add_custom_target(manpage)
    add_custom_command(
        TARGET manpage
        COMMAND ${pandoc_EXECUTABLE} -s -t man ${CMAKE_CURRENT_SOURCE_DIR}/icemon.md
            -o ${CMAKE_CURRENT_BINARY_DIR}/icemon.1
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/icemon.md
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/icemon.1)
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/icemon.1
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()

# It's not possible to install the docbook right now (Qt-only project)
#add_subdirectory(icemon)

find_program(ASCIIDOC asciidoc)
find_program(A2X a2x)

if(NOT ASCIIDOC OR NOT A2X)
    message(WARNING "Could not find asciidoc or a2x, manual page will not be generated.")
else()
    add_custom_target(manpage ALL)
    add_custom_command(
        TARGET manpage
        COMMAND ${ASCIIDOC} -a revnumber=${PROJECT_VERSION} -d manpage -b docbook
            -o ${CMAKE_CURRENT_BINARY_DIR}/icemon.xml ${CMAKE_CURRENT_SOURCE_DIR}/icemon.adoc
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/icemon.adoc
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/icemon.xml)
    add_custom_command(
        TARGET manpage
        COMMAND ${A2X} --doctype manpage --format manpage
             ${CMAKE_CURRENT_BINARY_DIR}/icemon.xml
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/icemon.xml
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/icemon.1)
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/icemon.1
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()

# It's not possible to install the docbook right now (Qt-only project)
#add_subdirectory(icemon)

macro(install_docbook_man_page name section)
  set(inputfn "man-${name}.${section}.xml")
  set(input "${CMAKE_CURRENT_SOURCE_DIR}/${inputfn}")
  set(outputfn "${name}.${section}")
  set(output "${CMAKE_CURRENT_BINARY_DIR}/${outputfn}")
  set(target "manpage-${outputfn}")

  add_custom_command(
    OUTPUT ${output}
    COMMAND ${DOCBOOK2X} ${input}
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

if(DOCBOOK2X)
endif()

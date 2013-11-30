# It's not possible to install the docbook right now (Qt-only project)
#add_subdirectory(icemon)

if(DOCBOOK_TO_MAN_EXECUTABLE)
  install_docbook_man_page(icemon 1)
endif()

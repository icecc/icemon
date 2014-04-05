add_subdirectory(images)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(icemon_SRCS
  fakemonitor.cc
  hostinfo.cc
  icecreammonitor.cc
  job.cc
  main.cc
  mainwindow.cc
  monitor.cc
  statusview.cc
  utils.cc

  views/detailedhostview.cc
  views/flowtableview.cc
  views/ganttstatusview.cc
  views/hostlistmodel.cc
  views/hostlistview.cc
  views/joblistmodel.cc
  views/joblistview.cc
  views/listview.cc
  #views/poolview.cc
  views/starview.cc
  views/summaryview.cc
)

qt4_add_resources(resources_SRCS icemon.qrc)
qt4_automoc(${icemon_SRCS})

add_executable(icemon ${icemon_SRCS} ${resources_SRCS})

target_link_libraries(icemon
    ${LIBICECREAM_LIBRARIES}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${CMAKE_DL_LIBS}
)

########### install files ###############

install(TARGETS icemon ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES icemon.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

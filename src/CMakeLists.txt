add_subdirectory(images)

set(icemon_SRCS
  fakemonitor.cc
  hostinfo.cc
  icecreammonitor.cc
  job.cc
  main.cc
  mainwindow.cc
  monitor.cc
  statusview.cc
  statusviewfactory.cc
  utils.cc

  models/hostlistmodel.cc
  models/joblistmodel.cc

  views/detailedhostview.cc
  views/flowtableview.cc
  views/ganttstatusview.cc
  views/hostlistview.cc
  views/joblistview.cc
  views/listview.cc
  views/starview.cc
  views/summaryview.cc
)

qt_add_resources(resources_SRCS icemon.qrc)
add_executable(icemon ${icemon_SRCS} ${resources_SRCS})
target_link_libraries(icemon
    Icecream
    Qt6::Widgets
)

install(TARGETS icemon ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES icemon.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

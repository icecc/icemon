find_package(Icecream REQUIRED)

add_definitions(${QT_DEFINITIONS})

include_directories(
    ${LIBICECREAM_INCLUDE_DIR}
    ${QT_INCLUDES}

    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(icemon_SRCS
  fakemonitor.cc
  job.cc
  mainwindow.cc
  monitor.cc
  icecreammonitor.cc
  hostlistmodel.cc
  hostinfo.cc
  statusview.cc
  detailedhostview.cc
  flowtableview.cc
  hostlistview.cc
  joblistmodel.cc
  joblistview.cc
  listview.cc
  ganttstatusview.cc
  summaryview.cc
  starview.cc
  poolview.cc

  main.cc
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

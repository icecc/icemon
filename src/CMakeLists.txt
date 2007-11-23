project(icemon)

find_package(Icecream REQUIRED)

include_directories (${LIBICECREAM_INCLUDE_DIR} ${KDE4_INCLUDES})

set(icemon_SRCS
  job.cc
  monitor.cc
  mon-kde.cc
  hostinfo.cc
  statusview.cc
  detailedhostview.cc
  hostlistview.cc
  joblistview.cc
  listview.cc
  ganttstatusview.cc
  summaryview.cc
  starview.cc
  poolview.cc
)

kde4_add_executable(icemon ${icemon_SRCS})

target_link_libraries(icemon ${KDE4_KDEUI_LIBRARY} ${LIBICECREAM_LIBRARIES} ${QT_QT3SUPPORT_LIBRARY})

########### install files ###############

install(TARGETS icemon DESTINATION ${BIN_INSTALL_DIR})

install( FILES icemon.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES icemonui.rc DESTINATION ${DATA_INSTALL_DIR}/icemon )

kde4_install_icons( ${ICON_INSTALL_DIR}   )


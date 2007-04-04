project(icemon)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
find_package(Icecream REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)
link_directories (${KDE4_LIB_DIR})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${LIBICECREAM_INCLUDE_DIR})


set(libicemon_SRCS
  job.cc
  monitor.cc
  hostinfo.cc
  statusview.cc
  detailedhostview.cc
  hostlistview.cc
  hostview.cc
  joblistview.cc
)

kde4_automoc(${libicemon_SRCS})

set(icemon_SRCS ${libicemon_SRCS}
    mon-kde.cc
    listview.cc
    ganttstatusview.cc
    summaryview.cc
    starview.cc)

kde4_automoc(${icemon_SRCS})

kde4_add_executable(icemon ${icemon_SRCS})


target_link_libraries(icemon ${KDE4_KDEUI_LIBS} ${LIBICECREAM_LIBRARIES} ${QT_QT3SUPPORT_LIBRARY})

########### install files ###############

install(TARGETS icemon DESTINATION ${BIN_INSTALL_DIR})

install( FILES icemon.desktop DESTINATION ${XDG_APPS_DIR} )
install( FILES icemonui.rc DESTINATION ${DATA_INSTALL_DIR}/icemon )

kde4_install_icons( ${ICON_INSTALL_DIR}   )

########### next target #################

#set( icemonapplet_PART_SRCS ${icemon_SRCS}
#     icemonapplet.cpp )

#kde4_automoc(${icemonapplet_PART_SRCS})

#kde4_add_plugin(icemonapplet ${icemonapplet_PART_SRCS})

#

#target_link_libraries(icemonapplet  ${KDE4_KDEUI_LIBS} ${LIBICECREAM_LIBRARIES} )


########### install files ###############

#install_targets(${PLUGIN_INSTALL_DIR} icemonapplet )

#install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES icemonapplet.desktop )



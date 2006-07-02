project(icemon)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
find_package(Icecream REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
link_directories (${KDE4_LIB_DIR})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${LIBICECREAM_INCLUDE_DIR})


set(libicemon_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/job.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/monitor.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/hostinfo.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/statusview.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/detailedhostview.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/hostlistview.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/hostview.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/joblistview.cc
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

target_link_libraries(icemon  ${KDE4_KDEUI_LIBS} ${LIBICECREAM_LIBRARIES} )

########### install files ###############

install(TARGETS icemon DESTINATION bin)

install_files( ${XDG_APPS_DIR} FILES icemon.desktop )
install_files( ${DATA_INSTALL_DIR}/icemon FILES icemonui.rc )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

########### next target #################

#set( icemonapplet_PART_SRCS ${icemon_SRCS}
#     icemonapplet.cpp )

#kde4_automoc(${icemonapplet_PART_SRCS})

#kde4_add_plugin(icemonapplet ${icemonapplet_PART_SRCS})

#kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} icemonapplet )

#target_link_libraries(icemonapplet  ${KDE4_KDEUI_LIBS} ${LIBICECREAM_LIBRARIES} )


########### install files ###############

#install_targets(${PLUGIN_INSTALL_DIR} icemonapplet )

#install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES icemonapplet.desktop )




INCLUDES = -I$(srcdir)/.. $(all_includes)

bin_PROGRAMS = icemon
icemon_SOURCES = mon-kde.cc statusview.cc listview.cc ganttstatusview.cc \
                 summaryview.cc starview.cc job.cc hostinfo.cc hostview.cc \
                 monitor.cc
icemon_LDADD = ../services/libcommon.la -lkdeui
icemon_LDFLAGS = $(all_libraries)

METASOURCES = AUTO

icedata_DATA = icemonui.rc
icedatadir = $(kde_appsdir)/icemon

picsdir = $(kde_appsdir)/icemon/pics
pics_DATA = icemonnode.png

kde_module_LTLIBRARIES = icemon_panelapplet.la

icemon_panelapplet_la_LDFLAGS = -module $(all_libraries)
icemon_panelapplet_la_LIBADD = ../services/libcommon.la -lkdeui
icemon_panelapplet_la_SOURCES = job.cc icemonapplet.cpp monitor.cc hostinfo.cc \
                                statusview.cc hostview.cc 

lnkdir = $(kde_appsdir)/kicker/applets
lnk_DATA = icemonapplet.desktop

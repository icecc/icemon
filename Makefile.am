
INCLUDES = -I$(KDEDIR)/include -I$(QTDIR)/include -I$(srcdir)/..

bin_PROGRAMS = icemon
icemon_SOURCES = mon-kde.cc statusview.cc listview.cc ganttstatusview.cc \
                 summaryview.cc starview.cc job.cc hostinfo.cc hostview.cc \
                 monitor.cc
icemon_LDADD = ../services/libcommon.la -lkdeui
icemon_LDFLAGS = -L$(KDEDIR)/lib -L$(QTDIR)/lib

METASOURCES = AUTO

install-data-local:
	$(mkinstalldirs) $(DESTDIR)${KDEDIR}/share/apps/icemon
	$(mkinstalldirs) $(DESTDIR)${KDEDIR}/share/apps/icemon/pics
	$(INSTALL_DATA) $(srcdir)/icemonui.rc $(DESTDIR)${KDEDIR}/share/apps/icemon
	$(INSTALL_DATA) $(srcdir)/icemonnode.png $(DESTDIR)${KDEDIR}/share/apps/icemon/pics

kde_moduledir = $(KDEDIR)/lib/kde3

kde_module_LTLIBRARIES = icemon_panelapplet.la

icemon_panelapplet_la_LDFLAGS = -module -L$(KDEDIR)/lib -L$(QTDIR)/lib
icemon_panelapplet_la_LIBADD = ../services/libcommon.la -lkdeui
icemon_panelapplet_la_SOURCES = job.cc icemonapplet.cpp monitor.cc hostinfo.cc \
                                statusview.cc hostview.cc 

lnkdir = $(KDEDIR)/share/apps/kicker/applets
lnk_DATA = icemonapplet.desktop

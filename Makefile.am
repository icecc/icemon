
INCLUDES = -I$(KDEDIR)/include -I$(QTDIR)/include -I$(srcdir)/..

bin_PROGRAMS = icemon
icemon_SOURCES = mon-kde.cc statusview.cc listview.cc ganttstatusview.cc \
                 summaryview.cc starview.cc job.cc hostinfo.cc
icemon_LDADD = ../services/libcommon.la -lkdeui
icemon_LDFLAGS = -L$(KDEDIR)/lib -L$(QTDIR)/lib

METASOURCES = AUTO

install-data-local:
	$(mkinstalldirs) $(DESTDIR)${KDEDIR}/share/apps/icemon
	$(mkinstalldirs) $(DESTDIR)${KDEDIR}/share/apps/icemon/pics
	$(INSTALL_DATA) $(srcdir)/icemonui.rc $(DESTDIR)${KDEDIR}/share/apps/icemon
	$(INSTALL_DATA) $(srcdir)/icemonnode.png $(DESTDIR)${KDEDIR}/share/apps/icemon/pics


INCLUDES = -I$(srcdir)/../services $(all_includes)

bin_PROGRAMS = icemon
icemon_SOURCES = mon-kde.cc ganttstatusview.cc
icemon_LDADD = ../services/libcommon.la -lkdeui
icemon_LDFLAGS = $(all_libraries)

mon-kde.o: mon-kde.moc
ganttstatusview.o: ganttstatusview.moc

mon-kde.moc: $(srcdir)/mon-kde.h
	$(QTDIR)/bin/moc -o mon-kde.moc $<

ganttstatusview.moc: $(srcdir)/ganttstatusview.h
	$(QTDIR)/bin/moc -o ganttstatusview.moc $<

install-data-local:
	$(mkinstalldirs) ${KDEDIR}/share/apps/icemon
	$(INSTALL_DATA) icemonui.rc ${KDEDIR}/share/apps/icemon

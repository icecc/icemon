
INCLUDES = -I$(KDEDIR)/include -I$(QTDIR)/include -I$(srcdir)/../services

bin_PROGRAMS = icemon
icemon_SOURCES = mon-kde.cc statusview.cc listview.cc ganttstatusview.cc # summaryview.cc starview.cc
icemon_LDADD = ../services/libcommon.la -lkdeui
icemon_LDFLAGS = -L$(KDEDIR)/lib -L$(QTDIR)/lib

mon-kde.o: mon-kde.moc
summaryview.o: summaryview.moc
ganttstatusview.o: ganttstatusview.moc
listview.o: listview.moc
starview.o: starview.moc

mon-kde.moc: $(srcdir)/mon-kde.h
	$(QTDIR)/bin/moc -o mon-kde.moc $<

summaryview.moc: $(srcdir)/summaryview.h
	$(QTDIR)/bin/moc -o summaryview.moc $<

listview.moc: $(srcdir)/listview.h
	$(QTDIR)/bin/moc -o listview.moc $<

starview.moc: $(srcdir)/starview.h
	$(QTDIR)/bin/moc -o starview.moc $<

ganttstatusview.moc: $(srcdir)/ganttstatusview.h
	$(QTDIR)/bin/moc -o ganttstatusview.moc $<

install-data-local:
	$(mkinstalldirs) $(DESTDIR)${KDEDIR}/share/apps/icemon
	$(mkinstalldirs) $(DESTDIR)${KDEDIR}/share/apps/icemon/pics
	$(INSTALL_DATA) $(srcdir)/icemonui.rc $(DESTDIR)${KDEDIR}/share/apps/icemon
	$(INSTALL_DATA) $(srcdir)/icemonnode.png $(DESTDIR)${KDEDIR}/share/apps/icemon/pics


INCLUDES = -I$(KDEDIR)/include -I$(QTDIR)/include

noinst_PROGRAMS = mon
mon_SOURCES = mon-kde.cc
mon_LDADD = -lkdeui
mon_LDFLAGS = -L$(KDEDIR)/lib -L$(QTDIR)/lib

mon-kde.o: mon-kde.moc

mon-kde.moc: mon-kde.h
	$(QTDIR)/bin/moc -o mon-kde.moc $<


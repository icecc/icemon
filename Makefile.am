
INCLUDES = -I$(KDEDIR)/include -I$(QTDIR)/include -I$(srcdir)/../services

bin_PROGRAMS = icemon
icemon_SOURCES = mon-kde.cc
icemon_LDADD = ../services/libcommon.la -lkdeui
icemon_LDFLAGS = -L$(KDEDIR)/lib -L$(QTDIR)/lib

mon-kde.o: mon-kde.moc

mon-kde.moc: mon-kde.h
	$(QTDIR)/bin/moc -o mon-kde.moc $<


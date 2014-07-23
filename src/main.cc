/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (c) 2014 Kevin Funk <kfunk@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <QApplication>
#include <QTextStream>

#include "mainwindow.h"
#include "version.h"

#include <stdio.h>

void printHelp()
{
    QTextStream out(stdout);
    out << "Usage: " << Icemon::Version::appShortName << " [options]" << endl;
    out << endl;
    out << Icemon::Version::description << endl;
    out << endl;
    out << "Options:" << endl;
    out << "\t-h, --help          \tShow this help" << endl;
    out << "\t-n, --netname <name>\tIcecream network name" << endl;
}

int main( int argc, char **argv )
{
    QByteArray netName;
    bool enableTestMode = false;
    for (int i = 1; i < argc; ++i ) {
        if (qstrcmp(argv[i], "--help") == 0 || qstrcmp(argv[i], "-h")  == 0) {
            printHelp();
            return 0;
        }
        if (qstrcmp(argv[i], "--netname") == 0 || qstrcmp(argv[i], "-n")  == 0) {
            if (i+1 < argc)
                netName = argv[++i];
        }
        if (qstrcmp(argv[i], "--testmode") == 0) {
            enableTestMode = true;
        }
    }

    QApplication app(argc, argv);
    QApplication::setOrganizationDomain("kde.org");
    QApplication::setApplicationName(Icemon::Version::appShortName);
    QApplication::setApplicationVersion(Icemon::Version::version);

    MainWindow mainWindow;
    if (!netName.isEmpty()) {
        mainWindow.setCurrentNet(netName);
    }
    if (enableTestMode) {
        mainWindow.setTestModeEnabled(true);
    }
    mainWindow.show();

    return app.exec();
}



/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2007 Dirk Mueller <mueller@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QApplication>

#include "mainwindow.h"
#include "version.h"

#include <stdio.h>

void printHelp() {
    QString help;
    help += QApplication::translate("Usage", "Usage: %1 [options]").arg(appShortName);
    help += '\n';
    help += '\n';
    help += QApplication::translate("Description", description);
    help += '\n';
    help += '\n';
    help += QApplication::translate("Options", "Options:");
    help += '\n';
    help += QApplication::translate("Netname Options",  "\t-n, netname <name>\tIcecream network name");
    help += '\n';
    help += '\n';
    fputs(qPrintable(help), stdout);
}

int main( int argc, char **argv )
{
    QByteArray netName;

    for (int i = 1; i < argc; ++i ) {
        if (qstrcmp(argv[i], "-help") == 0 || qstrcmp(argv[i], "-h")  == 0) {
            printHelp();
            return 0;
        }
        if (qstrcmp(argv[i], "-netname") == 0 || qstrcmp(argv[i], "-n")  == 0) {
            if (i+1 < argc)
                netName = argv[++i];
        }
    }


    QApplication app(argc, argv);
    QApplication::setOrganizationDomain("kde.org");
    QApplication::setApplicationName(appShortName);
    QApplication::setApplicationVersion(version);

    MainWindow *mainWidget = new MainWindow;

    if ( !netName.isEmpty() ) {
        mainWidget->setCurrentNet( netName );
    }

    mainWidget->show();

    return app.exec();
}



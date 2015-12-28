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
#include <QCommandLineParser>

#include "mainwindow.h"
#include "version.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QApplication::setApplicationName(QLatin1String(Icemon::Version::appShortName));
    QApplication::setApplicationVersion(QLatin1String(Icemon::Version::version));

    QCommandLineParser parser;
    parser.setApplicationDescription(QLatin1String(Icemon::Version::description));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption netnameOption(QStringList() << QStringLiteral("n") << QStringLiteral("netname"),
        QCoreApplication::translate("main", "Icecream network name."),
        QCoreApplication::translate("main", "name", "network name"));
    parser.addOption(netnameOption);
    QCommandLineOption testmodeOption(QStringLiteral("testmode"),
        QCoreApplication::translate("main", "Testing mode."));
    parser.addOption(testmodeOption);

    parser.process(app);

    const QByteArray netName = parser.value(netnameOption).toLatin1();

    MainWindow mainWindow;
    if (!netName.isEmpty()) {
        mainWindow.setCurrentNet(netName);
    }
    if (parser.isSet(testmodeOption)) {
        mainWindow.setTestModeEnabled(true);
    }
    mainWindow.show();

    return app.exec();
}


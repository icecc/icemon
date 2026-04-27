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

#include "hostinfo.h"
#include "mainwindow.h"
#include "fakemonitor.h"
#include "icecreammonitor.h"
#include "version.h"

#include "config-icemon.h"

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
    QCommandLineOption schednameOption(QStringList() << QStringLiteral("s") << QStringLiteral("scheduler"),
        QCoreApplication::translate("main", "Icecream scheduler hostname"),
        QCoreApplication::translate("main", "hostname", "scheduler hostname"));
    parser.addOption(schednameOption);
    QCommandLineOption schedportOption(QStringList() << QStringLiteral("p") << QStringLiteral("port"),
        QCoreApplication::translate("main", "Icecream scheduler port"),
        QCoreApplication::translate("main", "port", "scheduler port"));
    parser.addOption(schedportOption);

#if ICEMON_DEVELOPER_BUILD
    QCommandLineOption testmodeOption(QStringLiteral("testmode"),
        QCoreApplication::translate("main", "Testing mode."));
    parser.addOption(testmodeOption);
#endif

    parser.process(app);

    const QString netName = parser.value(netnameOption);
    const QString schedName = parser.value(schednameOption);

    HostInfoManager hostInfoManager;

    QScopedPointer<Monitor> monitor;
#if ICEMON_DEVELOPER_BUILD
    if (parser.isSet(testmodeOption)) {
        monitor.reset(new FakeMonitor(&hostInfoManager));
    } else {
        monitor.reset(new IcecreamMonitor(&hostInfoManager));
    }
#else
    monitor.reset(new IcecreamMonitor(&hostInfoManager));
#endif

    if (!netName.isEmpty()) {
        monitor->setCurrentNetname(netName);
    }
    if (!schedName.isEmpty()) {
        monitor->setCurrentSchedname(schedName);
    }
    if (!parser.value(schedportOption).isEmpty()) {
        monitor->setCurrentSchedport(parser.value(schedportOption).toUInt());
    }

    MainWindow mainWindow;
    mainWindow.setMonitor(monitor.get());
    mainWindow.show();

    return app.exec();
}

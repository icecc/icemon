/*
    This file is part of Icecream.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "hostinfo.h"

#include <QApplication>

#include <qdebug.h>

QVector<QColor> HostInfo::mColorTable;
QMap<int, QString> HostInfo::mColorNameMap;

void HostInfo::initColorTable()
{
    initColor(QStringLiteral("#A5080B"), QApplication::tr("cherry"));
    initColor(QStringLiteral("#76d26f"), QApplication::tr("pistachio"));
    initColor(QStringLiteral("#664a08"), QApplication::tr("chocolate"));
    initColor(QStringLiteral("#4c9dff"), QApplication::tr("smurf"));
    initColor(QStringLiteral("#6c2ca8"), QApplication::tr("blueberry"));
    initColor(QStringLiteral("#fa8344"), QApplication::tr("orange"));
    initColor(QStringLiteral("#55CFBD"), QApplication::tr("mint"));
    initColor(QStringLiteral("#db1230"), QApplication::tr("strawberry"));
    initColor(QStringLiteral("#a6ea5e"), QApplication::tr("apple"));
    initColor(QStringLiteral("#D6A3D8"), QApplication::tr("bubblegum"));

    initColor(QStringLiteral("#f2aa4d"), QApplication::tr("peach"));
    initColor(QStringLiteral("#aa1387"), QApplication::tr("plum"));
    initColor(QStringLiteral("#26c3f7"), QApplication::tr("polar sea"));
    initColor(QStringLiteral("#b8850e"), QApplication::tr("nut"));
    initColor(QStringLiteral("#6a188d"), QApplication::tr("blackberry"));
    initColor(QStringLiteral("#24b063"), QApplication::tr("woodruff"));
    initColor(QStringLiteral("#ffff0f"), QApplication::tr("banana"));
    initColor(QStringLiteral("#1e1407"), QApplication::tr("mocha"));
    initColor(QStringLiteral("#29B450"), QApplication::tr("kiwi"));
    initColor(QStringLiteral("#F8DD31"), QApplication::tr("lemon"));

    initColor(QStringLiteral("#fa7e91"), QApplication::tr("raspberry"));
    initColor(QStringLiteral("#c5a243"), QApplication::tr("caramel"));
    initColor(QStringLiteral("#b8bcff"), QApplication::tr("blueberry"));
    initColor(QStringLiteral("#af3765"), QApplication::tr("blackcurrant"));
    initColor(QStringLiteral("#f7d36f"), QApplication::tr("passionfruit"));
    initColor(QStringLiteral("#d51013"), QApplication::tr("pomegranate"));
    initColor(QStringLiteral("#C2C032"), QApplication::tr("pumpkin"));
    initColor(QStringLiteral("#f0e8e3"), QApplication::tr("vanilla"));
    initColor(QStringLiteral("#d8e0e3"), QApplication::tr("stracciatella"));
    // try to make the count a prime number (reminder: 19, 23, 29, 31)
}

void HostInfo::initColor(const QString &value, const QString &name)
{
    QColor c(value);
    // modify colors so they become readable
    c.setHsv(c.hsvHue(), c.hsvSaturation() / 2, (c.value() / 3) + 150);
    mColorTable.append(c);

    mColorNameMap.insert(c.red() + c.green() * 256 + c.blue() * 65536, name);
}

QString HostInfo::colorName(const QColor &c)
{
    int key = c.red() + c.green() * 256 + c.blue() * 65536;

    return mColorNameMap.value(key, QApplication::tr("<unknown>"));
}

HostInfo::HostInfo(unsigned int id)
    : mId(id)
{
}

QString HostInfo::toolTip() const
{
    return QApplication::translate(("tooltip"),
                                   "<h3><b>%1</b></h3>"
                                   "<table>"
                                   "<tr><td>IP:</td><td>%2</td></tr>"
                                   "<tr><td>Platform:</td><td>%3</td></tr>"
                                   "<tr><td>Flavor:</td><td> %4</td></tr>"
                                   "<tr><td>Id:</td><td>%5</td></tr>"
                                   "<tr><td>Speed:</td><td>%6</td></tr>"
                                   "</table>"
                                   "<p><img align=\"right\" src=\":/images/icemonnode.png\"/></p>")
           .arg(name().toHtmlEscaped()).arg(ip())
           .arg(platform()).arg(colorName(color()).toHtmlEscaped())
           .arg(QString::number(id()))
           .arg(QString::number(serverSpeed()));
}

void HostInfo::updateFromStatsMap(const StatsMap &stats)
{
    QString name = stats[QStringLiteral("Name")];

    if (name != mName) {
        mName = name;
        mColor = createColor(mName);
        mIp = stats[QStringLiteral("IP")];
        mPlatform = stats[QStringLiteral("Platform")];
        mProtocol = stats[QStringLiteral("Version")].toInt();
        mFeatures = stats[QStringLiteral("Features")];
    }

    mNoRemote = (stats[QStringLiteral("NoRemote")].compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
    mMaxJobs = stats[QStringLiteral("MaxJobs")].toUInt();
    mOffline = (stats[QStringLiteral("State")] == QLatin1String("Offline"));

    mServerSpeed = stats[QStringLiteral("Speed")].toFloat();

    mServerLoad = stats[QStringLiteral("Load")].toUInt();
}

QColor HostInfo::createColor(const QString &name)
{
    unsigned long h = 0;

    for (uint i = 0; i < ( uint )name.length(); ++i) {
        const int ch = name[i].unicode();
        h = (h << 4) + ch;
        const unsigned long g = h & 0xf0000000;
        if (g != 0) {
            h ^= g >> 24;
            h ^= g;
        }
    }

    h += name.length() + (name.length() << 17);
    h ^= h >> 2;

    // qDebug() << "HostInfo::createColor: " << h % mColorTable.count() << ": " << name << endl;

    return mColorTable[h % mColorTable.count()];
}

QColor HostInfo::createColor()
{
    static int num = 0;

    return mColorTable.at(num++ % mColorTable.count());
}

HostInfoManager::HostInfoManager()
{
    HostInfo::initColorTable();
}

HostInfoManager::~HostInfoManager()
{
    qDeleteAll(mHostMap);
}

HostInfo *HostInfoManager::find(unsigned int hostid) const
{
    return mHostMap.value(hostid, nullptr);
}

void HostInfoManager::checkNode(const HostInfo &info)
{
    HostMap::ConstIterator it = mHostMap.constFind(info.id());
    if (it == mHostMap.constEnd()) {
        auto hostInfo = new HostInfo(info);
        mHostMap.insert(info.id(), hostInfo);
        emit hostMapChanged();
    } else {
        // no-op
    }
}

HostInfo *HostInfoManager::checkNode(unsigned int hostid,
                                     const HostInfo::StatsMap &stats)
{
    HostMap::ConstIterator it = mHostMap.constFind(hostid);
    HostInfo *hostInfo;
    if (it == mHostMap.constEnd()) {
        hostInfo = new HostInfo(hostid);
        mHostMap.insert(hostid, hostInfo);
    } else {
        hostInfo = *it;
    }

    hostInfo->updateFromStatsMap(stats);
    if (hostInfo->isOffline()) {
        mHostMap.remove(hostid);
        delete hostInfo;
        hostInfo = nullptr;
    }

    emit hostMapChanged();

    return hostInfo;
}

QString HostInfoManager::nameForHost(unsigned int id) const
{
    HostInfo *hostInfo = find(id);
    if (hostInfo) {
        return hostInfo->name();
    }

    return QApplication::tr("<unknown>");
}

QColor HostInfoManager::hostColor(unsigned int id) const
{
    if (id) {
        HostInfo *hostInfo = find(id);
        if (hostInfo) {
            QColor tmp = hostInfo->color();
            Q_ASSERT(tmp.isValid() && (tmp.red() + tmp.green() + tmp.blue()));
            return tmp;
        }
    }

    //qDebug() << "id " << id << " got no color\n";
    Q_ASSERT(false);

    return QColor(0, 0, 0);
}

unsigned int HostInfoManager::maxJobs(unsigned int id) const
{
    if (id) {
        HostInfo *hostInfo = find(id);
        if (hostInfo) {
            return hostInfo->maxJobs();
        }
    }

    return 0;
}

HostInfoManager::HostMap HostInfoManager::hostMap() const
{
    return mHostMap;
}

void HostInfoManager::setSchedulerName(const QString &schedulerName)
{
    mSchedulerName = schedulerName;
}

void HostInfoManager::setNetworkName(const QString &networkName)
{
    mNetworkName = networkName;
}

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
#ifndef ICEMON_HOSTINFO_H
#define ICEMON_HOSTINFO_H

#include <QString>
#include <QColor>
#include <QMap>
#include <QObject>
#include <QtCore/QVector>

class HostInfo
{
public:
    explicit HostInfo(unsigned int id = 0);

    unsigned int id() const { return mId; }

    void setName(const QString &name) { mName = name; }
    QString name() const { return mName; }

    void setColor(const QColor &color) { mColor = color; }
    QColor color() const { return mColor; }

    void setIp(const QString &ip) { mIp = ip; }
    QString ip() const { return mIp; }

    void setPlatform(const QString &platform) { mPlatform = platform; }
    QString platform() const { return mPlatform; }

    void setMaxJobs(unsigned int jobs) { mMaxJobs = jobs; }
    unsigned int maxJobs() const { return mMaxJobs; }

    void setOffline(bool offline) { mOffline = offline; }
    bool isOffline() const { return mOffline; }

    void setNoRemote(bool noRemote) { mNoRemote = noRemote; }
    bool noRemote() const { return mNoRemote; }

    typedef QMap<QString, QString> StatsMap;
    void updateFromStatsMap(const StatsMap &stats);

    static void initColorTable();
    static QString colorName(const QColor &);

    void setServerSpeed(float serverSpeed) { mServerSpeed = serverSpeed; }
    float serverSpeed() const { return mServerSpeed; }

    void setServerLoad(unsigned int load) { mServerLoad = load; }
    unsigned int serverLoad() const { return mServerLoad; }

    QString toolTip() const;

    bool operator==(const HostInfo &rhs) const { return mId == rhs.mId; }
    bool operator!=(const HostInfo &rhs) const { return mId != rhs.mId; }
    int operator<(const HostInfo &rhs) const { return mId < rhs.mId; }

protected:
    // TODO: Move the whole color managing feature into a separate class
    friend class FakeMonitor;
    static void initColor(const QString &value, const QString &name);

    QColor createColor();
    QColor createColor(const QString &name);

private:
    unsigned int mId;
    QString mName;
    QColor mColor;
    QString mPlatform;

    QString mIp;

    unsigned int mMaxJobs;
    bool mOffline;
    bool mNoRemote;

    float mServerSpeed;

    unsigned int mServerLoad;

    static QVector<QColor> mColorTable;
    static QMap<int, QString> mColorNameMap;
};

class HostInfoManager
    : public QObject
{
    Q_OBJECT

public:
    HostInfoManager();
    ~HostInfoManager() override;

    HostInfo *find(unsigned int hostid) const;

    typedef QMap<unsigned int, HostInfo *> HostMap;

    HostMap hostMap() const;

    void checkNode(const HostInfo &info);
    HostInfo *checkNode(unsigned int hostid,
                        const HostInfo::StatsMap &statmsg);

    QString nameForHost(unsigned int id) const;
    QColor hostColor(unsigned int id) const;
    unsigned int maxJobs(unsigned int id) const;

    QString schedulerName() const { return mSchedulerName; }
    void setSchedulerName(const QString &schedulerName);
    QString networkName() const { return mNetworkName; }
    void setNetworkName(const QString &networkName);

signals:
    void hostMapChanged();

private:
    HostMap mHostMap;
    QString mSchedulerName;
    QString mNetworkName;
};

#endif
// vim:ts=4:sw=4:noet

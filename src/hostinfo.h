/*
    This file is part of Icecream.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef ICEMON_HOSTINFO_H
#define ICEMON_HOSTINFO_H

#include <QString>
#include <QColor>
#include <QMap>
#include <QtCore/QVector>

class HostInfo
{
  public:
    HostInfo( unsigned int id );

    unsigned int id() const;

    QString name() const;
    QColor color() const;

    QString ip() const;
    QString platform() const;

    unsigned int maxJobs() const;
    bool isOffline() const;

    typedef QMap<QString,QString> StatsMap;
    void updateFromStatsMap( const StatsMap &stats );

    static void initColorTable();
    static QString colorName( const QColor & );

    float serverSpeed() const;

    unsigned int serverLoad() const;

    QString toolTip() const;

  protected:
    static void initColor( const QString &value, const QString &name );

    QColor createColor();
    QColor createColor( const QString &name );

  private:
    unsigned int mId;
    QString mName;
    QColor mColor;
    QString mPlatform;

    QString mIp;

    unsigned int mMaxJobs;
    bool mOffline;

    float mServerSpeed;

    unsigned int mServerLoad;

    static QVector<QColor> mColorTable;
    static QMap<int,QString> mColorNameMap;
};

class HostInfoManager
{
  public:
    HostInfoManager();
    ~HostInfoManager();

    HostInfo *find( unsigned int hostid ) const;

    typedef QMap<unsigned int,HostInfo *> HostMap;

    HostMap hostMap() const;

    HostInfo *checkNode( unsigned int hostid,
                         const HostInfo::StatsMap &statmsg );

    QString nameForHost( unsigned int id ) const;
    QColor hostColor( unsigned int id ) const;
    unsigned int maxJobs( unsigned int id ) const;

    QString schedulerName() const { return mSchedulerName; }
    void setSchedulerName( const QString& schedulerName );
    QString networkName() const { return mNetworkName; }
    void setNetworkName( const QString& networkName );

  private:
    HostMap mHostMap;
    QString mSchedulerName;
    QString mNetworkName;
};

#endif
// vim:ts=4:sw=4:noet

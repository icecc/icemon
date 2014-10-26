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
QMap<int,QString> HostInfo::mColorNameMap;

void HostInfo::initColorTable()
{
    initColor( "#A5080B", QApplication::tr("cherry") );
    initColor( "#76d26f", QApplication::tr("pistachio"));
    initColor( "#664a08", QApplication::tr("chocolate"));
    initColor( "#4c9dff", QApplication::tr("smurf"));
    initColor( "#6c2ca8", QApplication::tr("blueberry"));
    initColor( "#fa8344", QApplication::tr("orange"));
    initColor( "#55CFBD", QApplication::tr("mint"));
    initColor( "#db1230", QApplication::tr("strawberry"));
    initColor( "#a6ea5e", QApplication::tr("apple"));
    initColor( "#D6A3D8", QApplication::tr("bubblegum"));
    initColor( "#f2aa4d", QApplication::tr("peach"));
    initColor( "#aa1387", QApplication::tr("plum"));
    initColor( "#26c3f7", QApplication::tr("polar sea"));
    initColor( "#b8850e", QApplication::tr("nut"));
    initColor( "#6a188d", QApplication::tr("blackberry"));
    initColor( "#24b063", QApplication::tr("woodruff"));
    initColor( "#ffff0f", QApplication::tr("banana"));
    initColor( "#1e1407", QApplication::tr("mocha"));
    initColor( "#29B450", QApplication::tr("kiwi"));
    initColor( "#F8DD31", QApplication::tr("lemon"));
    initColor( "#fa7e91", QApplication::tr("raspberry"));
    initColor( "#c5a243", QApplication::tr("caramel"));
    initColor( "#b8bcff", QApplication::tr("blueberry"));
    // try to make the count a prime number (reminder: 19, 23, 29, 31)
    // initColor( "#ffb8c0", QApplication::tr("blackcurrant"));
    // initColor( "#f7d36f", QApplication::tr("passionfruit"));
    // initColor( "#d51013", QApplication::tr("pomegranate"));
    // initColor( "#C2C032", QApplication::tr("pumpkin" ) );
}

void HostInfo::initColor( const QString &value , const QString &name )
{
    QColor c( value );
    // modify colors so they become readable
    c.setHsv( c.hsvHue(), c.hsvSaturation()/2, (c.value()/3)+150 );
    mColorTable.append( c );

    mColorNameMap.insert( c.red() + c.green() * 256 + c.blue() * 65536, name );
}

QString HostInfo::colorName( const QColor &c )
{
  int key = c.red() + c.green() * 256 + c.blue() * 65536;

  return mColorNameMap.value( key, QApplication::tr("<unknown>") );
}

HostInfo::HostInfo( unsigned int id )
  : mId( id )
{
}

QString HostInfo::toolTip() const
{
    return QApplication::translate(("tooltip"),
    "<p><table><tr><td>"
    "<img align=\"right\" src=\":/images/icemonnode.png\"><br><b>%1"
    "</b><br>"
    "<table>"
    "<tr><td>IP:</td><td>%2</td></tr>"
    "<tr><td>Platform:</td><td>%3</td></tr>"
    "<tr><td>Flavor:</td><td> %4</td></tr>"
    "<tr><td>Id:</td><td>%5</td></tr>"
    "<tr><td>Speed:</td><td>%6</td></tr>"
    "</table></td></tr></table></p>")
            .arg(name()).arg(ip())
            .arg(platform()).arg(colorName(color()))
            .arg(QString::number(id()))
            .arg(QString::number(serverSpeed()));
}

void HostInfo::updateFromStatsMap( const StatsMap &stats )
{
  QString name = stats["Name"];

  if ( name != mName ) {
    mName = name;
    mColor = createColor( mName );
    mIp = stats["IP"];
    mPlatform = stats["Platform"];
  }

  mNoRemote = ( stats["NoRemote"].compare( "true", Qt::CaseInsensitive ) == 0 );
  mMaxJobs = stats["MaxJobs"].toUInt();
  mOffline = ( stats["State"] == "Offline" );

  mServerSpeed = stats["Speed"].toFloat();

  mServerLoad = stats["Load"].toUInt();
}

QColor HostInfo::createColor( const QString &name )
{
    unsigned long h = 0;
    unsigned long g;
    int ch;

    for( uint i = 0; i < (uint)name.length(); ++i ) {
        ch = name[i].unicode();
        h = (h << 4) + ch;
        if ((g = (h & 0xf0000000)) != 0)
        {
            h ^= g >> 24;
            h ^= g;
        }
    }

    h += name.length() + ( name.length() << 17 );
    h ^= h >> 2;

    // qDebug() << "HostInfo::createColor: " << h % mColorTable.count() << ": " << name << endl;

    return mColorTable[ h % mColorTable.count() ];
}

QColor HostInfo::createColor()
{
  static int num = 0;

  return mColorTable.at( num++ % mColorTable.count() );
}

HostInfoManager::HostInfoManager()
{
  HostInfo::initColorTable();
}

HostInfoManager::~HostInfoManager()
{
  qDeleteAll(mHostMap);
}

HostInfo *HostInfoManager::find( unsigned int hostid ) const
{
  return mHostMap.value( hostid, 0 );
}

void HostInfoManager::checkNode(const HostInfo &info)
{
  HostMap::ConstIterator it = mHostMap.constFind(info.id());
  if (it == mHostMap.constEnd()) {
    HostInfo *hostInfo = new HostInfo(info);
    mHostMap.insert(info.id(), hostInfo);
    emit hostMapChanged();
  } else {
    // no-op
  }
}

HostInfo *HostInfoManager::checkNode( unsigned int hostid,
                                      const HostInfo::StatsMap &stats )
{
  HostMap::ConstIterator it = mHostMap.constFind( hostid );
  HostInfo *hostInfo;
  if ( it == mHostMap.constEnd() ) {
    hostInfo = new HostInfo( hostid );
    mHostMap.insert( hostid, hostInfo );
  } else {
    hostInfo = *it;
  }

  hostInfo->updateFromStatsMap( stats );
  emit hostMapChanged();

  return hostInfo;
}

QString HostInfoManager::nameForHost( unsigned int id ) const
{
  HostInfo *hostInfo = find( id );
  if ( hostInfo ) return hostInfo->name();

  return QApplication::tr("<unknown>");
}

QColor HostInfoManager::hostColor( unsigned int id ) const
{
  if ( id ) {
    HostInfo *hostInfo = find( id );
    if ( hostInfo ) {
        QColor tmp = hostInfo->color();
        Q_ASSERT( tmp.isValid() && ( tmp.red() + tmp.green() + tmp.blue() ));
        return tmp;
    }
  }

  //qDebug() << "id " << id << " got no color\n";
  Q_ASSERT( false );

  return QColor( 0, 0, 0 );
}

unsigned int HostInfoManager::maxJobs( unsigned int id ) const
{
  if ( id ) {
    HostInfo *hostInfo = find( id );
    if ( hostInfo ) return hostInfo->maxJobs();
  }

  return 0;
}

HostInfoManager::HostMap HostInfoManager::hostMap() const
{
  return mHostMap;
}

void HostInfoManager::setSchedulerName( const QString& schedulerName )
{
    mSchedulerName = schedulerName;
}

void HostInfoManager::setNetworkName( const QString& networkName )
{
    mNetworkName = networkName;
}

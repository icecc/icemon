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

#include "hostinfo.h"

#include <kdebug.h>
#include <klocale.h>

QValueVector<QColor> HostInfo::mColorTable;
QMap<int,QString> HostInfo::mColorNameMap;

void HostInfo::initColorTable()
{
    initColor( "#A5080B", i18n("cherry") );
    initColor( "#76d26f", i18n("pistachio"));
    initColor( "#664a08", i18n("chocolate"));
    initColor( "#4c9dff", i18n("smurf"));
    initColor( "#6c2ca8", i18n("blueberry"));
    initColor( "#fa8344", i18n("orange"));
    initColor( "#55CFBD", i18n("mint"));
    initColor( "#db1230", i18n("strawberry"));
    initColor( "#a6ea5e", i18n("apple"));
    initColor( "#D6A3D8", i18n("bubblegum"));
    initColor( "#f2aa4d", i18n("peach"));
    initColor( "#aa1387", i18n("plum"));
    initColor( "#26c3f7", i18n("polar sea"));
    initColor( "#b8850e", i18n("nut"));
    initColor( "#6a188d", i18n("blackberry"));
    initColor( "#24b063", i18n("woodruff"));
    initColor( "#ffff0f", i18n("banana"));
    initColor( "#1e1407", i18n("mocha"));
    initColor( "#29B450", i18n("kiwi"));
    initColor( "#F8DD31", i18n("lemon"));
    initColor( "#fa7e91", i18n("raspberry"));
    initColor( "#c5a243", i18n("caramel"));
    initColor( "#b8bcff", i18n("blueberry"));
    // try to make the count a prime number (reminder: 19, 23, 29, 31)
    // initColor( "#ffb8c0", i18n("blackcurrant"));
    // initColor( "#f7d36f", i18n("passionfruit"));
    // initColor( "#d51013", i18n("pomegranate"));
    // initColor( "#C2C032", i18n("pumpkin" ) );
}

void HostInfo::initColor( const QString &value , const QString &name )
{
    QColor c( value );
    mColorTable.append( c );

    mColorNameMap.insert( c.red() + c.green() * 256 + c.blue() * 65536, name );
}

QString HostInfo::colorName( const QColor &c )
{
  int key = c.red() + c.green() * 256 + c.blue() * 65536;

  QMap<int,QString>::ConstIterator it = mColorNameMap.find( key );
  if ( it == mColorNameMap.end() ) return i18n("<unknown>");
  else return *it;
}

HostInfo::HostInfo( unsigned int id )
  : mId( id )
{
}

unsigned int HostInfo::id() const
{
  return mId;
}

QColor HostInfo::color() const
{
  return mColor;
}

QString HostInfo::name() const
{
  return mName;
}

QString HostInfo::ip() const
{
  return mIp;
}

QString HostInfo::platform() const
{
    return mPlatform;
}

unsigned int HostInfo::maxJobs() const
{
  return mMaxJobs;
}

bool HostInfo::isOffline() const
{
  return mOffline;
}

float HostInfo::serverSpeed() const
{
    return mServerSpeed;
}

void HostInfo::updateFromStatsMap( const StatsMap &stats )
{
#if 0
  kdDebug() << "HostInfo::updateFromStatsMap():" << endl;
  StatsMap::ConstIterator it;
  for( it = stats.begin(); it != stats.end(); it++ ) {
    kdDebug() << "  STAT: " << it.key() << ": " << it.data() << endl;
  }
#endif

  QString name = stats["Name"];

  if ( name != mName ) {
    mName = name;
    mColor = createColor( mName );
    mIp = stats["IP"];
    mPlatform = stats["Platform"];
  }

  mMaxJobs = stats["MaxJobs"].toUInt();
  mOffline = ( stats["State"] == "Offline" );

  mServerSpeed = stats["Speed"].toFloat();
}

QColor HostInfo::createColor( const QString &name )
{
    unsigned long h = 0;
    unsigned long g;
    int ch;

    for( uint i = 0; i < name.length(); ++i ) {
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

    // kdDebug() << "HostInfo::createColor: " << h % mColorTable.count() << ": " << name << endl;

    return mColorTable[ h % mColorTable.count() ];
}

QColor HostInfo::createColor()
{
  static int num = 0;

  return mColorTable.at( num++ % mColorTable.count() );

#if 0
  QColor color( num, 255 - num, ( num * 3 ) % 255 );

  num += 48;
  num %= 255;

  return color;
#endif
}

HostInfoManager::HostInfoManager()
{
  HostInfo::initColorTable();
}

HostInfoManager::~HostInfoManager()
{
  HostMap::ConstIterator it;
  for( it = mHostMap.begin(); it != mHostMap.end(); ++it ) {
    delete *it;
  }
}

HostInfo *HostInfoManager::find( unsigned int hostid ) const
{
  HostMap::ConstIterator it = mHostMap.find( hostid );
  if ( it == mHostMap.end() ) return 0;
  else return *it;
}

HostInfo *HostInfoManager::checkNode( unsigned int hostid,
                                      const HostInfo::StatsMap &stats )
{
  HostMap::ConstIterator it = mHostMap.find( hostid );
  HostInfo *hostInfo;
  if ( it == mHostMap.end() ) {
    hostInfo = new HostInfo( hostid );
    mHostMap.insert( hostid, hostInfo );
  } else {
    hostInfo = *it;
  }

  hostInfo->updateFromStatsMap( stats );

  return hostInfo;
}

QString HostInfoManager::nameForHost( unsigned int id ) const
{
  if ( !id ) {
    kdError() << "Unknown host" << endl;
  } else {
    HostInfo *hostInfo = find( id );
    if ( hostInfo ) return hostInfo->name();
  }

  return i18n("<unknown>");
}

QColor HostInfoManager::hostColor( unsigned int id ) const
{
  if ( id ) {
    HostInfo *hostInfo = find( id );
    if ( hostInfo ) return hostInfo->color();
  }

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

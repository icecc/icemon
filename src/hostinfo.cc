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

#include <assert.h>

QVector<QColor> HostInfo::mColorTable;
QMap<int,QString> HostInfo::mColorNameMap;

void HostInfo::initColorTable()
{
    initColor( "#A5080B", QObject::tr("cherry") );
    initColor( "#76d26f", QObject::tr("pistachio"));
    initColor( "#664a08", QObject::tr("chocolate"));
    initColor( "#4c9dff", QObject::tr("smurf"));
    initColor( "#6c2ca8", QObject::tr("blueberry"));
    initColor( "#fa8344", QObject::tr("orange"));
    initColor( "#55CFBD", QObject::tr("mint"));
    initColor( "#db1230", QObject::tr("strawberry"));
    initColor( "#a6ea5e", QObject::tr("apple"));
    initColor( "#D6A3D8", QObject::tr("bubblegum"));
    initColor( "#f2aa4d", QObject::tr("peach"));
    initColor( "#aa1387", QObject::tr("plum"));
    initColor( "#26c3f7", QObject::tr("polar sea"));
    initColor( "#b8850e", QObject::tr("nut"));
    initColor( "#6a188d", QObject::tr("blackberry"));
    initColor( "#24b063", QObject::tr("woodruff"));
    initColor( "#ffff0f", QObject::tr("banana"));
    initColor( "#1e1407", QObject::tr("mocha"));
    initColor( "#29B450", QObject::tr("kiwi"));
    initColor( "#F8DD31", QObject::tr("lemon"));
    initColor( "#fa7e91", QObject::tr("raspberry"));
    initColor( "#c5a243", QObject::tr("caramel"));
    initColor( "#b8bcff", QObject::tr("blueberry"));
    // try to make the count a prime number (reminder: 19, 23, 29, 31)
    // initColor( "#ffb8c0", QObject::tr("blackcurrant"));
    // initColor( "#f7d36f", QObject::tr("passionfruit"));
    // initColor( "#d51013", QObject::tr("pomegranate"));
    // initColor( "#C2C032", QObject::tr("pumpkin" ) );
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
  if ( it == mColorNameMap.end() ) return QObject::tr("<unknown>");
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

unsigned int HostInfo::serverLoad() const
{
    return mServerLoad;
}

void HostInfo::updateFromStatsMap( const StatsMap &stats )
{
#if 0
  qDebug() << "HostInfo::updateFromStatsMap():" << endl;
  StatsMap::ConstIterator it;
  for( it = stats.begin(); it != stats.end(); it++ ) {
    qDebug() << "  STAT: " << it.key() << ": " << it.data() << endl;
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
  HostInfo *hostInfo = find( id );
  if ( hostInfo ) return hostInfo->name();

  return QObject::tr("<unknown>");
}

QColor HostInfoManager::hostColor( unsigned int id ) const
{
  if ( id ) {
    HostInfo *hostInfo = find( id );
    if ( hostInfo ) {
        QColor tmp = hostInfo->color();
        assert( tmp.isValid() && ( tmp.red() + tmp.green() + tmp.blue() ));
        return tmp;
    }
  }

  //qDebug() << "id " << id << " got no color\n";
  assert( false );

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

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
    // Sorry, the names are in german as my knowledge of the english language
    // doesn't seem to be good enough to translate icecream flavors.
    initColor( "#A5080B", i18n("kirsche") );
    initColor( "#76d26f", i18n("pistazie"));
    initColor( "#664a08", i18n("schokolade"));
    initColor( "#4c9dff", i18n("schlumpf-eis"));
    initColor( "#6c2ca8", i18n("blaubeere"));
    initColor( "#fa8344", i18n("orange"));
    initColor( "#55CFBD", i18n("minze"));
    initColor( "#db1230", i18n("erdbeer"));
    initColor( "#a6ea5e", i18n("apfel"));
    initColor( "#D6A3D8", i18n("kaugummi"));
    initColor( "#f2aa4d", i18n("pfirsich"));
    initColor( "#aa1387", i18n("pflaume"));
    initColor( "#26c3f7", i18n("eismeer"));
    initColor( "#b8850e", i18n("nuss"));
    initColor( "#6a188d", i18n("brombeere"));
    initColor( "#24b063", i18n("waldmeister"));
    initColor( "#ffff0f", i18n("banane"));
    initColor( "#1e1407", i18n("mocca"));
    initColor( "#29B450", i18n("kiwi"));
    initColor( "#f7d36f", i18n("maracuja"));
    initColor( "#F8DD31", i18n("zitrone"));
    initColor( "#fa7e91", i18n("himbeere"));
    initColor( "#c5a243", i18n("karamel"));
    initColor( "#b8bcff", i18n("heidelbeere"));
    initColor( "#ffb8c0", i18n("johannisbeere"));
    initColor( "#d51013", i18n("granatapfel"));
    initColor( "#b77a2a", i18n("zimt"));
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

unsigned int HostInfo::maxJobs() const
{
  return mMaxJobs;
}

bool HostInfo::isOffline() const
{
  return mOffline;
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
  }

  mMaxJobs = stats["MaxJobs"].toUInt();
  mOffline = ( stats["State"] == "Offline" );
}

QColor HostInfo::createColor( const QString &name )
{
  int n = 0;
  for( uint i = 0; i < name.length(); ++i ) {
    n += name[ i ].unicode();
  }

//  kdDebug() << "HostInfo::createColor: " << name << ": " << n << endl;

  return mColorTable[ n % mColorTable.count() ];
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

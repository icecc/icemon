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
  initColor( 210,  21, 100 , i18n("kirsche") );
  initColor( 118, 210, 111 , i18n("pistazie") );
  initColor( 102,  74,   8 , i18n("schokolade") );
  initColor( 255, 248, 167 , i18n("vanille") );
  initColor(  76, 157, 255 , i18n("schlumpf-eis") );
  initColor( 108,  44, 168 , i18n("blaubeere") );
  initColor( 250, 131,  68 , i18n("orange") );
  initColor( 181, 255, 224 , i18n("minze") );
  initColor( 219,  18,  48 , i18n("erdbeer") );
  initColor( 166, 234,  94 , i18n("apfel") );
  initColor( 245, 186, 247 , i18n("kaugummi") );
  initColor( 242, 170,  77 , i18n("pfirsich") );
  initColor( 170,  19, 135 , i18n("pflaume") );
  initColor(  38, 195, 247 , i18n("eismeer") );
  initColor( 184, 133,  14 , i18n("nuss") );
  initColor( 106,  24, 141 , i18n("brombeere") );
  initColor(  36, 176,  99 , i18n("waldmeister") );
  initColor( 255, 255,  15 , i18n("banane") );
  initColor(  30,  20,   7 , i18n("mocca") );
  initColor(  60, 183,  73 , i18n("kiwi") );
  initColor( 247, 211, 111 , i18n("maracuja") );
  initColor( 248, 255, 200 , i18n("zitrone") );
  initColor( 250, 126, 145 , i18n("himbeere") );
  initColor( 197, 162,  67 , i18n("karamel") );
  initColor( 184, 188, 255 , i18n("heidelbeere") );
  initColor( 255, 184, 192 , i18n("johannisbeere") );
  initColor( 213,  16,  19 , i18n("granatapfel") );
  initColor( 183, 122,  42 , i18n("zimt") );
}

void HostInfo::initColor( int r, int g, int b, const QString &name )
{
  mColorTable.append( QColor( r, g, b ) );

  mColorNameMap.insert( r + g * 256 + b * 65536, name );
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

HostInfo *HostInfoManager::find( unsigned int hostid )
{
  HostMap::ConstIterator it = mHostMap.find( hostid );
  if ( it == mHostMap.end() ) return 0;
  else return *it;
}

void HostInfoManager::checkNode( unsigned int hostid,
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
}

QString HostInfoManager::nameForHost( unsigned int id )
{
  if ( !id ) {
    kdError() << "Unknown host" << endl;
  } else {
    HostInfo *hostInfo = find( id );
    if ( hostInfo ) return hostInfo->name();
  }

  return i18n("<unknown>");
}

QColor HostInfoManager::hostColor( unsigned int id )
{
  if ( id ) {
    HostInfo *hostInfo = find( id );
    if ( hostInfo ) return hostInfo->color();
  }
  
  return QColor( 0, 0, 0 );
}

unsigned int HostInfoManager::maxJobs( unsigned int id )
{
  if ( id ) {
    HostInfo *hostInfo = find( id );
    if ( hostInfo ) return hostInfo->maxJobs();
  }
  
  return 0;
}

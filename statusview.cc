/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "statusview.h"

#include <klocale.h>
#include <kdebug.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

QString Job::stateAsString() const
{
    switch ( m_state ) {
    case WaitingForCS:
        return i18n( "Waiting" );
        break;
    case Compiling:
        return i18n( "Compiling" );
        break;
    case Finished:
        return i18n( "Finished" );
        break;
    case Failed:
        return i18n( "Failed" );
        break;
    case Idle:
        return i18n( "Idle" );
        break;
    case LocalOnly:
        return i18n( "LocalOnly" );
        break;
    }
    return QString::null;
}

void StatusView::checkNode( unsigned int hostid, const QString &statmsg )
{
    QString first = statmsg.left( statmsg.find( '\n' ) );
    if ( first.startsWith( "Name:" ) ) {
        first = first.mid( 5 ).stripWhiteSpace();
        kdDebug() << "host " << hostid << " " << first << endl;
        if ( mHostNameMap[hostid] != first ) {
            mHostColorMap[hostid] = QColor();
            mHostNameMap[hostid] = first;
        }
    }
}

QString StatusView::nameForHost( unsigned int id )
{
    if ( !id ) {
        kdError() << "Unknown host" << endl;
        return i18n("<unknown>");
    }

    QMap<unsigned int,QString>::ConstIterator it;
    it = mHostNameMap.find( id );
    if ( it != mHostNameMap.end() ) {
        return *it;
    }

    return i18n("<unknown>");
}

QColor StatusView::hostColor( unsigned int id )
{
  QMap<unsigned int,QColor>::ConstIterator it;
  it = mHostColorMap.find( id );
  if ( it != mHostColorMap.end() && ( *it ).isValid() ) {
    return *it;
  }

  static int num = 0;

  QColor color( num, 255 - num, ( num * 3 ) % 255 );

  mHostColorMap.insert( id, color );

  num += 48;
  num %= 255;

  return color;
}

void StatusView::inherit( StatusView *old )
{
    mHostColorMap = old->mHostColorMap;
    mHostNameMap = old->mHostNameMap;
}

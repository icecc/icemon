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
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "statusview.h"

#include "hostinfo.h"
#include "job.h"

#include <assert.h>
#include <QDebug>
#include <QTime>

StatusView::StatusView( HostInfoManager *m )
    : mHostInfoManager( m ), m_paused( false )
{
}

StatusView::~StatusView()
{
}

void StatusView::checkNode( unsigned int )
{
}

void StatusView::removeNode( unsigned int )
{
}

void StatusView::updateSchedulerState( bool online )
{
#if 0
  qDebug() << "Scheduler is" << ( online ? "online" : "offline" );
#endif
}

QString StatusView::nameForHost( unsigned int id )
{
  return mHostInfoManager->nameForHost( id );
}

QColor StatusView::hostColor( unsigned int id )
{
  return mHostInfoManager->hostColor( id );
}

QColor StatusView::textColor( const QColor &color )
{
  QColor textColor;

  float luminance = ( color.red() * 0.299 ) + ( color.green() * 0.587 ) +
                    ( color.blue() * 0.114 );
  if ( luminance > 140.0 ) textColor = Qt::black;
  else textColor = Qt::white;

  return textColor;
}

unsigned int StatusView::processor( const Job &job )
{
    unsigned int ret = 0;
    if ( job.state() == Job::LocalOnly || job.state() == Job::WaitingForCS ) {
        ret = job.client();
    } else {
        ret = job.server();
        if ( !ret ) {
	  //            Q_ASSERT( job.state() == Job::Finished );
            ret = job.client();
        }
    }
    assert( ret );
    return ret;
}

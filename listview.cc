/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>

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

#include "listview.h"

#include <services/logging.h>

#include <klocale.h>

using namespace std;

ListStatusViewItem::ListStatusViewItem( QListView *parent, const Job &_job )
    :  QListViewItem( parent ), job( _job )
{
    updateText( job );
}

void ListStatusViewItem::updateText( const Job &job)
{
    this->job = job;
    setText( 0, QString::number( job.jobId() ) );
    setText( 1, job.fileName() );
    ListStatusView *p = dynamic_cast<ListStatusView*>( listView() );
    if ( p ) {
        setText( 2, p->nameForHost( job.client() ) );
        if ( job.server() )
            setText( 3, p->nameForHost( job.server() ) );
        else
            setText( 3, QString::null );
    }
    setText( 4, job.stateAsString() );
    setText( 5, QString::number( job.real_msec ) );
    setText( 6, QString::number( job.user_msec ) );
    setText( 7, QString::number( job.majflt ) );
    setText( 8, QString::number( job.in_uncompressed ) );
    setText( 9, QString::number( job.out_uncompressed ) );
}

inline int compare( unsigned int i1, unsigned int i2 )
{
    if ( i1 < i2 )
        return -1;
    else if ( i1 == i2 )
        return 0;
    else
        return 1;
}

int ListStatusViewItem::compare( QListViewItem *i, int col,
                                 bool ) const
{
    const ListStatusViewItem *first = this;
    const ListStatusViewItem *other = dynamic_cast<ListStatusViewItem*>( i );
    assert( other );

    switch ( col ) {
    case 0:
        return ::compare( first->job.jobId(), other->job.jobId() );
    case 5:
        return ::compare( first->job.real_msec, other->job.real_msec );
    case 6:
        return ::compare( first->job.user_msec, other->job.user_msec );
    case 7:
        return ::compare( first->job.majflt, other->job.majflt );
    case 8:
        return ::compare( first->job.in_uncompressed, other->job.in_uncompressed );
    case 9:
        return ::compare( first->job.out_uncompressed, other->job.out_uncompressed );
    default:
        return first->text(col).compare( other->text( col ) );
    }
}

ListStatusView::ListStatusView( HostInfoManager *m, QWidget *parent,
                                const char *name )
	: KListView( parent, name ), StatusView( m )
{
    addColumn( i18n( "ID" ) );
    addColumn( i18n( "Filename" ) );
    addColumn( i18n( "Client" ) );
    addColumn( i18n( "Server" ) );
    addColumn( i18n( "State" ) );
    addColumn( i18n( "Real" ) );
    addColumn( i18n( "User" ) );
    addColumn( i18n( "Faults" ) );
    addColumn( i18n( "Size In" ) );
    addColumn( i18n( "Size Out" ) );
}

void ListStatusView::update( const Job &job )
{
    ItemMap::iterator it = items.find( job.jobId() );
    if ( it == items.end() )
    {
        items[job.jobId()] = new ListStatusViewItem( this, job );

    } else
    {
        ( *it )->updateText( job );
    }
}

#include "listview.moc"

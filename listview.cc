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

QString convertSize( unsigned int size )
{
    int divisor;
    QString str;

    if ( size >= (1 << 20) )
    {
        divisor = 1 << 20;
        str = i18n( "%1 MB" );
    }

    else if ( size >= (1 << 10) )
    {
        divisor = 1 << 10;
        str = i18n( "%1 KB" );
    }

    else
    {
        divisor = 1;
        str = i18n( "%1 B" );
    }

    return str.arg( KGlobal::locale()->formatNumber( static_cast<double>(size) / divisor, 1 ) );
}

enum Columns
{
    ColumnID,
    ColumnFilename,
    ColumnClient,
    ColumnServer,
    ColumnState,
    ColumnReal,
    ColumnUser,
    ColumnFaults,
    ColumnSizeIn,
    ColumnSizeOut
};

ListStatusViewItem::ListStatusViewItem( QListView *parent, const Job &_job )
    :  QListViewItem( parent ), job( _job )
{
    updateText( job );
}

void ListStatusViewItem::updateText( const Job &job)
{
    this->job = job;
    setText( ColumnID, QString::number( job.jobId() ) );
    setText( ColumnFilename, job.fileName() );
    ListStatusView *p = dynamic_cast<ListStatusView*>( listView() );
    if ( p ) {
        setText( ColumnClient, p->nameForHost( job.client() ) );
        if ( job.server() )
            setText( ColumnServer, p->nameForHost( job.server() ) );
        else
            setText( ColumnServer, QString::null );
    }
    setText( ColumnState, job.stateAsString() );
    setText( ColumnReal, QString::number( job.real_msec ) );
    setText( ColumnUser, QString::number( job.user_msec ) );
    setText( ColumnFaults, QString::number( job.majflt ) );
    setText( ColumnSizeIn, convertSize( job.in_uncompressed ) );
    setText( ColumnSizeOut, convertSize( job.out_uncompressed ) );
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
    case ColumnID:
        return ::compare( first->job.jobId(), other->job.jobId() );
    case ColumnReal:
        return ::compare( first->job.real_msec, other->job.real_msec );
    case ColumnUser:
        return ::compare( first->job.user_msec, other->job.user_msec );
    case ColumnFaults:
        return ::compare( first->job.majflt, other->job.majflt );
    case ColumnSizeIn:
        return ::compare( first->job.in_uncompressed, other->job.in_uncompressed );
    case ColumnSizeOut:
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

    setColumnAlignment( ColumnID, Qt::AlignRight );
    setColumnAlignment( ColumnReal, Qt::AlignRight );
    setColumnAlignment( ColumnUser, Qt::AlignRight );
    setColumnAlignment( ColumnFaults, Qt::AlignRight );
    setColumnAlignment( ColumnSizeIn, Qt::AlignRight );
    setColumnAlignment( ColumnSizeOut, Qt::AlignRight );

    setAllColumnsShowFocus(true);
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

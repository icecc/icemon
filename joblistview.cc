/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2004 Andre Wöbbeking <Woebbeking@web.de>

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

#include "joblistview.h"

#include "hostinfo.h"

#include <kglobal.h>
#include <klocale.h>

#include <qdir.h>
#include <qtimer.h>


static QString convertSize( unsigned int size )
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


JobListViewItem::JobListViewItem( KListView* parent, const Job& job )
    :  KListViewItem( parent )
{
    updateText( job );
}


void JobListViewItem::updateText( const Job& job)
{
    const bool fileNameChanged( mJob.fileName() != job.fileName() );

    mJob = job;

    setText( ColumnID, QString::number( job.jobId() ) );
    if ( JobListView* view = dynamic_cast<JobListView*>( listView() ) )
    {
        setText( ColumnClient, view->hostInfoManager()->nameForHost( job.client() ) );
        if ( job.server() )
            setText( ColumnServer, view->hostInfoManager()->nameForHost( job.server() ) );
        else
            setText( ColumnServer, QString::null );
    }
    setText( ColumnState, job.stateAsString() );
    setText( ColumnReal, QString::number( job.real_msec ) );
    setText( ColumnUser, QString::number( job.user_msec ) );
    setText( ColumnFaults, QString::number( job.majflt ) );
    setText( ColumnSizeIn, convertSize( job.in_uncompressed ) );
    setText( ColumnSizeOut, convertSize( job.out_uncompressed ) );

    if ( fileNameChanged )
        updateFileName();
}


void JobListViewItem::updateFileName()
{
    JobListView* view = dynamic_cast<JobListView*>( listView() );
    if ( !view )
        return;

    const char separator = QDir::separator();

    QString fileName = mJob.fileName();

    const int numberOfFilePathParts = view->numberOfFilePathParts();
    if ( numberOfFilePathParts > 0 )
    {
        int counter = numberOfFilePathParts;
        int index = 0;
        do
        {
            index = fileName.findRev( separator, index - 1);
        }
        while ( counter-- && ( index > 0 ) );

        if ( index > 0 )
            fileName = QString::fromLatin1( "..." ) + fileName.mid( index );
    }
    else if ( numberOfFilePathParts == 0)
    {
        fileName = fileName.mid( fileName.findRev( separator ) + 1);
    }

    setText( ColumnFilename, fileName );
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


int JobListViewItem::compare( QListViewItem* item,
                              int column,
                              bool ) const
{
    const JobListViewItem* first = this;
    const JobListViewItem* other = dynamic_cast<JobListViewItem*>( item );

    switch ( column )
    {
    case ColumnID:
        return ::compare( first->mJob.jobId(), other->mJob.jobId() );
    case ColumnReal:
        return ::compare( first->mJob.real_msec, other->mJob.real_msec );
    case ColumnUser:
        return ::compare( first->mJob.user_msec, other->mJob.user_msec );
    case ColumnFaults:
        return ::compare( first->mJob.majflt, other->mJob.majflt );
    case ColumnSizeIn:
        return ::compare( first->mJob.in_uncompressed, other->mJob.in_uncompressed );
    case ColumnSizeOut:
        return ::compare( first->mJob.out_uncompressed, other->mJob.out_uncompressed );
    default:
        return first->text( column ).compare( other->text( column ) );
    }
}


JobListView::JobListView( const HostInfoManager* manager,
                          QWidget* parent,
                          const char* name )
    : KListView( parent, name ),
      mHostInfoManager( manager ),
      mNumberOfFilePathParts( 2 ),
      mExpireDuration( -1 ),
      mExpireTimer( new QTimer( this ) )
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

    connect(mExpireTimer, SIGNAL( timeout() ),
            this, SLOT( slotExpireFinishedJobs() ) );
}


void JobListView::update( const Job& job )
{
    ItemMap::iterator it = mItems.find( job.jobId() );
    if ( it == mItems.end() )
        it = mItems.insert( job.jobId(), new JobListViewItem( this, job ) );
    else
        ( *it )->updateText( job );

    const bool finished = ( job.state() == Job::Finished ) || ( job.state() == Job::Failed );
    if ( finished )
        expireItem( *it );
}


int JobListView::numberOfFilePathParts() const
{
    return mNumberOfFilePathParts;
}


void JobListView::setNumberOfFilePathParts( int number )
{
    if ( number == mNumberOfFilePathParts )
        return;
}


bool JobListView::isClientColumnVisible() const
{
    return columnWidth( ColumnClient );
}


void JobListView::setClientColumnVisible( bool visible )
{
    if ( visible == isClientColumnVisible() )
        return;

    if ( visible )
    {
        setColumnWidthMode( ColumnClient, Maximum );
        setColumnWidth( ColumnClient, 50 ); // at least the user can see it again
    }
    else
    {
        setColumnWidthMode( ColumnClient, Manual );
        setColumnWidth( ColumnClient, 0 );
    }
}


bool JobListView::isServerColumnVisible() const
{
    return columnWidth( ColumnServer );
}


void JobListView::setServerColumnVisible( bool visible )
{
    if ( visible == isServerColumnVisible() )
        return;

    if ( visible )
    {
        setColumnWidthMode( ColumnServer, Maximum );
        setColumnWidth( ColumnServer, 50 ); // at least the user can see it again
    }
    else
    {
        setColumnWidthMode( ColumnServer, Manual );
        setColumnWidth( ColumnServer, 0 );
    }
}


int JobListView::expireDuration() const
{
    return mExpireDuration;
}


void JobListView::setExpireDuration( int duration )
{
    mExpireDuration = duration;
}


void JobListView::clear()
{
    mExpireTimer->stop();

    mItems.clear();
    mFinishedJobs.clear();

    KListView::clear();
}


void JobListView::slotExpireFinishedJobs()
{
    const QTime currentTime = QTime::currentTime();

    // this list is sorted by the age of the finished jobs, the oldest is the first
    // so we've to find the first job which isn't old enough to expire
    FinishedJobs::iterator it = mFinishedJobs.begin();
    for ( const FinishedJobs::iterator itEnd = mFinishedJobs.end(); it != itEnd; ++it )
    {
        if ( ( *it ).first.secsTo( currentTime ) < mExpireDuration )
            break;

        removeItem( ( *it ).second );
    }

    mFinishedJobs.erase( mFinishedJobs.begin(), it );

    if ( mFinishedJobs.empty() )
        mExpireTimer->stop();
}


void JobListView::expireItem( JobListViewItem* item )
{
    if ( mExpireDuration == 0 )
    {
        removeItem( item );
    }
    else if ( mExpireDuration > 0 )
    {
        mFinishedJobs.push_back( FinishedJob( QTime::currentTime(), item ) );

        if ( !mExpireTimer->isActive() )
            mExpireTimer->start( 1000 );
    }
}


void JobListView::removeItem( JobListViewItem* item )
{
    mItems.remove( item->job().jobId() );
    delete item;
}


#include "joblistview.moc"

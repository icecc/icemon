/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
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
#include "ganttstatusview.h"
#include "job.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

GanttTimeScaleWidget::GanttTimeScaleWidget( QWidget *parent, const char *name )
	: QWidget( parent, name, WResizeNoErase | WRepaintNoErase ),
          mPixelsPerSecond( 40 )
{
}

void GanttTimeScaleWidget::setPixelsPerSecond( int v )
{
  mPixelsPerSecond = v;
}

void GanttTimeScaleWidget::paintEvent( QPaintEvent *pe )
{
	const QRect r = pe->rect();

	QPixmap buffer( r.size() );
	buffer.fill( paletteBackgroundColor() );

	QPainter p( &buffer );
	const QFontMetrics fm = p.fontMetrics();

	// Maybe the rectangle overlaps the right half of a number, check whether
	// that is the case and repaint that number if necessary.
	if ( r.x() % 100 != 0 ) {
		const int lastNumberXPos = r.x() - ( r.x() % 100 );
		QString lastNumber = QString::number( lastNumberXPos / 100 * 5 );
		if ( r.x() % 100 < p.fontMetrics().width( lastNumber ) ) {
			p.drawText( lastNumberXPos - r.x() + 2, fm.ascent(), lastNumber );
		}
	}

	// Now draw all the bars and numbers, very straightforward.
	for ( int x = 0; x < r.width(); ++x ) {
		const int absX = x + r.x();
		if ( absX % ( mPixelsPerSecond * 10 ) == 0 ) {
			p.drawLine( x, -r.y(), x, height() / 2 - r.y() );
			p.drawText( x + 2, fm.ascent() - r.y(),
                                    QString::number( absX /
                                                     mPixelsPerSecond ) );
		} else if ( absX % ( mPixelsPerSecond * 5 ) == 0 ) {
			p.drawLine( x, -r.y(), x, height() / 4 - r.y() );
			p.drawText( x + 2, fm.ascent() - r.y(),
                                    QString::number( absX /
                                                     mPixelsPerSecond ) );
		} else if ( absX % ( mPixelsPerSecond ) == 0 ) {
			p.drawLine( x, -r.y(), x, height() / 8 - r.y() );
		}
	}

	bitBlt( this, r.topLeft(), &buffer );
}

GanttProgress::GanttProgress( StatusView *statusView, QWidget *parent,
                              const char *name )
	: QWidget( parent, name, WResizeNoErase | WRepaintNoErase ),
          mStatusView( statusView ), mClock( 0 ), mIsFree( true )
{
}

void GanttProgress::progress()
{
    mClock += 1;
    adjustGraph();
    QWidget::update();
}

void GanttProgress::adjustGraph()
{
  // Remove non-visible jobs
  if ( m_jobs.count() >= 2 &&
       mClock - m_jobs[ m_jobs.count() - 2 ].clock > width() ) {
    m_jobs.remove( m_jobs.last() );
  }
}

void GanttProgress::update( const Job &job )
{
#if 0
    kdDebug() << "GanttProgress::update( job ): " << job.fileName() << endl;

    kdDebug() << "  num jobs: " << m_jobs.count() << endl;
    kdDebug() << "  first id: " << m_jobs.first().job.jobId() << endl;
    kdDebug() << "  this id: " << job.jobId() << endl;
#endif

    if ( !m_jobs.isEmpty() && m_jobs.first().job == job ) {
//       kdDebug() << "  Known Job. State: " << job.state() << endl;
        if ( job.state() == Job::Finished || job.state() == Job::Failed ) {
          Job j = IdleJob();
          m_jobs.prepend( JobData( j, mClock ) );
          mIsFree = true;
        }
    } else {
//        kdDebug() << " New Job" << endl;
        m_jobs.prepend( JobData( job, mClock ) );
        mIsFree = ( job.state() == Job::Idle );
    }

//    kdDebug() << "num jobs: " << m_jobs.count() << " jobs" << endl;
}

void GanttProgress::drawGraph( QPainter &p )
{
//    kdDebug() << "drawGraph() " << m_jobs.count() << " jobs" << endl;
    if( height() == 0 )
        return;

    bool lastBox = false;
    int xStart = 0;
    QValueList< JobData >::ConstIterator it = m_jobs.begin();
    for ( ; ( it != m_jobs.end() ) && !lastBox; ++it ) {
        int xEnd = mClock - (*it).clock;

        if ( xEnd > width() ) {
          xEnd = width();
          lastBox = true;
        }

        int xWidth = xEnd - xStart;
        if( xWidth == 0 )
            continue;

//        kdDebug() << "XStart: " << xStart << "  xWidth: " << xWidth << endl;

        // Draw the rectangle for the current job
        QColor color = colorForStatus( ( *it ).job );
        p.fillRect( xStart, 0, xWidth, height(), color );
        p.setPen( color.dark() );
        p.drawRect( xStart, 0, xWidth, height() );

        if( xWidth > 4 && height() > 4 ) {
            int width = xWidth - 4;
            QString s = ( *it ).job.fileName();
            if ( !s.isEmpty() ) {
                s = s.mid( s.findRev( '/' ) + 1, s.length() );
//              s = s.left( s.findRev( '.' ) );
//              s[0] = s[0].upper();
        // Optimization - cache the drawn text in a pixmap, and update the cache
        // only if the pixmap height doesn't match, if the pixmap width is too large,
        // or if the shortened text with another character added (next_text_width) would fit
                if( width >= (*it).next_text_width || width < (*it).text_cache.width()
                    || height() - 4 != (*it).text_cache.height()) {
                    // If we print the filename, check whether we need to truncate it and
                    // append "..." at the end.
                    int text_width = p.fontMetrics().width( s );
                    if( text_width > width ) {
                        int threeDotsWidth = p.fontMetrics().width( "..." );
                        int next_width = 0;
                        int newLength = 0;
                        for(;
                             next_width <= width;
                             ++newLength ) {
                            text_width = next_width;
                            next_width = p.fontMetrics().width( s.left( newLength ) ) +
                                threeDotsWidth;
                        }
                        (*it).next_text_width = next_width;
                        s  = s.left( newLength > 2 ? newLength - 2 : 0 ) + "...";
                    } else {
                        (*it).next_text_width = 1000000; // large number (no next width)
                    }
                    // Finally draw the text.
                    if( text_width > 0 ) {
                        (*it).text_cache.resize( text_width, height() - 4 );
                        (*it).text_cache.fill( color );
                        QPainter painter( &(*it).text_cache );
                        painter.drawText( 0, 0, text_width, height() - 4,
                                    Qt::AlignTop | Qt::AlignLeft, s );
                    }
                }
                if( !(*it).text_cache.isNull())
                    p.drawPixmap( xStart + 2, 2, (*it).text_cache );
            }
        }
        xStart = xEnd;
    }
}

QColor GanttProgress::colorForStatus( const Job &job ) const
{
    if ( job.state() == Job::Idle ) {
        return Qt::gray;
    } else {
        QColor c = mStatusView->hostColor( job.client() );
        if ( job.state() == Job::LocalOnly ) return c.light();
        else return c;
    }
}

void GanttProgress::paintEvent( QPaintEvent * )
{
    QPixmap buffer( width(), height() );
//    buffer.fill( Qt::yellow );
    buffer.fill( paletteBackgroundColor() );

    QPainter p( &buffer );
    drawGraph( p );

    bitBlt( this, 0, 0, &buffer );
}

void GanttProgress::resizeEvent( QResizeEvent * )
{
    adjustGraph();
}

GanttStatusView::GanttStatusView( QWidget *parent, const char *name )
	: QWidget( parent, name, WRepaintNoErase | WResizeNoErase )
{
    m_topLayout = new QGridLayout( this, 2, 2, 0, -1, "topLayout" );
    m_topLayout->setSpacing( 5 );
    m_topLayout->setMargin( 4 );
    m_topLayout->setColStretch( 1, 10 );

    GanttTimeScaleWidget *timeScale = new GanttTimeScaleWidget( this );
    timeScale->setFixedHeight( 50 );
    m_topLayout->addWidget( timeScale, 0, 1 );

    m_progressTimer = new QTimer( this );
    connect( m_progressTimer, SIGNAL( timeout() ), SLOT( updateGraphs() ) );
    m_ageTimer = new QTimer( this );
    connect( m_ageTimer, SIGNAL( timeout() ), SLOT( checkAge() ) );

    mUpdateInterval = 25;
    timeScale->setPixelsPerSecond( 1000 / mUpdateInterval );

    start();
}

void GanttStatusView::update( const Job &job )
{
    if ( !mRunning ) return;

    if ( job.state() == Job::WaitingForCS ) return;

#if 0
    kdDebug() << "GanttStatusView::update(): ID: " << job.jobId() << "  "
              << job.fileName() << "  Status:" << int( job.state() )
              << "  Server: " << job.server() << endl;
#endif

    QMap<unsigned int, GanttProgress *>::Iterator it;

    it = mJobMap.find( job.jobId() );

    if ( it != mJobMap.end() ) {
//        kdDebug() << "  Job found" << endl;
        it.data()->update( job );
        if ( job.state() == Job::Finished || job.state() == Job::Failed ) {
            mJobMap.remove( it );
        }
        return;
    }

    GanttProgress *slot = 0;

    unsigned int processor;
    if ( job.state() == Job::LocalOnly ) processor = job.client();
    else processor = job.server();

    if ( !processor ) {
      kdDebug() << "GanttStatusView::update(): processor for job "
                << job.jobId() << " is empty." << endl;
      return;
    }

    NodeMap::ConstIterator it2 = mNodeMap.find( processor );
    if ( it2 == mNodeMap.end() ) {
//        kdDebug() << "  Server not known" << endl;
        slot = registerNode( processor );
    } else {
        SlotList slotList = it2.data();
        SlotList::ConstIterator it3;
        for( it3 = slotList.begin(); it3 != slotList.end(); ++it3 ) {
            if ( (*it3)->isFree() ) {
//                kdDebug() << "  Found free slot" << endl;
                slot = *it3;
                break;
            }
        }
        if ( it3 == slotList.end() ) {
//            kdDebug() << "  Create new slot" << endl;
            slot = registerNode( processor );
        }
    }

    if ( !slot ) {
      kdError() << "GanttStatusView::update(): Unable to find slot" << endl;
    } else {
      mJobMap.insert( job.jobId(), slot );
      slot->update( job );
      mAgeMap[ processor ] = 0;
    }
}

QWidget * GanttStatusView::widget()
{
    return this;
}

void GanttStatusView::checkNode( unsigned int hostid, const QString &statmsg )
{
    StatusView::checkNode( hostid, statmsg );

    if ( !mRunning ) return;

    if ( mNodeMap.find( hostid ) == mNodeMap.end())
        registerNode( hostid )->update( IdleJob());
#warning TODO split statmsg somewhere global
    unsigned int max_kids = 1;
    for( unsigned int i = mNodeMap[ hostid ].count();
         i < max_kids;
         ++i )
        registerNode( hostid )->update( IdleJob());

    mAgeMap[ hostid ] = 0;

    SlotList slotList = mNodeMap[ hostid ]; // make a copy
    int to_remove = slotList.count() - max_kids;
    if( to_remove <= 0 )
        return;
    for( SlotList::Iterator it2 = slotList.fromLast();
         it2 != slotList.end();
         --it2 ) {
        if( (*it2)->isFree() && (*it2)->fullyIdle()) {
            removeSlot( hostid, *it2 );
            if( --to_remove == 0 )
                return;
        }
    }
}

GanttProgress *GanttStatusView::registerNode( unsigned int hostid )
{
//    kdDebug() << "GanttStatusView::registerNode(): " << ip << endl;

    static int lastRow = 0;

    QColor color = hostColor( hostid );

    QVBoxLayout *nodeLayout;

    NodeLayoutMap::ConstIterator it = mNodeLayouts.find( hostid );
    if ( it == mNodeLayouts.end() ) {
      ++lastRow;

      nodeLayout = new QVBoxLayout( 0, ( QString::number( hostid ) + "_layout" ).latin1() );
      m_topLayout->addLayout( nodeLayout, lastRow, 1 );
      mNodeLayouts.insert( hostid, nodeLayout );
      mNodeRows.insert( hostid, lastRow );
    } else {
      nodeLayout = it.data();
    }

    NodeRowMap::ConstIterator rowIt = mNodeRows.find( hostid );
    if ( rowIt == mNodeRows.end() ) {
      kdError() << "Unknown node row." << endl;
    } else {
      int row = *rowIt;
      NodeLabelMap::ConstIterator labelIt = mNodeLabels.find( hostid );
      if ( labelIt == mNodeLabels.end() ) {
        QString name = nameForHost( hostid );
        QLabel *l = new QLabel( name, this );
        l->setPaletteForegroundColor( color );
        m_topLayout->addWidget( l, row, 0 );
        l->show();
        mNodeLabels.insert( hostid, l );
      }
    }

    GanttProgress *w = new GanttProgress( this, this );
    nodeLayout->addWidget( w );

    mNodeMap[ hostid ].append( w );
    mAgeMap[ hostid ] = 0;

    m_topLayout->setRowStretch( mNodeRows[ hostid ], mNodeMap[ hostid ].size() );

    w->show();

    return w;
}

void GanttStatusView::removeSlot( unsigned int hostid, GanttProgress* slot )
{
    kdDebug() << "GanttStatusView::removeSlot(): " << hostid << endl;
    NodeLayoutMap::ConstIterator it = mNodeLayouts.find( hostid );
    if ( it == mNodeLayouts.end() )
        return;

    mNodeMap[ hostid ].remove( slot );
    JobMap newJobMap;
    for( QMap<unsigned int, GanttProgress *>::Iterator it = mJobMap.begin();
         it != mJobMap.end();  // QMap::remove doesn't return an iterator like
         ++it ) {              // e.g. in QValueList, and I'm not sure if 'it'
        if( (*it) != slot )    // or '++it' would be still valid, so let's copy
            newJobMap[ it.key() ] = *it; // still valid items to a new map
    }
    mJobMap = newJobMap;

    m_topLayout->setRowStretch( mNodeRows[ hostid ], mNodeMap[ hostid ].size() );
    delete slot;
}

void GanttStatusView::unregisterNode( unsigned int hostid )
{
    kdDebug() << "GanttStatusView::unregisterNode(): " << hostid << endl;
    NodeLayoutMap::ConstIterator it = mNodeLayouts.find( hostid );
    if ( it == mNodeLayouts.end() )
        return;
    while( !mNodeMap[ hostid ].isEmpty())
        removeSlot( hostid, mNodeMap[ hostid ].first());
    NodeLabelMap::Iterator labelIt = mNodeLabels.find( hostid );
    if ( labelIt != mNodeLabels.end() ) {
      delete *labelIt;
      mNodeLabels.remove( labelIt );
    }
    mAgeMap[ hostid ] = -1;
}

void GanttStatusView::updateGraphs()
{
    NodeMap::ConstIterator it;
    for ( it = mNodeMap.begin(); it != mNodeMap.end(); ++it ) {
        SlotList::ConstIterator it2;
        for( it2 = (*it).begin(); it2 != (*it).end(); ++it2 ) {
            (*it2)->progress();
        }
    }
}

void GanttStatusView::stop()
{
  mRunning = false;
  m_progressTimer->stop();
  m_ageTimer->stop();
}

void GanttStatusView::start()
{
  mRunning = true;
  m_progressTimer->start( mUpdateInterval );
  m_ageTimer->start( 10000 );
}

void GanttStatusView::checkNodes()
{
  checkAge();
}

void GanttStatusView::checkAge()
{
    QValueList<unsigned int> to_unregister;
    for( AgeMap::Iterator it = mAgeMap.begin();
         it != mAgeMap.end();
         ++it ) {
        if( *it > 1 )
            to_unregister.append( it.key() );
        else if( *it < 0 )
            ; // unregistered ones
        else
            ++(*it);
    }
    for( QValueList<unsigned int>::ConstIterator it = to_unregister.begin();
         it != to_unregister.end();
         ++it )
        unregisterNode( *it );
}

#include "ganttstatusview.moc"

#include "ganttstatusview.h"
#include "job.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <kdebug.h>

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

GanttProgress::GanttProgress( QMap<QString,QColor> &hostColors, QWidget *parent, const char *name )
	: QWidget( parent, name, WResizeNoErase | WRepaintNoErase ),
          mHostColors( hostColors ), mClock( 0 ), mIsFree( true )
{
}

void GanttProgress::setHostColors( QMap<QString,QColor> &v )
{
    mHostColors = v;
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
       mClock - m_jobs[ m_jobs.count() - 2 ].second > width() ) {
    m_jobs.remove( m_jobs.last() );
  }
}

void GanttProgress::update( const Job &job )
{
#if 0
    kdDebug() << "GanttProgress::update( job ): " << job.fileName() << endl;

    kdDebug() << "  num jobs: " << m_jobs.count() << endl;
    kdDebug() << "  first id: " << m_jobs.first().first.jobId() << endl;
    kdDebug() << "  this id: " << job.jobId() << endl;
#endif

    if ( !m_jobs.isEmpty() && m_jobs.first().first == job ) {
//       kdDebug() << "  Known Job. State: " << job.state() << endl;
        if ( job.state() == Job::Finished || job.state() == Job::Failed ) {
          Job j = IdleJob();
          m_jobs.prepend( qMakePair( j, mClock ) );
          mIsFree = true;
        }
    } else {
//        kdDebug() << " New Job" << endl;
        m_jobs.prepend( qMakePair( job, mClock ) );
        mIsFree = ( job.state() == Job::Idle );
    }

//    kdDebug() << "num jobs: " << m_jobs.count() << " jobs" << endl;
}

void GanttProgress::drawGraph( QPainter &p )
{
//    kdDebug() << "drawGraph() " << m_jobs.count() << " jobs" << endl;

    bool lastBox = false;
    int xStart = 0;
    QValueList< QPair<Job, int > >::ConstIterator it = m_jobs.begin();
    for ( ; ( it != m_jobs.end() ) && !lastBox; ++it ) {
        int xEnd = mClock - (*it).second;

        if ( xEnd > width() ) {
          xEnd = width();
          lastBox = true;
        }

        int xWidth = xEnd - xStart;

//        kdDebug() << "XStart: " << xStart << "  xWidth: " << xWidth << endl;

        // Draw the rectangle for the current job
        QColor color = colorForStatus( ( *it ).first );
        p.fillRect( xStart, 0, xWidth, height(), color );
        p.setPen( color.dark() );
        p.drawRect( xStart, 0, xWidth, height() );

        QString s = ( *it ).first.fileName();
        if ( !s.isEmpty() ) {
          s = s.mid( s.findRev( '/' ) + 1, s.length() );
//          s = s.left( s.findRev( '.' ) );
//          s[0] = s[0].upper();
        }

        // If we print the filename, check whether we need to truncate it and
        // append "..." at the end.
        if ( p.fontMetrics().width( s ) >= xWidth - 3 ) {
            int newLength = 0;
            int threeDotsWidth = p.fontMetrics().width( "..." );
            while ( p.fontMetrics().width( s.left( newLength ) ) +
                    threeDotsWidth < xWidth - 3 )
                ++newLength;
            s  = s.left( newLength - 1 ) + "...";
        }

        // Finally draw the text.
        p.drawText( xStart + 3, 3, xWidth - 3, height() - 3,
                    Qt::AlignTop | Qt::AlignLeft, s );

        xStart = xEnd;
    }
}

QColor GanttProgress::colorForStatus( const Job &job ) const
{
    if ( job.state() == Job::Idle ) {
        return Qt::gray;
    } else {
        QMap<QString,QColor>::ConstIterator it = mHostColors.find( job.client() );
        if ( it != mHostColors.end() ) {
            QColor c = it.data();
            if ( job.state() == Job::LocalOnly )
                return c.light();
            return c;
        }
        else return Qt::blue;
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

    QString processor;
    if ( job.state() == Job::LocalOnly ) processor = job.client();
    else processor = job.server();

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
      createHostColor( job.client() );
      slot->update( job );
      mAgeMap[ processor ] = 0;
    }
}

QWidget * GanttStatusView::widget()
{
    return this;
}

void GanttStatusView::checkNode( const QString &host, unsigned int max_kids )
{
    if ( !mRunning ) return;

    if ( mNodeMap.find( host ) == mNodeMap.end())
        registerNode( host )->update( IdleJob());
    for( unsigned int i = mNodeMap[ host ].count();
         i < max_kids;
         ++i )
        registerNode( host )->update( IdleJob());

    mAgeMap[ host ] = 0;

    SlotList slotList = mNodeMap[ host ]; // make a copy
    int to_remove = slotList.count() - max_kids;
    if( to_remove <= 0 )
        return;
    for( SlotList::Iterator it2 = slotList.fromLast();
         it2 != slotList.end();
         --it2 ) {
        if( (*it2)->isFree() && (*it2)->fullyIdle()) {
            removeSlot( host, *it2 );
            if( --to_remove == 0 )
                return;
        }
    }
}

GanttProgress *GanttStatusView::registerNode( const QString &name )
{
    kdDebug() << "GanttStatusView::registerNode(): " << name << endl;

    static int lastRow = 0;

    createHostColor( name );
    QColor color = mHostColors[ name ];

    QVBoxLayout *nodeLayout;

    NodeLayoutMap::ConstIterator it = mNodeLayouts.find( name );
    if ( it == mNodeLayouts.end() ) {
      ++lastRow;

      QLabel *l = new QLabel( name, this );
      l->setPaletteForegroundColor( color );
      m_topLayout->addWidget( l, lastRow, 0 );
      l->show();

      nodeLayout = new QVBoxLayout( 0, ( name + "_layout" ).latin1() );
      m_topLayout->addLayout( nodeLayout, lastRow, 1 );
      mNodeLayouts.insert( name, nodeLayout );
      mNodeRows.insert( name, lastRow );
    } else {
      nodeLayout = it.data();
    }

    GanttProgress *w = new GanttProgress( mHostColors, this );
    nodeLayout->addWidget( w );

    mNodeMap[ name ].append( w );
    mAgeMap[ name ] = 0;

    m_topLayout->setRowStretch( mNodeRows[ name ], mNodeMap[ name ].size() );

    w->show();

    return w;
}

void GanttStatusView::removeSlot( const QString& name, GanttProgress* slot )
{
    kdDebug() << "GanttStatusView::removeSlot(): " << name << endl;
    NodeLayoutMap::ConstIterator it = mNodeLayouts.find( name );
    if ( it == mNodeLayouts.end() )
        return;

    mNodeMap[ name ].remove( slot );
    JobMap newJobMap;
    for( QMap<unsigned int, GanttProgress *>::Iterator it = mJobMap.begin();
         it != mJobMap.end();  // QMap::remove doesn't return an iterator like
         ++it ) {              // e.g. in QValueList, and I'm not sure if 'it'
        if( (*it) != slot )    // or '++it' would be still valid, so let's copy
            newJobMap[ it.key() ] = *it; // still valid items to a new map
    }
    mJobMap = newJobMap;

    m_topLayout->setRowStretch( mNodeRows[ name ], mNodeMap[ name ].size() );
    delete slot;
}

void GanttStatusView::unregisterNode( const QString& name )
{
    kdDebug() << "GanttStatusView::unregisterNode(): " << name << endl;
    NodeLayoutMap::ConstIterator it = mNodeLayouts.find( name );
    if ( it == mNodeLayouts.end() )
        return;
    // rows cannot be removed from QGridLayout :-/ , but at least show no slots
    while( !mNodeMap[ name ].isEmpty())
        removeSlot( name, mNodeMap[ name ].first());
    mAgeMap[ name ] = -1;
}

void GanttStatusView::createHostColor( const QString &host )
{
  if ( mHostColors.find( host ) != mHostColors.end() ) return;

  static int num = 0;

  QColor color( num, 255 - num, ( num * 3 ) % 255 );

  mHostColors.insert( host, color );

  num += 48;
  num %= 255;
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
  m_ageTimer->start( 5000 ); 
}

void GanttStatusView::checkAge()
{
    QStringList to_unregister;
    for( AgeMap::Iterator it = mAgeMap.begin();
         it != mAgeMap.end();
         ++it ) {
        if( *it > 1 )
            to_unregister.append( it.key());
        else if( *it < 0 )
            ; // unregistered ones
        else
            ++(*it);
    }
    for( QStringList::ConstIterator it = to_unregister.begin();
         it != to_unregister.end();
         ++it )
        unregisterNode( *it );
}

#include "ganttstatusview.moc"

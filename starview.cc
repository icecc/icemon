/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
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
#include "starview.h"
#include "logging.h"

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <qcanvas.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qvaluelist.h>
#include <qtooltip.h>

#include <math.h>

using namespace std;

class HostItem : public QCanvasText
{
  public:
    HostItem( const QString &hostName, QCanvas *canvas )
      : QCanvasText( hostName, canvas ), m_hostName( hostName ),
        m_stateItem( 0 )
    {
      setZ( 100 );

      QRect r = boundingRect();
      mBaseWidth = r.width() + 10;
      mBaseHeight = r.height() + 10;

      m_boxItem = new QCanvasEllipse( mBaseWidth, mBaseHeight, canvas );
      m_boxItem->setZ( 80 );
      m_boxItem->move( r.width() / 2, r.height() / 2 );
      m_boxItem->show();

      m_jobHalo = new QCanvasEllipse( mBaseWidth, mBaseHeight, canvas );
      m_jobHalo->setZ( 70 );
      m_jobHalo->move( r.width() / 2, r.height() / 2 );
      m_jobHalo->show();

      setHostColor( QColor( 200, 200, 200 ) );
    }

    ~HostItem()
    {
    }

    void setHostColor( const QColor &color )
    {
      m_boxItem->setBrush( color );
      m_jobHalo->setBrush( color.light() );

      float luminance = ( color.red() * 0.299 ) + ( color.green() * 0.587 ) +
                        ( color.blue() * 0.114 );
      if ( luminance > 140.0 ) setColor( black );
      else setColor( white );
    }

    void setState( Job::State state ) { m_state = state; }
    Job::State state() const { return m_state; }

    void setStateItem( QCanvasItem *item ) { m_stateItem = item; }
    QCanvasItem *stateItem() { return m_stateItem; }

    void setClient( const QString &client ) { m_client = client; }
    QString client() const { return m_client; }

    QString hostName() const { return m_hostName; }

    void moveBy( double dx, double dy )
    {
      QCanvasText::moveBy( dx, dy );

      QRect r = boundingRect();

      m_boxItem->moveBy( dx, dy );
      m_jobHalo->moveBy( dx, dy );
    }

    void update( const Job &job )
    {
      setState( job.state() );
      setClient( job.client() );

      if ( job.state() == Job::WaitingForCS ) return;

      bool finished = job.state() == Job::Finished ||
                      job.state() == Job::Failed;

      JobList::Iterator it = m_jobs.find( job.jobId() );
      bool newJob = ( it == m_jobs.end() );

      if ( newJob && finished ) return;
      if ( !newJob && !finished ) return;

      if ( newJob ) m_jobs.insert( job.jobId(), job );
      else if ( finished ) m_jobs.remove( it );

      m_jobHalo->setSize( mBaseWidth + m_jobs.count() * 4,
                          mBaseHeight + m_jobs.count() * 4 );
    }

  private:
    Job::State m_state;
    QString m_hostName;
    QCanvasItem *m_stateItem;
    QString m_client;

    int mBaseWidth;
    int mBaseHeight;

    QCanvasEllipse *m_boxItem;

    QCanvasEllipse *m_jobHalo;

    JobList m_jobs;
};

class WhatsStat : public QToolTip {
public:
    WhatsStat( QWidget *widget )
        : QToolTip( widget ) {
        QMimeSourceFactory::defaultFactory()->setPixmap( "computer",
                                                         UserIcon("icemonnode") );
    }
    virtual void maybeTip ( const QPoint &p ) {
        tip( QRect( p.x() - 20, p.y() - 20, 40, 40 ),
             "<p><table><tr><td>"
             "<img source=\"computer\"><br>coolos</td><td>"
             "<table>"
             "<tr><td>Jobs:</td><td>7</td></tr>"
             "<tr><td>File:</td><td>/etc/nowhere</td></tr>"
             "</table></td></tr></table></p>" );
    }
};

StarView::StarView( QWidget *parent, const char *name )
  : QWidget( parent, name, WRepaintNoErase | WResizeNoErase )
{
    m_canvas = new QCanvas( this );
    m_canvas->resize( width(), height() );

    QHBoxLayout *layout = new QHBoxLayout( this );
    layout->setMargin( 0 );

    m_canvasView = new QCanvasView( m_canvas, this );
    m_canvasView->setVScrollBarMode( QScrollView::AlwaysOff );
    m_canvasView->setHScrollBarMode( QScrollView::AlwaysOff );
    layout->addWidget( m_canvasView );

    m_localhostItem = new HostItem( i18n( "Scheduler" ), m_canvas );
    centerLocalhostItem();
    m_localhostItem->show();

    m_canvas->update();

    new WhatsStat( this );

}

void StarView::update( const Job &job )
{
#if 0
  kdDebug() << "StarView::update() " << job.jobId()
            << " server: " << job.server() << " client: " << job.client()
            << " state: " << job.stateAsString() << endl;
#endif

  if ( job.state() == Job::WaitingForCS ) {
    drawNodeStatus();
    return;
  }

  bool finished = job.state() == Job::Finished || job.state() == Job::Failed;

  QMap<unsigned int,HostItem *>::Iterator it;
  it = mJobMap.find( job.jobId() );
  if ( it != mJobMap.end() ) {
    (*it)->update( job );
    if ( finished ) {
      mJobMap.remove( it );
    }
    return;
  }

  QString host = processor( job );
  if ( host.isEmpty() ) {
    kdDebug() << "Empty host" << endl;
    return;
  }
  HostItem *hostItem = m_hostItems.find( host );
  if ( !hostItem ) hostItem = createHostItem( host );

  hostItem->update( job );

  if ( !finished ) mJobMap.insert( job.jobId(), hostItem );

  drawNodeStatus();
}

QWidget *StarView::widget()
{
  return this;
}

void StarView::resizeEvent( QResizeEvent * )
{
    m_canvas->resize( width(), height() );
    centerLocalhostItem();
    arrangeHostItems();
    m_canvas->update();
}

void StarView::centerLocalhostItem()
{
    const QRect br = m_localhostItem->boundingRect();
    const int newX = ( width() - br.width() ) / 2;
    const int newY = ( height() - br.height() ) / 2;
    m_localhostItem->move( newX, newY );
}

void StarView::arrangeHostItems()
{
    const int radius = int( kMin( m_canvas->width() / 2.5,
                                  m_canvas->height() / 2.5 ) );
    const double step = 2 * M_PI / m_hostItems.count();

    double angle = 0.0;
    QDictIterator<HostItem> it( m_hostItems );
    while ( it.current() != 0 ) {
        it.current()->move( m_localhostItem->x() + ( cos( angle ) * radius ),
                            m_localhostItem->y() + ( sin( angle ) * radius ) );
        angle += step;
        ++it;
    }
}

QString StarView::processor( const Job &job )
{
  if ( job.state() == Job::LocalOnly || job.state() == Job::WaitingForCS ) {
    return job.client();
  } else {
    return job.server();
  }
}

HostItem *StarView::createHostItem( const QString &host )
{
  kdDebug() << "New node for '" << host << "'" << endl;

  HostItem *hostItem = new HostItem( nameForIp( host ), m_canvas );
  hostItem->setHostColor( hostColor( host ) );
  m_hostItems.insert( host, hostItem );
  hostItem->show();

  arrangeHostItems();

  m_canvas->update();

  return hostItem;
}

void StarView::drawNodeStatus()
{
    QDictIterator<HostItem> it( m_hostItems );
    for ( ; it.current() != 0; ++it )
        drawState( it.current() );
    m_canvas->update();
}

void StarView::drawState( HostItem *node )
{
    delete node->stateItem();
    QCanvasItem *newItem = 0;

    const QPoint nodeCenter = node->boundingRect().center();
    const QPoint localCenter = m_localhostItem->boundingRect().center();

    switch ( node->state() ) {
        case Job::Compiling: {
            QCanvasLine *line = new QCanvasLine( m_canvas );
            QColor color;
            QString client = node->client();
            if ( client.isEmpty() ) color = Qt::green;
            else color = hostColor( client );
            line->setPen( color );

            line->setPoints( nodeCenter.x(), nodeCenter.y(),
                             localCenter.x(), localCenter.y() );
            line->show();
            newItem = line;
            break;
        }
        case Job::WaitingForCS: {
            QCanvasLine *line = new QCanvasLine( m_canvas );
            line->setPen( QPen( Qt::darkGreen, 0, QPen::DashLine ) );

            line->setPoints( nodeCenter.x(), nodeCenter.y(),
                             localCenter.x(), localCenter.y() );
            line->show();
            newItem = line;
            break;
        }
        default:
            break;
    }

    node->setStateItem( newItem );
}

#include "starview.moc"

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

#include "hostinfo.h"

#include <services/logging.h>

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
    enum { RttiHostItem = 1000 };

    HostItem( const QString &text, QCanvas *canvas )
      : QCanvasText( text, canvas ), mHostInfo( 0 )
    {
      init();
    }

    HostItem( HostInfo *hostInfo, QCanvas *canvas )
      : QCanvasText( hostInfo->name(), canvas ), mHostInfo( hostInfo ),
        m_stateItem( 0 )
    {
      init();
    }

    ~HostItem()
    {
    }

    int rtti() const { return RttiHostItem; }

    HostInfo *hostInfo() const { return mHostInfo; }

    void setHostColor( const QColor &color )
    {
      m_boxItem->setBrush( color );
      m_jobHalo->setBrush( color.light() );

      float luminance = ( color.red() * 0.299 ) + ( color.green() * 0.587 ) +
                        ( color.blue() * 0.114 );
      if ( luminance > 140.0 ) setColor( black );
      else setColor( white );
    }

    void setIsActiveClient( bool active ) { mIsActiveClient = active; }
    bool isActiveClient() const { return mIsActiveClient; }
    
    void setIsCompiling( bool compiling ) { mIsCompiling = compiling; }
    bool isCompiling() const { return mIsCompiling; }

    void setStateItem( QCanvasItem *item ) { m_stateItem = item; }
    QCanvasItem *stateItem() { return m_stateItem; }

    void setClient( unsigned int client ) { m_client = client; }
    unsigned int client() const { return m_client; }

    QString hostName() const { return mHostInfo->name(); }

    void moveBy( double dx, double dy )
    {
      QCanvasText::moveBy( dx, dy );

      QRect r = boundingRect();

      m_boxItem->moveBy( dx, dy );
      m_jobHalo->moveBy( dx, dy );
    }

    void update( const Job &job )
    {
      setIsCompiling( job.state() == Job::Compiling );
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
    void init()
    {
      setZ( 100 );

      QRect r = boundingRect();
      mBaseWidth = r.width() + 10;
      mBaseHeight = r.height() + 10;

      m_boxItem = new QCanvasEllipse( mBaseWidth, mBaseHeight, canvas() );
      m_boxItem->setZ( 80 );
      m_boxItem->move( r.width() / 2, r.height() / 2 );
      m_boxItem->show();

      m_jobHalo = new QCanvasEllipse( mBaseWidth, mBaseHeight, canvas() );
      m_jobHalo->setZ( 70 );
      m_jobHalo->move( r.width() / 2, r.height() / 2 );
      m_jobHalo->show();

      setHostColor( QColor( 200, 200, 200 ) );

      mIsActiveClient = false;
      mIsCompiling = false;
    }

    HostInfo *mHostInfo;

    bool mIsActiveClient;
    bool mIsCompiling;
    
    QCanvasItem *m_stateItem;
    unsigned int m_client;

    int mBaseWidth;
    int mBaseHeight;

    QCanvasEllipse *m_boxItem;

    QCanvasEllipse *m_jobHalo;

    JobList m_jobs;
};

class WhatsStat : public QToolTip
{
  public:
    WhatsStat( QCanvas *canvas, QWidget *widget )
        : QToolTip( widget ), mCanvas( canvas )
    {
        QMimeSourceFactory::defaultFactory()->setPixmap( "computer",
                                                         UserIcon("icemonnode") );
    }

    virtual void maybeTip ( const QPoint &p )
    {
        HostItem *item = 0;
        QCanvasItemList items = mCanvas->collisions( p );
        QCanvasItemList::ConstIterator it;
        for( it = items.begin(); it != items.end(); ++it ) {
          if ( (*it)->rtti() == HostItem::RttiHostItem ) {
            item = static_cast<HostItem *>( *it );
            break;
          }
        }
        if ( item ) {
          HostInfo *hostInfo = item->hostInfo();
          if ( !hostInfo ) return;

          tip( QRect( p.x() - 20, p.y() - 20, 40, 40 ),
               "<p><table><tr><td>"
               "<img source=\"computer\"><br><b>" + item->hostName() +
               "</b><br>" +
               i18n("IP: %1").arg( hostInfo->ip() ) + "<br>" +
               i18n("Flavor: %1")
               .arg( HostInfo::colorName( hostInfo->color() ) ) +
               "</td><td>"
               "<table>"
               "<tr><td>Jobs:</td><td>7</td></tr>"
               "<tr><td>File:</td><td>/etc/nowhere</td></tr>"
               "</table></td></tr></table></p>" );
        }
    }

  private:
    QCanvas *mCanvas;
};

StarView::StarView( HostInfoManager *m, QWidget *parent, const char *name )
  : QWidget( parent, name, WRepaintNoErase | WResizeNoErase ), StatusView( m )
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

    createKnownHosts();

    m_canvas->update();

    new WhatsStat( m_canvas, this );
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
      unsigned int clientid = job.client();
      HostItem *clientItem = findHostItem( clientid );
      if ( clientItem ) clientItem->setIsActiveClient( false );
    }
    return;
  }

  unsigned int hostid = processor( job );
  if ( !hostid ) {
    kdDebug() << "Empty host" << endl;
    return;
  }

  HostItem *hostItem = findHostItem( hostid );
  if ( !hostItem ) hostItem = createHostItem( hostid );

  hostItem->update( job );

  if ( !finished ) mJobMap.insert( job.jobId(), hostItem );

  if ( job.state() == Job::Compiling ) {
    unsigned int clientid = job.client();
    HostItem *clientItem = findHostItem( clientid );
    if ( clientItem ) clientItem->setIsActiveClient( true );
  }

  drawNodeStatus();
}

HostItem *StarView::findHostItem( unsigned int hostid )
{
  HostItem *hostItem = 0;
  QMapIterator<unsigned int, HostItem*> it = m_hostItems.find( hostid );
  if ( it != m_hostItems.end() ) hostItem = it.data();
  return hostItem;
}

void StarView::checkNode( unsigned int hostid )
{
  if ( !hostid ) return;

  HostItem *hostItem = findHostItem( hostid );
  if ( !hostItem ) hostItem = createHostItem( hostid );
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
    const int xRadius = int( m_canvas->width() / 2.5 );
    const int yRadius = int( m_canvas->height() / 2.5 );

    const double step = 2 * M_PI / m_hostItems.count();

    double angle = 0.0;
    for ( QMap<unsigned int, HostItem*>::ConstIterator it = m_hostItems.begin(); it != m_hostItems.end(); ++it )
    {
        it.data()->move( m_localhostItem->x() + ( cos( angle ) * xRadius ),
                         m_localhostItem->y() + ( sin( angle ) * yRadius ) );
        angle += step;
    }
}

unsigned int StarView::processor( const Job &job )
{
  if ( job.state() == Job::LocalOnly || job.state() == Job::WaitingForCS ) {
    return job.client();
  } else {
    return job.server();
  }
}

HostItem *StarView::createHostItem( unsigned int hostid )
{
//  kdDebug() << "New node for '" << hostid << "'" << endl;

  HostItem *hostItem = new HostItem( hostInfoManager()->find( hostid ),
                                     m_canvas );
  hostItem->setHostColor( hostColor( hostid ) );
  m_hostItems.insert( hostid, hostItem );
  hostItem->show();

  arrangeHostItems();

  m_canvas->update();

  return hostItem;
}

void StarView::drawNodeStatus()
{
    for ( QMap<unsigned int, HostItem*>::ConstIterator it = m_hostItems.begin(); it != m_hostItems.end(); ++it )
        drawState( *it );
    m_canvas->update();
}

void StarView::drawState( HostItem *node )
{
    delete node->stateItem();
    QCanvasItem *newItem = 0;

    const QPoint nodeCenter = node->boundingRect().center();
    const QPoint localCenter = m_localhostItem->boundingRect().center();

    QColor color;
    unsigned int client = node->client();
    if ( !client ) color = Qt::green;
    else color = hostColor( client );

    if ( node->isCompiling() ) {
      QCanvasLine *line = new QCanvasLine( m_canvas );
      line->setPen( color );

      line->setPoints( nodeCenter.x(), nodeCenter.y(),
                       localCenter.x(), localCenter.y() );
      line->show();
      newItem = line;
    } else if ( node->isActiveClient() ) {
      QCanvasLine *line = new QCanvasLine( m_canvas );
      line->setPen( QPen( color, 0, QPen::DashLine ) );

      line->setPoints( nodeCenter.x(), nodeCenter.y(),
                       localCenter.x(), localCenter.y() );
      line->show();
      newItem = line;
    }

    node->setStateItem( newItem );
}

void StarView::createKnownHosts()
{
  HostInfoManager::HostMap hosts = hostInfoManager()->hostMap();

  HostInfoManager::HostMap::ConstIterator it;
  for( it = hosts.begin(); it != hosts.end(); ++it ) {
    unsigned int id = (*it)->id();
    if ( !findHostItem( id ) ) createHostItem( id );
  }
}

#include "starview.moc"

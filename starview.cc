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
#include <kdialog.h>

#include <qlayout.h>
#include <qtimer.h>
#include <qvaluelist.h>
#include <qtooltip.h>
#include <qslider.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <math.h>

using namespace std;

StarViewConfigDialog::StarViewConfigDialog( QWidget *parent )
  : QDialog( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n("Number of nodes per ring"), this );
  topLayout->addWidget( label );

  QBoxLayout *nodesLayout = new QHBoxLayout( topLayout );

  int nodesPerRing = 25;

  mNodesPerRingSlider = new QSlider( 1, 50, 1, nodesPerRing, Horizontal, this );
  nodesLayout->addWidget( mNodesPerRingSlider );
  connect( mNodesPerRingSlider, SIGNAL( valueChanged( int ) ),
           SIGNAL( configChanged() ) );
  connect( mNodesPerRingSlider, SIGNAL( valueChanged( int ) ),
           SLOT( slotNodesPerRingChanged( int ) ) );

  mNodesPerRingLabel = new QLabel( QString::number( nodesPerRing ), this );
  nodesLayout->addWidget( mNodesPerRingLabel );

  QFrame *hline = new QFrame( this );
  hline->setFrameShape( QFrame::HLine );
  topLayout->addWidget( hline );

  QBoxLayout *buttonLayout = new QHBoxLayout( topLayout );

  buttonLayout->addStretch( 1 );

  QPushButton *button = new QPushButton( i18n("Close"), this );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( hide() ) );
}

void StarViewConfigDialog::slotNodesPerRingChanged( int nodes )
{
  mNodesPerRingLabel->setText( QString::number( nodes ) );
}

void StarViewConfigDialog::setMaxNodes( int maxNodes )
{
  mNodesPerRingSlider->setMaxValue( maxNodes + 1 );
}

int StarViewConfigDialog::nodesPerRing()
{
  return mNodesPerRingSlider->value();
}

HostItem::HostItem( const QString &text, QCanvas *canvas )
  : QCanvasText( text, canvas ), mHostInfo( 0 ), m_stateItem( 0 )
{
  init();
}

HostItem::HostItem( HostInfo *hostInfo, QCanvas *canvas )
  : QCanvasText( hostInfo->name(), canvas ), mHostInfo( hostInfo ),
    m_stateItem( 0 )
{
  init();
}

HostItem::~HostItem()
{
}

void HostItem::deleteSubItems()
{
  delete m_boxItem;
  delete m_jobHalo;
  delete m_stateItem;
}

void HostItem::setHostColor( const QColor &color )
{
  m_boxItem->setBrush( color );
  m_jobHalo->setBrush( color.light() );

  setColor( StatusView::textColor( color ) );
}

QString HostItem::hostName() const
{
  return mHostInfo->name();
}

void HostItem::moveBy( double dx, double dy )
{
  QCanvasText::moveBy( dx, dy );

  QRect r = boundingRect();

  m_boxItem->moveBy( dx, dy );
  m_jobHalo->moveBy( dx, dy );
}

void HostItem::update( const Job &job )
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

void HostItem::init()
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

               "<table>" +
               "<tr><td>" + i18n("IP:") + "</td><td>" + hostInfo->ip()
               + "</td></tr>" +
               "<tr><td>" + i18n("Platform:") + "</td><td>" +
               hostInfo->platform() + "</td></tr>"
               "<tr><td>" + i18n("Flavor:") + "</td><td>" +
               HostInfo::colorName( hostInfo->color() ) + "</td></tr>" +
               "<tr><td>" + i18n("Id:") + "</td><td>" +
               QString::number( hostInfo->id() ) + "</td></tr>" +
               "<tr><td>" + i18n("Speed:") + "</td><td>" +
               QString::number( hostInfo->serverSpeed() ) + "</td></tr>" +
               "</table>"

               "</td></tr></table></p>" );
        }
    }

  private:
    QCanvas *mCanvas;
};

StarView::StarView( HostInfoManager *m, QWidget *parent, const char *name )
  : QWidget( parent, name, WRepaintNoErase | WResizeNoErase ), StatusView( m )
{
    mConfigDialog = new StarViewConfigDialog( this );
    connect( mConfigDialog, SIGNAL( configChanged() ),
             SLOT( slotConfigChanged() ) );

    m_canvas = new QCanvas( this );
    m_canvas->resize( width(), height() );

    QHBoxLayout *layout = new QHBoxLayout( this );
    layout->setMargin( 0 );

    m_canvasView = new QCanvasView( m_canvas, this );
    m_canvasView->setVScrollBarMode( QScrollView::AlwaysOff );
    m_canvasView->setHScrollBarMode( QScrollView::AlwaysOff );
    layout->addWidget( m_canvasView );

    m_schedulerItem = new HostItem( "", m_canvas );
    centerSchedulerItem();
    m_schedulerItem->show();

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
    m_canvas->update();
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
    drawNodeStatus();
    m_canvas->update();
    return;
  }

  unsigned int hostid = processor( job );
  if ( !hostid ) {
    kdDebug() << "Empty host" << endl;
    return;
  }

  HostItem *hostItem = findHostItem( hostid );
  if ( !hostItem ) return;

  hostItem->update( job );

  if ( !finished ) mJobMap.insert( job.jobId(), hostItem );

  if ( job.state() == Job::Compiling ) {
    unsigned int clientid = job.client();
    HostItem *clientItem = findHostItem( clientid );
    if ( clientItem ) clientItem->setIsActiveClient( true );
  }

  drawNodeStatus();
  m_canvas->update();
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
//  kdDebug() << "StarView::checkNode() " << hostid << endl;

  if ( !hostid ) return;

  HostItem *hostItem = findHostItem( hostid );
  if ( !hostItem ) {
    hostItem = createHostItem( hostid );
    arrangeHostItems();
    drawNodeStatus();
  }
}

void StarView::removeNode( unsigned int hostid )
{
//  kdDebug() << "StarView::removeNode() " << hostid << endl;

  HostItem *hostItem = findHostItem( hostid );

  if ( hostItem && hostItem->hostInfo()->isOffline() ) {
#if 0
    kdDebug() << "StarView::removeNode() " << hostid << " ("
              << int( hostItem ) << ")" << endl;
#endif

    m_hostItems.remove( hostid );

    QValueList<unsigned int> obsoleteJobs;

    QMap<unsigned int,HostItem *>::Iterator it;
    for( it = mJobMap.begin(); it != mJobMap.end(); ++it ) {
#if 0
      kdDebug() << " JOB: " << it.key() << " (" << int( it.data() )
                << ")" << endl;
#endif
      if ( it.data() == hostItem ) {
#if 0
        kdDebug() << " Delete Job " << it.key() << endl;
#endif
        obsoleteJobs.append( it.key() );
      }
    }

    QValueList<unsigned int>::ConstIterator it2;
    for( it2 = obsoleteJobs.begin(); it2 != obsoleteJobs.end(); ++it2 ) {
      mJobMap.remove( *it2 );
    }

    hostItem->deleteSubItems();
    delete hostItem;

    arrangeHostItems();
    drawNodeStatus();

    m_canvas->update();
  }
}

void StarView::updateSchedulerState( bool online )
{
  QString txt;
  if ( online ) {
    txt = i18n("Scheduler");
  } else {
    txt = "";
  }
  m_schedulerItem->deleteSubItems();
  delete m_schedulerItem;

  if ( !online ) {
    QMap<unsigned int,HostItem *>::ConstIterator it;
    for( it = m_hostItems.begin(); it != m_hostItems.end(); ++it ) {
      (*it)->deleteSubItems();
      delete *it;
    }
    m_hostItems.clear();
    mJobMap.clear();
  }

  m_schedulerItem = new HostItem( txt, m_canvas );
  m_schedulerItem->show();
  centerSchedulerItem();
  m_canvas->update();
}

QWidget *StarView::widget()
{
  return this;
}

void StarView::resizeEvent( QResizeEvent * )
{
    m_canvas->resize( width(), height() );
    centerSchedulerItem();
    arrangeHostItems();
    drawNodeStatus();
    m_canvas->update();
}

void StarView::centerSchedulerItem()
{
    const QRect br = m_schedulerItem->boundingRect();
    const int newX = ( width() - br.width() ) / 2;
    const int newY = ( height() - br.height() ) / 2;
    m_schedulerItem->move( newX, newY );
}

void StarView::slotConfigChanged()
{
  arrangeHostItems();
  drawNodeStatus();
  m_canvas->update();
}

void StarView::arrangeHostItems()
{
//  kdDebug() << "StarView::arrangeHostItems()" << endl;

  int count = m_hostItems.count();

//  kdDebug() << "  Count: " << count << endl;

  int nodesPerRing = mConfigDialog->nodesPerRing();

  int ringCount = int( count / nodesPerRing ) + 1;

//  kdDebug() << "  Rings: " << ringCount << endl;

  const int xRadius = int( m_canvas->width() / 2.5 );
  const int yRadius = int( m_canvas->height() / 2.5 );

  const double step = 2 * M_PI / count;

  double angle = 0.0;
  int i = 0;
  QMap<unsigned int, HostItem*>::ConstIterator it;
  for ( it = m_hostItems.begin(); it != m_hostItems.end(); ++it ) {
    float factor = 1 - ( 1.0 / ( ringCount + 1 ) ) * ( i % ringCount );

    int xr = int( xRadius * factor );
    int yr = int( yRadius * factor );

    HostItem *item = it.data();

    QRect rect = item->boundingRect();
    int xOffset = rect.width() / 2;
    int yOffset = rect.height() / 2;

    item->move( width() / 2 + ( cos( angle ) * xr ) - xOffset,
                height() / 2 + ( sin( angle ) * yr ) - yOffset );

    angle += step;
    ++i;
  }
}

HostItem *StarView::createHostItem( unsigned int hostid )
{
  HostInfo *i = hostInfoManager()->find( hostid );

//  kdDebug() << "New node for " << hostid << " (" << i->name() << ")" << endl;

  assert( !i->name().isEmpty() );

  HostItem *hostItem = new HostItem( i, m_canvas );
  hostItem->setHostColor( hostColor( hostid ) );
  m_hostItems.insert( hostid, hostItem );
  hostItem->show();

  arrangeHostItems();

  m_canvas->update();

  if ( m_hostItems.count() > 25 ) {
    mConfigDialog->setMaxNodes( m_hostItems.count() );
  }

  return hostItem;
}

void StarView::drawNodeStatus()
{
  QMap<unsigned int, HostItem*>::ConstIterator it;
  for ( it = m_hostItems.begin(); it != m_hostItems.end(); ++it ) {
    drawState( *it );
  }
}

void StarView::drawState( HostItem *node )
{
    delete node->stateItem();
    QCanvasItem *newItem = 0;

    const QPoint nodeCenter = node->boundingRect().center();
    const QPoint localCenter = m_schedulerItem->boundingRect().center();

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

void StarView::configureView()
{
  mConfigDialog->show();
  mConfigDialog->raise();
}

#include "starview.moc"

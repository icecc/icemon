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

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kdialog.h>

#include <qlayout.h>
#include <qtooltip.h>
#include <qslider.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qregexp.h>
#include <qcheckbox.h>
#include <QResizeEvent>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <math.h>

static bool suppressDomain = false;

StarViewConfigDialog::StarViewConfigDialog( QWidget *parent )
  : QDialog( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n("Number of nodes per ring:"), this );
  topLayout->addWidget( label );

  QBoxLayout *nodesLayout = new QHBoxLayout();
  topLayout->addLayout( nodesLayout );

  int nodesPerRing = 25;

  mNodesPerRingSlider = new QSlider( Qt::Horizontal, this );
  mNodesPerRingSlider->setMinimum( 1 );
  mNodesPerRingSlider->setMaximum( 50 );
  mNodesPerRingSlider->setSingleStep( 1 );
  mNodesPerRingSlider->setValue( nodesPerRing );
  nodesLayout->addWidget( mNodesPerRingSlider );
  connect( mNodesPerRingSlider, SIGNAL( valueChanged( int ) ),
           SIGNAL( configChanged() ) );
  connect( mNodesPerRingSlider, SIGNAL( valueChanged( int ) ),
           SLOT( slotNodesPerRingChanged( int ) ) );

  mNodesPerRingLabel = new QLabel( QString::number( nodesPerRing ), this );
  nodesLayout->addWidget( mNodesPerRingLabel );

  label = new QLabel( i18n("Architecture filter:"), this );
  topLayout->addWidget( label );
  mArchFilterEdit = new QLineEdit( this );
  topLayout->addWidget( mArchFilterEdit );
  connect( mArchFilterEdit, SIGNAL( textChanged( const QString & ) ),
           SIGNAL( configChanged() ) );

  mSuppressDomainName = new QCheckBox( i18n("Suppress domain name"), this);
  topLayout->addWidget( mSuppressDomainName );
  connect( mSuppressDomainName, SIGNAL( toggled ( bool ) ),
           SLOT( slotSuppressDomainName ( bool ) ) );

  QFrame *hline = new QFrame( this );
  hline->setFrameShape( QFrame::HLine );
  topLayout->addWidget( hline );

  QBoxLayout *buttonLayout = new QHBoxLayout();
  topLayout->addLayout( buttonLayout );

  buttonLayout->addStretch( 1 );

  QPushButton *button = new QPushButton( i18n("&Close"), this );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( hide() ) );
}

void StarViewConfigDialog::slotNodesPerRingChanged( int nodes )
{
  mNodesPerRingLabel->setText( QString::number( nodes ) );
}

void StarViewConfigDialog::setMaxNodes( int maxNodes )
{
  mNodesPerRingSlider->setMaximum( maxNodes + 1 );
}

int StarViewConfigDialog::nodesPerRing()
{
  return mNodesPerRingSlider->value();
}

QString StarViewConfigDialog::archFilter()
{
  return mArchFilterEdit->text();
}

void StarViewConfigDialog::slotSuppressDomainName( bool b )
{
  suppressDomain = b;
  configChanged();
}


HostItem::HostItem( const QString &text, QGraphicsScene *canvas, HostInfoManager *m )
  : QGraphicsItemGroup( 0, canvas ), mHostInfo( 0 ), mHostInfoManager( m ),
    m_stateItem( 0 )
{
  init();

  m_textItem->setText(text);

  updateName();
}

HostItem::HostItem( HostInfo *hostInfo, QGraphicsScene *canvas, HostInfoManager *m )
  : QGraphicsItemGroup( 0, canvas ), mHostInfo( hostInfo ),
    mHostInfoManager( m ), m_stateItem( 0 )
{
  init();
}

HostItem::~HostItem()
{
}

void HostItem::init()
{
  mBaseWidth = 0;
  mBaseHeight = 0;

  m_boxItem = new QGraphicsEllipseItem( this, scene() );
  m_boxItem->setZValue( 80 );

  m_textItem = new QGraphicsSimpleTextItem( this, scene() );
  m_textItem->setZValue( 100 );

  setHostColor( QColor( 200, 200, 200 ) );

  mIsActiveClient = false;
  mIsCompiling = false;

  m_client = 0;
}

void HostItem::setHostColor( const QColor &color )
{
  m_boxItem->setBrush( color );

  m_textItem->setBrush( StatusView::textColor( color ) );
}

QString HostItem::hostName() const
{
  return mHostInfo->name();
}

void HostItem::updateName()
{
  if (mHostInfo) {
      QString s = mHostInfo->name();
      if (suppressDomain) {
          int l = s.indexOf('.');
          if (l>0)
              s.truncate(l);
      }
      m_textItem->setText(s);
  }

  QRectF r = m_textItem->boundingRect();
  mBaseWidth = r.width() + 10 ;
  mBaseHeight = r.height() + 10 ;

  m_boxItem->setRect(-5, -5, mBaseWidth, mBaseHeight);

  updateHalos();
}

void HostItem::setCenterPos( double x, double y )
{
    // move all items (also the sub items)
    setPos( x - m_textItem->boundingRect().width()/2, y - m_textItem->boundingRect().height()/2 );
  //  setPos( x, y );
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

  if ( newJob ) {
    m_jobs.insert( job.jobId(), job );
    createJobHalo( job );
  } else if ( finished ) {
    deleteJobHalo( job );
    m_jobs.erase( it );
  }
}

void HostItem::createJobHalo( const Job &job )
{
  QGraphicsEllipseItem *halo = new QGraphicsEllipseItem(
      centerPosX(), centerPosY(), mBaseWidth, mBaseHeight,
                                             this, scene() );
  halo->setZValue( 70 - m_jobHalos.size() );
  halo->setPen(QPen(Qt::NoPen));
  halo->show();

  m_jobHalos.insert( job, halo );

  updateHalos();
}

void HostItem::deleteJobHalo( const Job &job )
{
  QMap<Job,QGraphicsEllipseItem*>::Iterator it = m_jobHalos.find( job );
  if ( it == m_jobHalos.end() ) return;

  QGraphicsEllipseItem *halo = *it;
  delete halo;
  m_jobHalos.erase( it );

  updateHalos();
}

void HostItem::updateHalos()
{
  int count = 1;

  QMap<Job,QGraphicsEllipseItem*>::Iterator it;
  for( it = m_jobHalos.begin(); it != m_jobHalos.end(); ++it ) {
    QGraphicsEllipseItem *halo = it.value();
    halo->setRect( halo->x() - 5 - count * 3, halo->y() - 5 - count * 3, mBaseWidth + count * 6, mBaseHeight + count * 6 );
    halo->setBrush( mHostInfoManager->hostColor( it.key().client() ) );
    ++count;
  }
}

StarView::StarView( HostInfoManager *m, QWidget *parent, const char *name )
  : QWidget( parent ), StatusView( m )
{
    setObjectName( name );
    mConfigDialog = new StarViewConfigDialog( this );
    connect( mConfigDialog, SIGNAL( configChanged() ),
             SLOT( slotConfigChanged() ) );

    m_canvas = new QGraphicsScene ( this );
    m_canvas->setSceneRect( 0, 0, width(), height() );

    QHBoxLayout *layout = new QHBoxLayout( this );
    layout->setMargin( 0 );

    m_canvasView = new QGraphicsView( m_canvas, this );
    m_canvasView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_canvasView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_canvasView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    layout->addWidget( m_canvasView );

    m_schedulerItem = new HostItem( "", m_canvas, hostInfoManager() );
    m_schedulerItem->setZValue(150);
    centerSchedulerItem();
    m_schedulerItem->show();

    createKnownHosts();
}

void StarView::update( const Job &job )
{
#if 0
  kDebug() << "StarView::update() " << job.jobId()
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
      mJobMap.erase( it );
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
    kDebug() << "Empty host" << endl;
    return;
  }

  HostItem *hostItem = findHostItem( hostid );
  if ( !hostItem ) return;

  hostItem->update( job );

  if ( !finished ) mJobMap.insert( job.jobId(), hostItem );

  if ( job.state() == Job::Compiling ) {
    unsigned int clientid = job.client();
    HostItem *clientItem = findHostItem( clientid );
    if ( clientItem ) {
      clientItem->setClient( clientid );
      clientItem->setIsActiveClient( true );
    }
  }

  drawNodeStatus();
  m_canvas->update();
}

HostItem *StarView::findHostItem( unsigned int hostid )
{
  HostItem *hostItem = 0;
  QMap<unsigned int, HostItem*>::iterator it = m_hostItems.find( hostid );
  if ( it != m_hostItems.end() ) hostItem = it.value();
  return hostItem;
}

void StarView::checkNode( unsigned int hostid )
{
//  kDebug() << "StarView::checkNode() " << hostid << endl;

  if ( !hostid ) return;

  if ( !filterArch( hostid ) ) return;

  HostItem *hostItem = findHostItem( hostid );
  if ( !hostItem ) {
    hostItem = createHostItem( hostid );
    arrangeHostItems();
    drawNodeStatus();
  }
}

void StarView::removeNode( unsigned int hostid )
{
//  kDebug() << "StarView::removeNode() " << hostid << endl;

  HostItem *hostItem = findHostItem( hostid );

  if ( hostItem && hostItem->hostInfo()->isOffline() ) {
    removeItem( hostItem );
  }
}

void StarView::forceRemoveNode( unsigned int hostid )
{
  HostItem *hostItem = findHostItem( hostid );

  if ( hostItem ) {
    removeItem( hostItem );
  }
}

void StarView::removeItem( HostItem *hostItem )
{
#if 0
  kDebug() << "StarView::removeItem() " << hostid << " ("
            << int( hostItem ) << ")" << endl;
#endif

  m_hostItems.remove( hostItem->hostInfo()->id() );

  QList<unsigned int> obsoleteJobs;

  QMap<unsigned int,HostItem *>::Iterator it;
  for( it = mJobMap.begin(); it != mJobMap.end(); ++it ) {
#if 0
    kDebug() << " JOB: " << it.key() << " (" << int( it.value() )
              << ")" << endl;
#endif
    if ( it.value() == hostItem ) {
#if 0
      kDebug() << " Delete Job " << it.key() << endl;
#endif
      obsoleteJobs.append( it.key() );
    }
  }

  QList<unsigned int>::ConstIterator it2;
  for( it2 = obsoleteJobs.begin(); it2 != obsoleteJobs.end(); ++it2 ) {
    mJobMap.remove( *it2 );
  }

  delete hostItem;

  arrangeHostItems();
  drawNodeStatus();

  m_canvas->update();
}

void StarView::updateSchedulerState( bool online )
{
  QString txt;
  if ( online ) {
    txt = i18n("Scheduler");
  } else {
    txt = "";
  }
  delete m_schedulerItem;

  if ( !online ) {
    QMap<unsigned int,HostItem *>::ConstIterator it;
    for( it = m_hostItems.begin(); it != m_hostItems.end(); ++it ) {
      delete *it;
    }
    m_hostItems.clear();
    mJobMap.clear();
  }

  m_schedulerItem = new HostItem( txt, m_canvas, hostInfoManager() );
  m_schedulerItem->setZValue(100);
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
    m_canvas->setSceneRect( 0, 0, width(), height() );
    centerSchedulerItem();
    arrangeHostItems();
    drawNodeStatus();
    m_canvas->update();
}

bool StarView::event ( QEvent* e )
{
    if (e->type() != QEvent::ToolTip) return QWidget::event(e);

    QPoint p ( static_cast<QHelpEvent*>(e)->pos());

    HostItem *item = 0;
    if ( QGraphicsItem* graphicsItem = m_canvasView->itemAt( p ) )
        item = dynamic_cast<HostItem*>( graphicsItem->parentItem() );
    if ( item ) {
        HostInfo *hostInfo = item->hostInfo();
        if ( !hostInfo ) return QWidget::event(e);

        QPoint gp( static_cast<QHelpEvent*>(e)->globalPos());
        QToolTip::showText(gp+QPoint(10,10),
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

                "</td></tr></table></p>", this );
    }
    return QWidget::event(e);
}

void StarView::centerSchedulerItem()
{
    m_schedulerItem->setCenterPos( width() / 2, height() / 2 );
}

void StarView::slotConfigChanged()
{
//  kDebug() << "StarView::slotConfigChanged()" << endl;

  HostInfoManager::HostMap hostMap = hostInfoManager()->hostMap();
  HostInfoManager::HostMap::ConstIterator it;
  for( it = hostMap.begin(); it != hostMap.end(); ++it ) {
    if ( filterArch( *it ) ) checkNode( it.key() );
    else forceRemoveNode( it.key() );
  }

  arrangeHostItems();
  drawNodeStatus();
  m_canvas->update();
}

void StarView::arrangeHostItems()
{
//  kDebug() << "StarView::arrangeHostItems()" << endl;

  int count = m_hostItems.count();

//  kDebug() << "  Count: " << count << endl;

  int nodesPerRing = mConfigDialog->nodesPerRing();

  int ringCount = int( count / nodesPerRing ) + 1;

//  kDebug() << "  Rings: " << ringCount << endl;
  double radiusFactor = 2.5;
  if (suppressDomain) radiusFactor = 4;
  const int xRadius = qRound( m_canvas->width() / radiusFactor );
  const int yRadius = qRound( m_canvas->height() / radiusFactor );

  const double step = 2 * M_PI / count;

  double angle = 0.0;
  int i = 0;
  QMap<unsigned int, HostItem*>::ConstIterator it;
  for ( it = m_hostItems.begin(); it != m_hostItems.end(); ++it ) {
    double factor = 1 - ( 1.0 / ( ringCount + 1 ) ) * ( i % ringCount );

    double xr = xRadius * factor;
    double yr = yRadius * factor;

    HostItem *item = it.value();

    item->updateName();

    item->setCenterPos( width() / 2 + cos( angle ) * xr,
                        height() / 2 + sin( angle ) * yr );

    angle += step;
    ++i;
  }

  m_canvas->update();
}

HostItem *StarView::createHostItem( unsigned int hostid )
{
  HostInfo *i = hostInfoManager()->find( hostid );

  if ( !i || i->isOffline() || i->name().isEmpty() )
    return 0;

//  kDebug() << "New node for " << hostid << " (" << i->name() << ")" << endl;

  //assert( !i->name().isEmpty() );

  HostItem *hostItem = new HostItem( i, m_canvas, hostInfoManager() );
  hostItem->setHostColor( hostColor( hostid ) );
  m_hostItems.insert( hostid, hostItem );
  hostItem->show();

  arrangeHostItems();

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
    QGraphicsLineItem *newItem = 0;

    QColor color;
    unsigned int client = node->client();
    if ( !client ) color = Qt::green;
    else color = hostColor( client );

    if ( node->isCompiling() ) {
      newItem = new QGraphicsLineItem( node, m_canvas );
      newItem->setPen( color );
    } else if ( node->isActiveClient() ) {
      newItem = new QGraphicsLineItem( node, m_canvas );
      newItem->setPen( QPen( color, 2, Qt::DashLine ) );
    }

    if ( newItem ) {
      newItem->setLine( node->relativeCenterPosX(), node->relativeCenterPosY(), m_schedulerItem->centerPosX() - node->pos().x(),
                        m_schedulerItem->centerPosY() - node->pos().y() );
      newItem->setZValue(0);
      newItem->show();
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

  m_canvas->update();
}

void StarView::configureView()
{
  mConfigDialog->show();
  mConfigDialog->raise();
}

bool StarView::filterArch( unsigned int hostid )
{
  HostInfo *i = hostInfoManager()->find( hostid );
  if ( !i ) {
    kError() << "No HostInfo for id " << hostid << endl;
    return false;
  }

  return filterArch( i );
}

bool StarView::filterArch( HostInfo *i )
{
  if ( mConfigDialog->archFilter().isEmpty() ) return true;

  QRegExp regExp( mConfigDialog->archFilter() );

  if ( regExp.indexIn( i->platform() ) >= 0 ) {
    return true;
  }

  return false;
}

#include "starview.moc"

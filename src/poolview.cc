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
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "poolview.h"

#include "hostinfo.h"

#include <qdebug.h>

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
#include <QTimer>
#include <QApplication>
#include <QHostInfo>

#include <math.h>


static const qreal FRICTION_FACTOR = 0.05;
static const qreal MAX_VELOCITY = 10.;
static const qreal MIN_VELOCITY_FOR_BOUNCE = 0.1;

static bool suppressDomain = false;
static bool showJobLines = false;
static bool clientsAttractHosts = false;

PoolItem::PoolItem( const QString &text, PoolView *poolView, HostInfoManager *m )
  : QGraphicsItemGroup( 0, poolView->canvas() ), mHostInfo( 0 ), mHostInfoManager( m ),
    m_stateItem( 0 ), mX( 0 ), mY( 0 ), m_poolView( poolView )
{
    init();

    m_textItem->setText(text);

    updateName();
}

PoolItem::PoolItem( HostInfo *hostInfo, PoolView *poolView, HostInfoManager *m )
  : QGraphicsItemGroup( 0, poolView->canvas() ), mHostInfo( hostInfo ),
    mHostInfoManager( m ), m_stateItem( 0 ), m_poolView( poolView )
{
    init();
}

PoolItem::~PoolItem()
{
  foreach( QGraphicsLineItem *line, m_jobLines ) {
    if ( scene() )
      scene()->removeItem( line );
  }
}

void PoolItem::init()
{
  m_boxItem = new QGraphicsEllipseItem( this, scene() );
  m_boxItem->setZValue( 80 );
  m_boxItem->setPen( QPen( Qt::NoPen ) );
  setSize( m_poolView->poolItemWidth(), m_poolView->poolItemHeight() );
  m_textItem = new QGraphicsSimpleTextItem( 0, scene() );
  m_textItem->setZValue( 100 );

  addToGroup( m_boxItem );
  addToGroup( m_textItem );
  setHostColor( QColor( 200, 200, 200 ) );

  mIsActiveClient = false;
  mIsCompiling = false;

  m_client = 0;

  mVelocity = 0;
  mVelocityAngle = qrand() / (double)RAND_MAX * 2 * M_PI;

  if ( mHostInfo->name() == QHostInfo::localHostName() ) {
    QFont f;
    f.setBold( true );
    f.setUnderline( true );
    m_textItem->setFont( f );
    m_textItem->show();
    mIsLocalHost = true;
  }
  else {
    mIsLocalHost = false;
    m_textItem->hide();
  }
}

void PoolItem::setHostColor( const QColor &color )
{
    m_boxItem->setBrush( color );
}

QString PoolItem::hostName() const
{
    return mHostInfo->name();
}

void PoolItem::updateName()
{
    if (mHostInfo) {
        QString s = mHostInfo->name();
        if ( suppressDomain ) {
            int l = s.indexOf('.');
            if (l>0)
                s.truncate(l);
        }
	m_textItem->setText(s);
	m_textItem->setPos(  m_poolView->poolItemWidth() / 2. - m_textItem->boundingRect().width() / 2.,
			     m_poolView->poolItemHeight() / 2. - m_textItem->boundingRect().height() / 2. );
    }

}

double PoolItem::centerPosX() const
{
  return pos().x() + m_poolView->poolItemWidth() / 2.;
}

double PoolItem::centerPosY() const
{
  return pos().y() + m_poolView->poolItemHeight() / 2.;
}

void PoolItem::setCenterPos( double x, double y )
{
  // move all items (also the sub items)
  mX = x;
  mY = y;
  setPos( x - m_poolView->poolItemWidth() / 2., y - m_poolView->poolItemHeight() / 2.);
}

void PoolItem::setRandPos()
{
  int count = 0;
  qreal w = scene()->width();
  qreal h = scene()->height();
  qreal x,y;
  do {
    x =  qrand()/(double)RAND_MAX * w;
    y =  qrand()/(double)RAND_MAX * h;
    setCenterPos( x, y );
  } while( collidingItems().count() && ++count < 40 );

  show();
}

void PoolItem::update( const Job &job )
{
    setIsCompiling( job.state() == Job::Compiling );
    setClient( job.client() );

    if ( isCompiling() || isActiveClient() || job.state() == Job::WaitingForCS  ) {
      m_textItem->show();
      setHostColor( mHostInfoManager->hostColor( hostInfo()->id() ).darker(130) );
    }
    else {
      if ( !mIsLocalHost )
	m_textItem->hide();
      setHostColor( QColor( 200, 200, 200 ) );
    }


    if ( job.state() == Job::WaitingForCS ) {
      return;
    }

    bool finished = job.state() == Job::Finished ||
                    job.state() == Job::Failed;

    JobList::Iterator it = m_jobs.find( job.jobId() );
    bool newJob = ( it == m_jobs.end() );

    if ( newJob && finished ) {
      delete m_jobLines[ job.jobId() ];
      m_jobLines.remove( job.jobId() );
      return;
    }
    if ( !newJob && !finished ) return;

    if ( newJob )
    {
      m_jobs.insert( job.jobId(), job );
      m_jobLines[ job.jobId() ] = new QGraphicsLineItem( 0, scene() );
      m_jobLines[ job.jobId() ]->setZValue( 5 );
      if ( ! ::showJobLines )
	m_jobLines[ job.jobId()]->hide();
    } else if ( finished ) {
      m_jobs.remove( job.jobId() );
      delete m_jobLines[ job.jobId() ];
      m_jobLines.remove( job.jobId() );
    }
}

void PoolItem::setSize( qreal w, qreal h )
{
  m_boxItem->setRect( 0, 0, w, h );
}

void PoolItem::checkBorders()
{
  if ( centerPosX() - m_poolView->poolItemWidth() / 2. < 0 ) {
    mVelocityAngle = M_PI - mVelocityAngle;
    setCenterPos( m_poolView->poolItemWidth() / 2., centerPosY() );
  }

  if ( centerPosX() + m_poolView->poolItemWidth() / 2. > m_poolView->canvas()->width() ) {
    mVelocityAngle = M_PI - mVelocityAngle;
    setCenterPos( m_poolView->canvas()->width() - m_poolView->poolItemWidth() / 2., centerPosY() );
  }

  if ( centerPosY() - m_poolView->poolItemHeight() / 2. < 0 ) {
    mVelocityAngle = - mVelocityAngle;
    setCenterPos( centerPosX(), m_poolView->poolItemHeight() / 2. );
  }

  if ( centerPosY() + m_poolView->poolItemHeight() / 2. > m_poolView->canvas()->height() ) {
    mVelocityAngle = - mVelocityAngle;
    setCenterPos( centerPosX(), m_poolView->canvas()->height() - m_poolView->poolItemHeight() / 2.  );
  }
}

void PoolItem::checkCollision()
{
  //  return;

  // Taking in account that our shapes are circles, we can do much faster than
  // qt collision detection
  QList<PoolItem*> cItems;
  foreach( PoolItem *item, m_poolView->m_poolItems ) {
    qreal length = QLineF( QPointF( centerPosX(), centerPosY() ),
			   QPointF( item->centerPosX(), item->centerPosY() ) ).length();
    if ( length  < 2 * radius() && item != this )
      cItems << item;
  }

  // Compute bouncing
  foreach( PoolItem *item, cItems ) {

    // Bounce direction
    QLineF lineBetween(QPointF(mX, mY), QPointF(item->mX, item->mY));
    qreal angle = ::acos( lineBetween.dx() / lineBetween.length() );
    if ( lineBetween.dy() < 0 ) angle = 2*M_PI - angle;

    // Position correction
    qreal centerX = ( mX + item->mX ) / 2.;
    qreal centerY = ( mY + item->mY ) / 2.;
    mX = centerX - radius() * 1.01 * cos( angle );
    mY = centerY - radius() * 1.01 * sin( angle );
    setCenterPos( mX, mY );
    item->mX = centerX + item->radius()*1.01 * cos( angle );
    item->mY = centerY + item->radius()*1.01 * sin( angle );
    item->setCenterPos( item->mX, item->mY );


    if ( mVelocity > MIN_VELOCITY_FOR_BOUNCE ) {
      qreal vNormalThis = mVelocity * cos( mVelocityAngle - angle );
      qreal vTangentialThis = mVelocity * sin( mVelocityAngle - angle );
      qreal vNormalOther = item->mVelocity * cos( item->mVelocityAngle - angle );
      qreal vTangentialOther = item->mVelocity * sin( item->mVelocityAngle - angle );


      /* "Energy transfer" */
      qreal vNormal = sqrt( pow(vNormalThis, 2) + pow(vNormalOther, 2) ) / 2.;
      int sign = vNormalThis >= 0 ? 1 : -1;
      vNormalThis = - sign * vNormal;
      sign = vNormalOther >= 0 ? 1 : -1;
      vNormalOther = - sign * vNormal;
      // Reflection angles
      QLineF vLine( QPointF(0, 0), QPointF( vNormalThis, vTangentialThis) );
      mVelocityAngle = ::acos( vLine.dx() / vLine.length() );
      if ( vLine.dy() < 0 ) mVelocityAngle = 2 * M_PI - mVelocityAngle;
      mVelocityAngle += angle;
      //
      vLine = QLineF(  QPointF(0, 0), QPointF( vNormalOther, vTangentialOther) );
      item->mVelocityAngle = ::acos( vLine.dx() / vLine.length() );
      if ( vLine.dy() < 0 ) item->mVelocityAngle = 2 * M_PI - item->mVelocityAngle;
      item->mVelocityAngle += angle;

      // Final velocities are:
      mVelocity = sqrt( pow(vNormalThis, 2) + pow(vTangentialThis, 2) );
      item->mVelocity = sqrt( pow(vNormalOther, 2) + pow(vTangentialOther, 2) );
    }

  }
}

void PoolItem::computeNewPosition()
{
  int count = m_jobs.count();

  // Goes toward barycenter of all its hosts
  qreal xBar = 0;
  qreal yBar = 0;
  qreal vX, vY;
  vX = 0; vY = 0;

  mVelocity = mVelocity * (1 - FRICTION_FACTOR);

  // Velocity values if there is no interactions/gravity for this item
  vX = mVelocity * cos( mVelocityAngle );
  vY = mVelocity * sin( mVelocityAngle );
  mVelocity = qMin( qreal( sqrt( pow(vX, 2) + pow(vY, 2) ) ), MAX_VELOCITY );

  if ( count )
  {
    QList<PoolItem*> poolItems; // autosort
     // Servers attract clients
    foreach( Job j, m_jobs ) {
      PoolItem* poolItem = m_poolView->findPoolItem( j.server() );
      if ( poolItem && poolItem != this )
	poolItems << poolItem;

      if ( ::clientsAttractHosts ) {
	PoolItem* poolItem2 = m_poolView->findPoolItem( j.client() );
	if ( poolItem2 && poolItem2 != this )
	  poolItems << poolItem2;
      }
    }

    if ( poolItems.count() ) {
      foreach( PoolItem* poolItem, poolItems ) {
	xBar += poolItem->centerPosX();
	yBar += poolItem->centerPosY();
	setHostColor( poolItem->hostInfo()->color() );
      }
      xBar = xBar / (qreal)poolItems.count();
      yBar = yBar / (qreal)poolItems.count();
      vX +=  ( xBar - centerPosX() ) /  scene()->width() * 50  ;
      vY +=  ( yBar - centerPosY() ) /  scene()->height() * 50  ;

      QLineF l = QLineF( QPointF(0, 0), QPointF(vX, vY) );
      mVelocityAngle = ::acos( l.dx() / l.length() );
      if ( l.dy() < 0 ) mVelocityAngle = 2 * M_PI - mVelocityAngle;

      mVelocity = qMin( qreal( sqrt( pow(vX, 2) + pow(vY, 2) ) ), MAX_VELOCITY);
    }
  }


  setCenterPos( centerPosX() + vX, centerPosY() + vY );

}

void PoolItem::drawJobLines()
{
  // Draw job lines
  foreach( Job j, m_jobs ) {
    PoolItem* poolItem = m_poolView->findPoolItem( j.server() );
    if ( !poolItem || poolItem == this )
      continue;
    if ( ::showJobLines ) {
      m_jobLines[ j.jobId() ]->show();
      m_jobLines[ j.jobId() ]->setLine( centerPosX(), centerPosY(), poolItem->centerPosX(), poolItem->centerPosY() );
      m_jobLines[ j.jobId() ]->setPen( poolItem->hostInfo()->color() );
    }
    else {
      m_jobLines[ j.jobId() ]->hide();
    }
  }
}


PoolViewConfigDialog::PoolViewConfigDialog( QWidget *parent )
    : QDialog( parent )
{
    QBoxLayout *topLayout = new QVBoxLayout( this );

    mArchFilterEdit = new QLineEdit( this );
    topLayout->addWidget( mArchFilterEdit );
    connect( mArchFilterEdit, SIGNAL( textChanged( const QString & ) ),
             SIGNAL( configChanged() ) );

    mSuppressDomainName = new QCheckBox( tr("Suppress domain name"), this);
    topLayout->addWidget( mSuppressDomainName );
    connect( mSuppressDomainName, SIGNAL( toggled ( bool ) ),
             SLOT( slotSuppressDomainName ( bool ) ) );

    mShowJobLines = new QCheckBox( tr("Show job lines"), this );
    topLayout->addWidget( mShowJobLines );
    connect( mShowJobLines, SIGNAL( toggled( bool ) ),
	     SLOT( slotShowJobLines( bool )) );

    mClientsAttractHosts = new QCheckBox( tr("Clients attract hosts"), this);
    topLayout->addWidget( mClientsAttractHosts );
    connect( mClientsAttractHosts, SIGNAL( toggled( bool ) ),
	     SLOT( slotClientsAttractHosts( bool )) );

    QFrame *hline = new QFrame( this );
    hline->setFrameShape( QFrame::HLine );
    topLayout->addWidget( hline );

    QBoxLayout *buttonLayout = new QHBoxLayout();
    topLayout->addLayout( buttonLayout );

    buttonLayout->addStretch( 1 );

    QPushButton *button = new QPushButton( tr("&Close"), this );
    buttonLayout->addWidget( button );
    connect( button, SIGNAL( clicked() ), SLOT( hide() ) );
}

QString PoolViewConfigDialog::archFilter()
{
    return mArchFilterEdit->text();
}

void PoolViewConfigDialog::slotSuppressDomainName( bool b )
{
  ::suppressDomain = b;
  configChanged();
}

void PoolViewConfigDialog::slotShowJobLines( bool b )
{
  ::showJobLines = b;
  configChanged();
}

void PoolViewConfigDialog::slotClientsAttractHosts( bool b )
{
  ::clientsAttractHosts = b;
  configChanged();
}

PoolView::PoolView( HostInfoManager *m, QWidget *parent )
  : QWidget( parent ), StatusView( m ), m_poolItemWidth ( 30 ), m_poolItemHeight ( 30 )
{
    mConfigDialog = new PoolViewConfigDialog( this );
    connect( mConfigDialog, SIGNAL( configChanged() ),
             SLOT( slotConfigChanged() ) );

    m_canvas = new QGraphicsScene ( this );
    m_canvas->setSceneRect( 0, 0, 1000, 1000 );

    QHBoxLayout *layout = new QHBoxLayout( this );
    layout->setMargin( 0 );

    m_canvasView = new QGraphicsView( m_canvas, this );
    m_canvasView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_canvasView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_canvasView->setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
    m_canvasView->setViewportUpdateMode( QGraphicsView::FullViewportUpdate );
    layout->addWidget( m_canvasView );

    createKnownHosts();

    QTimer::singleShot( 0, this, SLOT( arrangePoolItems() ) );
}

void PoolView::update( const Job &job )
{
    unsigned int hostid = processor( job );
    if ( !hostid ) {
#if 0
        qDebug() << "Empty host" << endl;
#endif
        return;
    }

    PoolItem *poolItem = findPoolItem( hostid );
    if ( !poolItem ) return;

    poolItem->update( job );

    bool finished = job.state() == Job::Finished || job.state() == Job::Failed;

    QMap<unsigned int,PoolItem *>::Iterator it;
    it = mJobMap.find( job.jobId() );
    if ( it != mJobMap.end() ) {
        (*it)->update( job );
        if ( finished ) {
            mJobMap.erase( it );
            unsigned int clientid = job.client();
            PoolItem *clientItem = findPoolItem( clientid );
            if ( clientItem ) clientItem->setIsActiveClient( false );
        }
        return;
    }

    if ( !finished ) mJobMap.insert( job.jobId(), poolItem );

    if ( job.state() == Job::Compiling ) {
        unsigned int clientid = job.client();
        PoolItem *clientItem = findPoolItem( clientid );
        if ( clientItem ) {
            clientItem->setClient( clientid );
            clientItem->setIsActiveClient( true );
        }
    }

}

PoolItem *PoolView::findPoolItem( unsigned int hostid )
{
    PoolItem *poolItem = 0;
    QMap<unsigned int, PoolItem*>::iterator it = m_poolItems.find( hostid );
    if ( it != m_poolItems.end() ) poolItem = it.value();
    return poolItem;
}

void PoolView::checkNode( unsigned int hostid )
{
    if ( !hostid ) return;

    if ( !filterArch( hostid ) ) return;

    PoolItem *poolItem = findPoolItem( hostid );
    if ( !poolItem ) {
        poolItem = createPoolItem( hostid );
    }
}

void PoolView::removeNode( unsigned int hostid )
{
    PoolItem *poolItem = findPoolItem( hostid );

    if ( poolItem && poolItem->hostInfo()->isOffline() ) {
        removeItem( poolItem );
    }
}

void PoolView::forceRemoveNode( unsigned int hostid )
{
    PoolItem *poolItem = findPoolItem( hostid );

    if ( poolItem ) {
        removeItem( poolItem );
    }
}

void PoolView::removeItem( PoolItem *poolItem )
{
    m_poolItems.remove( poolItem->hostInfo()->id() );

    QList<unsigned int> obsoleteJobs;

    QMap<unsigned int,PoolItem *>::Iterator it;
    for( it = mJobMap.begin(); it != mJobMap.end(); ++it ) {
#if 0
        qDebug() << " JOB: " << it.key() << " (" << int( it.value() )
                 << ")" << endl;
#endif
        if ( it.value() == poolItem ) {
#if 0
            qDebug() << " Delete Job " << it.key() << endl;
#endif
            obsoleteJobs.append( it.key() );
        }
    }

    QList<unsigned int>::ConstIterator it2;
    for( it2 = obsoleteJobs.constBegin(); it2 != obsoleteJobs.constEnd(); ++it2 ) {
        mJobMap.remove( *it2 );
    }

    delete poolItem;
}

QWidget *PoolView::widget()
{
    return this;
}

void PoolView::resizeEvent( QResizeEvent * )
{
  m_canvasView->fitInView( m_canvas->sceneRect(), Qt::KeepAspectRatio );
}

bool PoolView::event ( QEvent* e )
{
    if (e->type() != QEvent::ToolTip) return QWidget::event(e);

    QPoint p ( static_cast<QHelpEvent*>(e)->pos());

    PoolItem *item = 0;
    if ( QGraphicsItem* graphicsItem = m_canvasView->itemAt( p ) )
        item = dynamic_cast<PoolItem*>( graphicsItem->parentItem() );
    if ( item ) {
        HostInfo *hostInfo = item->hostInfo();
        if ( !hostInfo ) return QWidget::event(e);

        QPoint gp( static_cast<QHelpEvent*>(e)->globalPos());
        QToolTip::showText(gp+QPoint(10,10),
                           "<p><table><tr><td>"
                           "<img source=\"computer\"><br><b>" + item->hostName() +
                           "</b><br>" +

                           "<table>" +
                           "<tr><td>" + tr("IP:") + "</td><td>" + hostInfo->ip()
                           + "</td></tr>" +
                           "<tr><td>" + tr("Platform:") + "</td><td>" +
                           hostInfo->platform() + "</td></tr>"
                           "<tr><td>" + tr("Flavor:") + "</td><td>" +
                           HostInfo::colorName( hostInfo->color() ) + "</td></tr>" +
                           "<tr><td>" + tr("Id:") + "</td><td>" +
                           QString::number( hostInfo->id() ) + "</td></tr>" +
                           "<tr><td>" + tr("Speed:") + "</td><td>" +
                           QString::number( hostInfo->serverSpeed() ) + "</td></tr>" +
                           "<tr><td>" + tr("Load:") + "</td><td>" +
                           QString::number( hostInfo->serverLoad() ) + "</td></tr>" +
                           "</table>"

                           "</td></tr></table></p>", this );
    }
    return QWidget::event(e);
}


void PoolView::slotConfigChanged()
{
    HostInfoManager::HostMap hostMap = hostInfoManager()->hostMap();
    HostInfoManager::HostMap::ConstIterator it;
    for( it = hostMap.constBegin(); it != hostMap.constEnd(); ++it ) {
        if ( filterArch( *it ) ) checkNode( it.key() );
        else forceRemoveNode( it.key() );
    }

    foreach( PoolItem* item, m_poolItems )
      item->drawJobLines();
}

// Main loop
void PoolView::arrangePoolItems()
{
  foreach( PoolItem *it, m_poolItems ) {
    it->computeNewPosition();
    it->updateName();
    it->checkBorders(); // make it bounce on the scene boundaries
    it->checkCollision(); // make it bounce on other items
  }


  foreach( PoolItem *it, m_poolItems ) {
    it->drawJobLines();
  }

  QTimer::singleShot( 0, this, SLOT( arrangePoolItems() ) );
}

PoolItem *PoolView::createPoolItem( unsigned int hostid )
{
    HostInfo *i = hostInfoManager()->find( hostid );

    if ( !i || i->isOffline() || i->name().isEmpty() )
        return 0;

    PoolItem *poolItem = new PoolItem( i, this, hostInfoManager() );
    m_poolItems.insert( hostid, poolItem );

    poolItem->setRandPos();

    return poolItem;
}

void PoolView::createKnownHosts()
{
    HostInfoManager::HostMap hosts = hostInfoManager()->hostMap();

    HostInfoManager::HostMap::ConstIterator it;
    for( it = hosts.constBegin(); it != hosts.constEnd(); ++it ) {
        unsigned int id = (*it)->id();
        if ( !findPoolItem( id ) ) createPoolItem( id );
    }
}

void PoolView::configureView()
{
    mConfigDialog->show();
    mConfigDialog->raise();
}

bool PoolView::filterArch( unsigned int hostid )
{
    HostInfo *i = hostInfoManager()->find( hostid );
    if ( !i ) {
        qWarning() << "No HostInfo for id " << hostid << endl;
        return false;
    }

    return filterArch( i );
}

bool PoolView::filterArch( HostInfo *i )
{
    if ( mConfigDialog->archFilter().isEmpty() ) return true;

    QRegExp regExp( mConfigDialog->archFilter() );

    if ( regExp.indexIn( i->platform() ) >= 0 ) {
        return true;
    }

    return false;
}

QGraphicsScene *PoolView::canvas()
{
  return m_canvas;
}

bool PoolView::suppressDomain()
{
  return ::suppressDomain;
}

bool PoolView::showJobLines()
{
  return ::showJobLines;
}

#include "poolview.moc"

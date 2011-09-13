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

#include <qdebug.h>

#include <qlayout.h>
#include <qtooltip.h>
#include <qslider.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qregexp.h>
#include <qcheckbox.h>
#include <qdir.h>
#include <QResizeEvent>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <math.h>

static bool suppressDomain = false;

StarViewConfigDialog::StarViewConfigDialog( QWidget *parent )
    : QDialog( parent )
{
//    setButtons( Close );
//    setEscapeButton( Close );
//    showButtonSeparator( true );

    QBoxLayout *topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );

    QLabel *label = new QLabel( tr("Number of nodes per ring:") );
    topLayout->addWidget( label );

    QBoxLayout *nodesLayout = new QHBoxLayout();
    topLayout->addLayout( nodesLayout );

    int nodesPerRing = 25;

    mNodesPerRingSlider = new QSlider( Qt::Horizontal );
    mNodesPerRingSlider->setMinimum( 1 );
    mNodesPerRingSlider->setMaximum( 50 );
    mNodesPerRingSlider->setSingleStep( 1 );
    mNodesPerRingSlider->setValue( nodesPerRing );
    nodesLayout->addWidget( mNodesPerRingSlider );
    connect( mNodesPerRingSlider, SIGNAL( valueChanged( int ) ),
             SIGNAL( configChanged() ) );
    connect( mNodesPerRingSlider, SIGNAL( valueChanged( int ) ),
             SLOT( slotNodesPerRingChanged( int ) ) );

    mNodesPerRingLabel = new QLabel( QString::number( nodesPerRing ) );
    nodesLayout->addWidget( mNodesPerRingLabel );

    label = new QLabel( tr("Architecture filter:") );
    topLayout->addWidget( label );
    mArchFilterEdit = new QLineEdit;
    topLayout->addWidget( mArchFilterEdit );
    connect( mArchFilterEdit, SIGNAL( textChanged( const QString & ) ),
             SIGNAL( configChanged() ) );

    mSuppressDomainName = new QCheckBox( tr("Suppress domain name") );
    topLayout->addWidget( mSuppressDomainName );
    connect( mSuppressDomainName, SIGNAL( toggled ( bool ) ),
             SLOT( slotSuppressDomainName ( bool ) ) );

    connect( this, SIGNAL( closeClicked() ), SLOT( hide() ) );
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
    m_boxItem->setPen( QPen( Qt::NoPen ) );

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
        halo->setPen( Qt::NoPen );
        ++count;
    }
}

StarView::StarView( HostInfoManager *m, QWidget *parent )
    : QWidget( parent ), StatusView( m )
{
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
    qDebug() << "StarView::update() " << job.jobId()
             << " server: " << job.server() << " client: " << job.client()
             << " state: " << job.stateAsString() << endl;
#endif
    if (job.state() == Job::WaitingForCS) {
        drawNodeStatus();
        return;
    }

    unsigned int hostid = processor( job );
    if ( !hostid ) {
        qDebug() << "Empty host" << endl;
        return;
    }

    HostItem *hostItem = findHostItem( hostid );
    if ( !hostItem ) return;

    hostItem->update( job );

    bool finished = job.state() == Job::Finished || job.state() == Job::Failed;

    QMap<unsigned int, HostItem *>::Iterator it;
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
        return;
    }

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
//  qDebug() << "StarView::checkNode() " << hostid << endl;

    if ( !hostid ) return;

    if ( !filterArch( hostid ) ) return;

    HostItem *hostItem = findHostItem( hostid );
    if ( !hostItem ) {
        hostItem = createHostItem( hostid );
        arrangeHostItems();
    }
}

void StarView::removeNode( unsigned int hostid )
{
//  qDebug() << "StarView::removeNode() " << hostid << endl;

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
    qDebug() << "StarView::removeItem() " << hostid << " ("
             << int( hostItem ) << ")" << endl;
#endif

    m_hostItems.remove( hostItem->hostInfo()->id() );

    QList<unsigned int> obsoleteJobs;

    QMap<unsigned int,HostItem *>::Iterator it;
    for( it = mJobMap.begin(); it != mJobMap.end(); ++it ) {
#if 0
        qDebug() << " JOB: " << it.key() << " (" << int( it.value() )
                 << ")" << endl;
#endif
        if ( it.value() == hostItem ) {
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

    delete hostItem->stateItem();
    delete hostItem;

    arrangeHostItems();
}

void StarView::updateSchedulerState( bool online )
{
    QString txt;
    if ( online ) {
        txt = tr("Scheduler");
    } else {
        txt = "";
    }
    delete m_schedulerItem;

    if ( !online ) {
        QMap<unsigned int,HostItem *>::ConstIterator it;
        for( it = m_hostItems.constBegin(); it != m_hostItems.constEnd(); ++it ) {
            delete *it;
        }
        m_hostItems.clear();
        mJobMap.clear();
    }

    m_schedulerItem = new HostItem( txt, m_canvas, hostInfoManager() );
    m_schedulerItem->setZValue(100);
    m_schedulerItem->show();
    centerSchedulerItem();
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
}

bool StarView::event ( QEvent* e )
{
    if (e->type() != QEvent::ToolTip) return QWidget::event(e);

    QPoint p ( static_cast<QHelpEvent*>(e)->pos());

    HostItem *item = 0;
    QGraphicsItem* graphicsItem = m_canvasView->itemAt( p );
    if ( graphicsItem )
        item = dynamic_cast<HostItem*>( graphicsItem->parentItem() );
    if ( item ) {
        HostInfo *hostInfo = item->hostInfo();
        const QPoint gp( static_cast<QHelpEvent*>(e)->globalPos());
        const QRect itemRect = m_canvasView->mapFromScene(graphicsItem->sceneBoundingRect()).boundingRect();
        if ( hostInfo ) {
            QToolTip::showText(gp+QPoint(10,10),
                           "<p><table><tr><td>"
                           "<img align=\"right\" source=\"icons:computer.png\"><br><b>" + item->hostName() +
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
                           "</table>"

                           "</td></tr></table></p>", this, itemRect );
        } else {
            QToolTip::showText(gp+QPoint(10,10),
                           "<p><table><tr><td>"
                           "<img align=\"right\" source=\"icons:computer.png\"><br><b>" + tr("Scheduler") + "</b><br/>"
                           "<table>" +
                           "<tr><td>" + tr("Host: %1").arg(hostInfoManager()->schedulerName()) + "</td></tr>" +
                           "<tr><td>" + tr("Network name: %1").arg(hostInfoManager()->networkName()) + "</td></tr>" +
                           "</table>"
                           "</td></tr></table></p>", this, itemRect );
        }
    } else {
         QToolTip::hideText();
    }
    return QWidget::event(e);
}

void StarView::centerSchedulerItem()
{
    m_schedulerItem->setCenterPos( width() / 2, height() / 2 );
}

void StarView::slotConfigChanged()
{
//  qDebug() << "StarView::slotConfigChanged()" << endl;

    HostInfoManager::HostMap hostMap = hostInfoManager()->hostMap();
    HostInfoManager::HostMap::ConstIterator it;
    for( it = hostMap.constBegin(); it != hostMap.constEnd(); ++it ) {
        if ( filterArch( *it ) ) checkNode( it.key() );
        else forceRemoveNode( it.key() );
    }

    arrangeHostItems();
}

void StarView::arrangeHostItems()
{
//  qDebug() << "StarView::arrangeHostItems()" << endl;

    int count = m_hostItems.count();

//  qDebug() << "  Count: " << count << endl;

    int nodesPerRing = mConfigDialog->nodesPerRing();

    int ringCount = int( count / nodesPerRing ) + 1;

//  qDebug() << "  Rings: " << ringCount << endl;
    double radiusFactor = 2.5;
    if (suppressDomain) radiusFactor = 4;
    const int xRadius = qRound( m_canvas->width() / radiusFactor );
    const int yRadius = qRound( m_canvas->height() / radiusFactor );

    const double step = 2 * M_PI / count;

    double angle = 0.0;
    int i = 0;
    QMap<unsigned int, HostItem*>::ConstIterator it;
    for ( it = m_hostItems.constBegin(); it != m_hostItems.constEnd(); ++it ) {
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
}

HostItem *StarView::createHostItem( unsigned int hostid )
{
    HostInfo *i = hostInfoManager()->find( hostid );

    if ( !i || i->isOffline() || i->name().isEmpty() )
        return 0;

//  qDebug() << "New node for " << hostid << " (" << i->name() << ")" << endl;

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
    for ( it = m_hostItems.constBegin(); it != m_hostItems.constEnd(); ++it ) {
        drawState( *it );
    }
}

void StarView::drawState( HostItem *node )
{
    delete node->stateItem();
    QGraphicsLineItem *newItem = 0;

    unsigned int client = node->client();
    QColor color = client ? hostColor( client ) : Qt::green;

    if ( node->isCompiling() || node->isActiveClient() ) {
        newItem = new QGraphicsLineItem( qRound( node->centerPosX() ),
                                         qRound( node->centerPosY() ),
                                         qRound( m_schedulerItem->centerPosX() ),
                                         qRound( m_schedulerItem->centerPosY() ) );
        if ( node->isCompiling() ) {
            newItem->setPen( QPen( color, 0 ) );
            newItem->setZValue( -301 );
        } else if ( node->isActiveClient() ) {
            newItem->setPen( QPen( color, 1, Qt::DashLine ) );
            newItem->setZValue( -300 );
        }
        m_canvas->addItem( newItem );
    }

    node->setStateItem( newItem );
}

void StarView::createKnownHosts()
{
    HostInfoManager::HostMap hosts = hostInfoManager()->hostMap();

    HostInfoManager::HostMap::ConstIterator it;
    for( it = hosts.constBegin(); it != hosts.constEnd(); ++it ) {
        unsigned int id = (*it)->id();
        if ( !findHostItem( id ) ) createHostItem( id );
    }
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
        qDebug() << "No HostInfo for id " << hostid;
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

/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>

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

#include <qcanvas.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qvaluelist.h>

#include <math.h>

using namespace std;

class NodeItem : public QCanvasText
{
public:
    NodeItem( const QString &hostName, QCanvas *canvas )
        : QCanvasText( hostName, canvas ),
          m_hostName( hostName ),
          m_stateItem( 0 )
        {
        }

    void setState( Job::State state ) { m_state = state; }
    Job::State state() const { return m_state; }

    void setStateItem( QCanvasItem *item ) { m_stateItem = item; }
    QCanvasItem *stateItem() { return m_stateItem; }

    QString hostName() const { return m_hostName; }

private:
    Job::State m_state;
    QString m_hostName;
    QCanvasItem *m_stateItem;
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

    m_localhostItem = new QCanvasText( i18n( "localhost" ), m_canvas );
    centerLocalhostItem();
    m_localhostItem->show();

    m_canvas->update();
}

void StarView::update( const Job &job )
{
  kdDebug() << "StarView::checkForNewNode() " << job.jobId()
            << " server: " << job.server() << " client: " << job.client()
            << " state: " << job.stateAsString() << endl;

  checkForNewNode( job );
  updateNodeStatus( job );
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
    arrangeNodeItems();
    m_canvas->update();
}

void StarView::centerLocalhostItem()
{
    const QRect br = m_localhostItem->boundingRect();
    const int newX = ( width() - br.width() ) / 2;
    const int newY = ( height() - br.height() ) / 2;
    m_localhostItem->move( newX, newY );
}

void StarView::arrangeNodeItems()
{
    const int radius = kMin( m_canvas->width() / 2, m_canvas->height() / 2 );
    const double step = 2 * M_PI / m_nodeItems.count();

    double angle = 0.0;
    QDictIterator<NodeItem> it( m_nodeItems );
    while ( it.current() != 0 ) {
        it.current()->move( m_localhostItem->x() + ( cos( angle ) * radius ),
                            m_localhostItem->y() + ( sin( angle ) * radius ) );
        angle += step;
        ++it;
    }
}

void StarView::checkForNewNode( const Job &job )
{
    kdDebug() << "StarView::checkForNewNode() " << job.jobId() << endl;

    QString server = job.server();
    if ( server.isEmpty() ) {
      kdDebug() << "Empty server" << endl;
      return;
    }
    NodeItem *nodeItem = m_nodeItems.find( server );
    if ( !nodeItem ) {
      kdDebug() << "New node for '" << server << "'" << endl;
      nodeItem = new NodeItem( server, m_canvas );
      m_nodeItems.insert( server, nodeItem );
      nodeItem->show();

      arrangeNodeItems();

      m_canvas->update();
    }
}

void StarView::updateNodeStatus( const Job &job )
{
    kdDebug() << "StarView::updateNodeStatus() " << job.jobId() << endl;

    QString server = job.server();
    if ( server.isEmpty() ) {
      kdDebug() << "Empty server" << endl;
      return;
    }
    NodeItem *nodeItem = m_nodeItems.find( server );
    if ( !nodeItem ) {
      kdError() << "NodeItem for '" << server << "' is missing." << endl;
    } else {
      nodeItem->setState( job.state() );
    }
}

void StarView::drawNodeStatus()
{
    QDictIterator<NodeItem> it( m_nodeItems );
    for ( ; it.current() != 0; ++it )
        drawState( it.current() );
    m_canvas->update();
}

void StarView::drawState( NodeItem *node )
{
    delete node->stateItem();
    QCanvasItem *newItem = 0;

    const QPoint nodeCenter = node->boundingRect().center();
    const QPoint localCenter = m_localhostItem->boundingRect().center();

    switch ( node->state() ) {
        case Job::Compiling: {
            QCanvasLine *line = new QCanvasLine( m_canvas );
            line->setPen( Qt::green );

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
#if 0
        case Job::Send: {
            QPointArray points( 3 );
            points.setPoint( 0, localCenter.x() - 5, localCenter.y() );
            points.setPoint( 1, nodeCenter );
            points.setPoint( 2, localCenter.x() + 5, localCenter.y() );

            QCanvasPolygon *poly = new QCanvasPolygon( m_canvas );
            poly->setBrush( Qt::blue );
            poly->setPoints( points );
            poly->show();

            newItem = poly;
            break;
        }
        case Job::Receive: {
            QPointArray points( 3 );
            points.setPoint( 0, nodeCenter.x() - 5, nodeCenter.y() );
            points.setPoint( 1, localCenter );
            points.setPoint( 2, nodeCenter.x() + 5, nodeCenter.y() );

            QCanvasPolygon *poly = new QCanvasPolygon( m_canvas );
            poly->setBrush( Qt::blue );
            poly->setPoints( points );
            poly->show();

            newItem = poly;
            break;
        }
#endif
    }

    node->setStateItem( newItem );
}

#include "starview.moc"

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
#ifndef ICEMON_POOLVIEW_H
#define ICEMON_POOLVIEW_H

#include "job.h"
#include "statusview.h"

#include <q3canvas.h>
#include <qdialog.h>
#include <QResizeEvent>
#include <QLabel>
#include <QGraphicsEllipseItem>


class HostInfo;
class PoolItem;

class QSlider;
class QLabel;
class QLineEdit;
class QCheckBox;
class QGraphicsView;
class QGraphicsLineItem;

class PoolView;

class PoolItem : public QGraphicsItemGroup
{
public:
    enum { RttiPoolItem = 1000 };

    PoolItem( const QString &text, PoolView *poolView, HostInfoManager * );
    PoolItem( HostInfo *hostInfo, PoolView *poolView, HostInfoManager * );
    ~PoolItem();

    void init();

    HostInfo *hostInfo() const { return mHostInfo; }

    void setHostColor( const QColor &color );

    void setIsActiveClient( bool active ) { mIsActiveClient = active; }
    bool isActiveClient() const { return mIsActiveClient; }

    void setIsCompiling( bool compiling ) { mIsCompiling = compiling; }
    bool isCompiling() const { return mIsCompiling; }

    void setStateItem( QGraphicsItem *item ) { m_stateItem = item; }
    QGraphicsItem *stateItem() { return m_stateItem; }

    void setClient( unsigned int client ) { m_client = client; }
    unsigned int client() const { return m_client; }

    QString hostName() const;
    void updateName();

    double centerPosX() const;
    double centerPosY() const;

    double relativeCenterPosX() const { return m_textItem->boundingRect().width()/2; }
    double relativeCenterPosY() const { return m_textItem->boundingRect().height()/2; }

    void computeNewPosition();
    void checkBorders();
    void checkCollision();
    void drawJobLines();
    // Choose a random position for the item
    void setRandPos();
    void setCenterPos( double x, double y );
    void setSize( qreal w, qreal h );

    void update( const Job &job );

    qreal velocity() const { return mVelocity; }
    qreal radius() const { return m_boxItem->rect().height() / 2.; }

private:
    HostInfo *mHostInfo;
    HostInfoManager *mHostInfoManager;
    JobList m_jobs;

    bool mIsActiveClient;
    bool mIsCompiling;

    QGraphicsItem *m_stateItem;
    QGraphicsSimpleTextItem* m_textItem;
    unsigned int m_client;

    // Position and velocity
    qreal mX;
    qreal mY;
    qreal mVelocityAngle;
    qreal mVelocity;

    
    QGraphicsEllipseItem *m_boxItem;
    QMap<Job, QGraphicsEllipseItem *> m_jobHalos;
    QMap<int, QGraphicsLineItem*> m_jobLines;
  
    PoolView *m_poolView;

    bool mIsLocalHost;
};


class PoolViewConfigDialog : public QDialog
{
    Q_OBJECT

public:
    PoolViewConfigDialog( QWidget *parent );

    bool suppressDomainName() const;
    bool showJobLines() const;

    QString archFilter();
protected slots:
    void slotSuppressDomainName( bool );
    void slotShowJobLines( bool );
signals:
    void configChanged();

private:
    QLineEdit *mArchFilterEdit;
    QCheckBox *mSuppressDomainName;
    QCheckBox *mShowJobLines;
};



class PoolView : public QWidget, public StatusView
{
    Q_OBJECT

public:
    PoolView( HostInfoManager *, QWidget *parent );

    void update( const Job &job );
    QWidget *widget();

    QString id() const { return "pool"; }

    PoolItem *findPoolItem( unsigned int hostid );

    void checkNode( unsigned int hostid );

    void removeNode( unsigned int hostid );

    void configureView();

    QGraphicsScene* canvas();
    bool suppressDomain();
    bool showJobLines();

    int poolItemWidth() { return m_poolItemWidth; }
    int poolItemHeight() { return m_poolItemHeight; }

    QMap<unsigned int,PoolItem *> m_poolItems;

protected:
    virtual void resizeEvent( QResizeEvent *e );
    virtual bool event(QEvent *event);

    /**
       Return true if node should be shown and false if not.
    */
    bool filterArch( unsigned int hostid );
    /**
       Return true if node should be shown and false if not.
    */
    bool filterArch( HostInfo * );

    void removeItem( PoolItem * );
    void forceRemoveNode( unsigned int hostid );

    protected slots:
    void slotConfigChanged();

private slots:
    void arrangePoolItems();
private:
    void createKnownHosts();
    PoolItem *createPoolItem( unsigned int hostid );

    void drawNodeStatus();
    // void drawState( PoolItem *node );
    void updatePoolItemsSize();

    PoolViewConfigDialog *mConfigDialog;

    QGraphicsScene *m_canvas;
    QGraphicsView *m_canvasView;
 
    
    QMap<unsigned int,PoolItem *> mJobMap;
    
    const int m_poolItemWidth;
    const int m_poolItemHeight;

    qreal m_scaleFactor;
};

#endif
// vim:ts=4:sw=4:noet

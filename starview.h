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
#ifndef ICEMON_STARVIEW_H
#define ICEMON_STARVIEW_H

#include "mon-kde.h"

#include <qdict.h>
#include <qcanvas.h>
#include <qdialog.h>

class HostInfo;

class QSlider;
class QLabel;

class StarViewConfigDialog : public QDialog
{
    Q_OBJECT
  public:
    StarViewConfigDialog( QWidget *parent );

    int nodesPerRing();

    void setMaxNodes( int );

  protected slots:
    void slotNodesPerRingChanged( int nodes );
  
  signals:
    void configChanged();

  private:
    QSlider *mNodesPerRingSlider;
    QLabel *mNodesPerRingLabel;
};


class HostItem : public QCanvasText
{
  public:
    enum { RttiHostItem = 1000 };

    HostItem( const QString &text, QCanvas *canvas );
    HostItem( HostInfo *hostInfo, QCanvas *canvas );
    ~HostItem();

    void deleteSubItems();

    int rtti() const { return RttiHostItem; }

    HostInfo *hostInfo() const { return mHostInfo; }

    void setHostColor( const QColor &color );

    void setIsActiveClient( bool active ) { mIsActiveClient = active; }
    bool isActiveClient() const { return mIsActiveClient; }
    
    void setIsCompiling( bool compiling ) { mIsCompiling = compiling; }
    bool isCompiling() const { return mIsCompiling; }

    void setStateItem( QCanvasItem *item ) { m_stateItem = item; }
    QCanvasItem *stateItem() { return m_stateItem; }

    void setClient( unsigned int client ) { m_client = client; }
    unsigned int client() const { return m_client; }

    QString hostName() const;

    void moveBy( double dx, double dy );

    void update( const Job &job );

  private:
    void init();

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


class StarView : public QWidget, public StatusView
{
    Q_OBJECT
  public:
    StarView( HostInfoManager *, QWidget *parent, const char *name = 0 );

    void update( const Job &job );
    QWidget *widget();

    QString id() const { return "star"; }

    HostItem *StarView::findHostItem( unsigned int hostid );

    void checkNode( unsigned int hostid );

    void removeNode( unsigned int hostid );

    void configureView();

  protected:
    virtual void resizeEvent( QResizeEvent *e );

  protected slots:
    void slotConfigChanged();

  private:
    void createKnownHosts();
    void centerLocalhostItem();
    HostItem *createHostItem( unsigned int hostid );
    void arrangeHostItems();
    void drawNodeStatus();
    void drawState( HostItem *node );

    StarViewConfigDialog *mConfigDialog;

    QCanvas *m_canvas;
    QCanvasView *m_canvasView;
    QCanvasText *m_localhostItem;
    QMap<unsigned int,HostItem*> m_hostItems;
    QMap<unsigned int,HostItem *> mJobMap;
};

#endif
// vim:ts=4:sw=4:noet

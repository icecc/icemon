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

#include "job.h"
#include "statusview.h"

#include <qcanvas.h>
#include <qdialog.h>

class HostInfo;

class QSlider;
class QLabel;
class QLineEdit;
class QCheckBox;

class StarViewConfigDialog : public QDialog
{
    Q_OBJECT
  public:
    StarViewConfigDialog( QWidget *parent );

    int nodesPerRing();
    bool suppressDomainName() const;

    void setMaxNodes( int );

    QString archFilter();

  protected slots:
    void slotNodesPerRingChanged( int nodes );
    void slotSuppressDomainName( bool );

  signals:
    void configChanged();

  private:
    QSlider *mNodesPerRingSlider;
    QLabel *mNodesPerRingLabel;
    QLineEdit *mArchFilterEdit;
    QCheckBox *mSuppressDomainName;
};


class HostItem : public QCanvasText
{
  public:
    enum { RttiHostItem = 1000 };

    HostItem( const QString &text, QCanvas *canvas, HostInfoManager * );
    HostItem( HostInfo *hostInfo, QCanvas *canvas, HostInfoManager * );
    ~HostItem();

    void init();

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
    void updateName();

    void moveBy( double dx, double dy );

    double centerPosX() const { return m_boxItem->x(); }
    double centerPosY() const { return m_boxItem->y(); }

    void setCenterPos( double x, double y );

    void update( const Job &job );

  protected:
    void createJobHalo( const Job & );
    void deleteJobHalo( const Job &job );
    void updateHalos();

  private:
    HostInfo *mHostInfo;
    HostInfoManager *mHostInfoManager;

    bool mIsActiveClient;
    bool mIsCompiling;

    QCanvasItem *m_stateItem;
    unsigned int m_client;

    int mBaseWidth;
    int mBaseHeight;

    QCanvasEllipse *m_boxItem;

    QMap<Job, QCanvasEllipse *> m_jobHalos;

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

    void updateSchedulerState( bool online );

    void configureView();

  protected:
    virtual void resizeEvent( QResizeEvent *e );

    /**
      Return true if node should be shown and false if not.
    */
    bool filterArch( unsigned int hostid );
    /**
      Return true if node should be shown and false if not.
    */
    bool filterArch( HostInfo * );

    void removeItem( HostItem * );
    void forceRemoveNode( unsigned int hostid );

  protected slots:
    void slotConfigChanged();

  private:
    void createKnownHosts();
    void centerSchedulerItem();
    HostItem *createHostItem( unsigned int hostid );
    void arrangeHostItems();
    void drawNodeStatus();
    void drawState( HostItem *node );

    StarViewConfigDialog *mConfigDialog;

    QCanvas *m_canvas;
    QCanvasView *m_canvasView;
    HostItem *m_schedulerItem;
    QMap<unsigned int,HostItem *> m_hostItems;
    QMap<unsigned int,HostItem *> mJobMap;
};

#endif
// vim:ts=4:sw=4:noet

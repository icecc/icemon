/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef GANTTSTATUSVIEW_H
#define GANTTSTATUSVIEW_H

#include "job.h"
#include "statusview.h"

#include <qdialog.h>
#include <qmap.h>
#include <qpixmap.h>
#include <q3scrollview.h>
#include <qlist.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPaintEvent>

class QCheckBox;
class QGridLayout;
class QTimer;
class QVBoxLayout;

class GanttConfigDialog : public QDialog
{
    Q_OBJECT
  public:
    GanttConfigDialog( QWidget *parent );

    bool isTimeScaleVisible();

  signals:
    void configChanged();

  private:
    QCheckBox *mTimeScaleVisibleCheck;
};

class GanttTimeScaleWidget : public QWidget
{
    Q_OBJECT
  public:
    GanttTimeScaleWidget( QWidget *parent, const char *name = 0 );

    void setPixelsPerSecond( int );

  protected:
    virtual void paintEvent( QPaintEvent *e );

  private:
    int mPixelsPerSecond;
};

class GanttProgress : public QWidget
{
    Q_OBJECT
  public:
    GanttProgress( StatusView *statusView,
                   QWidget *parent, const char *name = 0 );

    bool isFree() const { return mIsFree; }
    bool fullyIdle() const { return m_jobs.count() == 1 && isFree(); }

  public slots:
    void progress();
    void update( const Job &job );

  protected:
    virtual void paintEvent( QPaintEvent *e );
    virtual void resizeEvent( QResizeEvent *e );

  private:
    void adjustGraph();
    void drawGraph( QPainter &p );
    QColor colorForStatus( const Job &job ) const;

    struct JobData
    {
      JobData( const Job& j, int c )
        : job( j ), clock( c ), next_text_width( 0 ) {}
      JobData() {}; // stupid QValueList

      bool operator==( const JobData& d )
      {
        return job == d.job && clock == d.clock;
      }

      Job job;
      int clock;
      mutable int next_text_width;
      mutable QPixmap text_cache;
    };

    StatusView *mStatusView;

    QList< JobData > m_jobs;

    int mClock;

    bool mIsFree;
};

class GanttStatusView : public Q3ScrollView, public StatusView
{
    Q_OBJECT
  public:
    GanttStatusView( HostInfoManager *, QWidget *parent, const char *name = 0 );
    virtual ~GanttStatusView() {}

    QString id() const { return "gantt"; }

    virtual void checkNode( unsigned int hostid );

    void start();
    void stop();
    void checkNodes();

    void configureView();

  public slots:
    virtual void update( const Job &job );
    virtual QWidget *widget();

  protected:
    void viewportResizeEvent( QResizeEvent *e );

  private slots:
    void slotConfigChanged();
    void updateGraphs();
    void checkAge();

  private:
    GanttProgress *registerNode( unsigned int hostid );
    void removeSlot( unsigned int hostid, GanttProgress* slot );
    void unregisterNode( unsigned int hostid );

    GanttConfigDialog *mConfigDialog;

    QWidget *mTopWidget;
    QGridLayout *m_topLayout;

    GanttTimeScaleWidget *mTimeScale;

    typedef QList<GanttProgress *> SlotList;
    typedef QMap<unsigned int,SlotList> NodeMap;
    NodeMap mNodeMap;
    typedef QMap<unsigned int,int> AgeMap;
    AgeMap mAgeMap;
    typedef QMap<unsigned int, GanttProgress *> JobMap;
    JobMap mJobMap;
    typedef QMap<unsigned int,QVBoxLayout *> NodeLayoutMap;
    NodeLayoutMap mNodeLayouts;
    typedef QMap<unsigned int,int> NodeRowMap;
    NodeRowMap mNodeRows;
    typedef QMap<unsigned int,QWidget *> NodeLabelMap;
    NodeLabelMap mNodeLabels;
    QTimer *m_progressTimer;
    QTimer *m_ageTimer;

    bool mRunning;

    int mUpdateInterval;

    int mMinimumProgressHeight;
};

#endif
// vim:ts=4:sw=4:noet

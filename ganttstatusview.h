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

#include "mon-kde.h"

#include <qmap.h>
#include <qvaluelist.h>
#include <qwidget.h>

class Job;
class JobList;
class QGridLayout;
class QTimer;

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
		GanttProgress( QMap<QString,QColor> &hostColors,
                               QWidget *parent, const char *name = 0 );

                void setHostColors( QMap<QString,QColor> & );

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
                    Job job;
                    int clock;
                    mutable int next_text_width;
                    mutable QPixmap text_cache;
                    JobData() {}; // stupid QValueList
                    bool operator==( const JobData& d )
                        { return job == d.job && clock == d.clock; }
                };
		QValueList< JobData > m_jobs;
                
                QMap<QString,QColor> &mHostColors;

                int mClock;

                bool mIsFree;
};

class GanttStatusView : public QWidget, public StatusView
{
    Q_OBJECT

public:
    GanttStatusView( QWidget *parent, const char *name = 0 );
    virtual ~GanttStatusView() {}

    QString id() const { return "gantt"; }

    void checkNode( const QString &host, unsigned int max_kids );

    void start();
    void stop();
    void checkNodes();

public slots:
    virtual void update( const Job &job );
    virtual QWidget *widget();

private slots:
    void updateGraphs();
    void checkAge();

private:
    GanttProgress *registerNode( const QString &name );
    void removeSlot( const QString& name, GanttProgress* slot );
    void unregisterNode( const QString &name );
    void createHostColor( const QString &host );
    QString nameForIp( const QString &ip );

    QGridLayout *m_topLayout;
    typedef QValueList<GanttProgress *> SlotList;
    typedef QMap<QString,SlotList> NodeMap;
    NodeMap mNodeMap;
    typedef QMap<QString,int> AgeMap;
    AgeMap mAgeMap;
    typedef QMap<unsigned int, GanttProgress *> JobMap;
    JobMap mJobMap;
    typedef QMap<QString,QVBoxLayout *> NodeLayoutMap;
    NodeLayoutMap mNodeLayouts;
    typedef QMap<QString,int> NodeRowMap;
    NodeRowMap mNodeRows;
    typedef QMap<QString,QWidget *> NodeLabelMap;
    NodeLabelMap mNodeLabels;
    QTimer *m_progressTimer;
    QTimer *m_ageTimer;

    QMap<QString,QColor> mHostColors;

    bool mRunning;

    int mUpdateInterval;
};

#endif // GANTTSTATUSVIEW_H
// vim:ts=4:sw=4:noet

#ifndef GANTTSTATUSVIEW_H
#define GANTTSTATUSVIEW_H

#include "mon-kde.h"

#include <qmap.h>
#include <qpair.h>
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

                bool isFree() { return mIsFree; }

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

		QValueList< QPair<Job, int> > m_jobs;
                
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

    void checkForNewNode( const QString &host );

    void start();
    void stop();

public slots:
    virtual void update( const Job &job );
    virtual QWidget *widget();

private slots:
    void updateGraphs();

private:
    GanttProgress *registerNode( const QString &name );
    void createHostColor( const QString &host );

    QGridLayout *m_topLayout;
    typedef QValueList<GanttProgress *> SlotList;
    typedef QMap<QString,SlotList> NodeMap;
    NodeMap mNodeMap;
    QMap<unsigned int, GanttProgress *> mJobMap;
    typedef QMap<QString,QVBoxLayout *> NodeLayoutMap;
    NodeLayoutMap mNodeLayouts;
    typedef QMap<QString,int> NodeRowMap;
    NodeRowMap mNodeRows;
    QTimer *m_progressTimer;

    QMap<QString,QColor> mHostColors;

    bool mRunning;

    int mUpdateInterval;
};

#endif // GANTTSTATUSVIEW_H
// vim:ts=4:sw=4:noet

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

	protected:
		virtual void paintEvent( QPaintEvent *e );
};

class GanttProgress : public QWidget
{
	Q_OBJECT
	public:
		GanttProgress( QMap<QString,QColor> &hostColors,
                               QWidget *parent, const char *name = 0 );

                void setHostColors( QMap<QString,QColor> & );
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
		int m_totalWidth;
                
                QMap<QString,QColor> &mHostColors;
};

class GanttStatusView : public QWidget, public StatusView
{
    Q_OBJECT

public:
    GanttStatusView( QWidget *parent, const char *name = 0 );
    virtual ~GanttStatusView() {}

public slots:
    virtual void update( const Job &job );
    virtual QWidget *widget();

private slots:
    void updateGraphs();

private:
    void checkForNewNodes( const Job &job );
    void updateNodes( const Job &job );
    void registerNode( const QString &name );
    void createHostColor( const QString &host );

    QGridLayout *m_topLayout;
    QMap<QString, GanttProgress *> m_nodeMap;
    QTimer *m_progressTimer;

    QMap<QString,QColor> mHostColors;
};

#endif // GANTTSTATUSVIEW_H
// vim:ts=4:sw=4:noet

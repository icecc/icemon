#ifndef MON_KDE_H
#define MON_KDE_H

#include <kmainwindow.h>

#include <qdict.h>

class KListView;
template <class T> class QValueList;

class QCanvas;
class QCanvasText;
class QCanvasView;
class NodeItem;

class Job
{
	public:
		enum State {
			Compile, Connect, Send, Receive, Blocked, Startup,
			CPP, Unknown
		};

		static QString stateAsString( State state );
		static State stringAsState( const QString &s );

		Job() { }
//		explicit Job( const struct dcc_mon_list * const job );

		bool operator==( const Job &rhs ) const;
		bool operator!=( const Job &rhs ) const { return !operator==( rhs ); }

		unsigned int pid() const { return m_pid; }
		QString fileName() const { return m_fileName; }
		QString host() const { return m_host; }
		State state() const { return m_state; }

	private:
		unsigned int m_pid;
		QString m_fileName;
		QString m_host;
		State m_state;
};

class JobList : public QValueList<Job>
{
	public:
		JobList() { }
//		JobList( const struct dcc_mon_list * const list );

		// Tests for equivalence, not equality!
		bool operator==( const JobList &rhs ) const;
		bool operator!=( const JobList &rhs ) const { return !operator==( rhs ); }
};

class StatusObserver : public QObject
{
	Q_OBJECT
	public:
		StatusObserver( QObject *parent, const char *name = 0 );

		void start();
		void stop();

	signals:
		void statusChanged( const JobList &jobs );

	private slots:
		void pollTimeout();

	private:
		void setupErrorMessages();

		QTimer *m_timer;
		QValueList<Job> m_prevJobs;
		QMap<int, QString> m_errorMessages;
};

class StatusView : public QWidget
{
	Q_OBJECT
	public:
		StatusView( QWidget *parent, const char *name = 0, WFlags f = 0 );

	public slots:
		virtual void update( const JobList &jobs ) = 0;
};

class ListStatusView : public StatusView
{
	Q_OBJECT
	public:
		ListStatusView( QWidget *parent, const char *name = 0 );

	public slots:
		virtual void update( const JobList &jobs );

	private:
		KListView *m_listView;
};

class StarStatusView : public StatusView
{
	Q_OBJECT
	public:
		StarStatusView( QWidget *parent, const char *name = 0 );

	public slots:
		virtual void update( const JobList &jobs );

	protected:
		virtual void resizeEvent( QResizeEvent *e );

	private:
		void centerLocalhostItem();
		void arrangeNodeItems();
		void checkForNewNodes( const JobList &jobs );
		void updateNodeStatus( const JobList &jobs );
		void drawNodeStatus();
		void drawState( NodeItem *node );

		QCanvas *m_canvas;
		QCanvasView *m_canvasView;
		QCanvasText *m_localhostItem;
		QDict<NodeItem> m_nodeItems;
};

class MainWindow : public KMainWindow
{
	Q_OBJECT
	public:
		MainWindow( QWidget *parent, const char *name = 0 );

	private slots:
		void setupListView();
		void setupStarView();
		void rememberJobs( const JobList &jobs );

	private:
		void setupView( StatusView *view );

		StatusObserver *m_observer;
		StatusView *m_view;
		JobList m_rememberedJobs;
};

#endif // MON_KDE_H
// vim:ts=4:sw=4:noet

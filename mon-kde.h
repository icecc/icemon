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
    enum State { WaitingForCS, Compiling, Finished };
    Job(unsigned int id, const QString &filename,
        const QString &environment, const QString &lang) {
        m_id = id;
        m_fileName = filename;
        m_env = environment;
        m_lang = lang;
        m_state = WaitingForCS;
    }
    Job() {
        m_id = 0;
    }

    bool operator==( const Job &rhs ) const { return m_id == rhs.m_id; }
    bool operator!=( const Job &rhs ) const { return m_id != rhs.m_id; }

    unsigned int jobId() const { return m_id; }
    QString fileName() const { return m_fileName; }
    QString host() const { return m_host; }
    State state() const { return m_state; }
    QString stateAsString() const;

private:
    unsigned int m_id;
    QString m_fileName;
    QString m_host;
    QString m_lang;
    QString m_env;
    State m_state;
};

class JobList : public QValueList<Job>
{
public:
    JobList() { }

    // Tests for equivalence, not equality!
    bool operator==( const JobList &rhs ) const;
    bool operator!=( const JobList &rhs ) const { return !operator==( rhs ); }
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
/*
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
*/

class MsgChannel;
class QSocketNotifier;
class Msg;

class MainWindow : public KMainWindow
{
    Q_OBJECT
public:
    MainWindow( QWidget *parent, const char *name = 0 );

private slots:
    void setupListView();
    void setupStarView();
    void rememberJobs( const JobList &jobs );
    void slotCheckScheduler();
    void msgReceived();

private:
    void setupView( StatusView *view );
    void checkScheduler(bool deleteit = false);
    void handle_getcs( Msg *m );
    void handle_job_begin( Msg *m );
    void handle_job_end( Msg *m );

    StatusView *m_view;
    JobList m_rememberedJobs;
    MsgChannel *scheduler;
    QSocketNotifier *scheduler_read;
};

#endif // MON_KDE_H
// vim:ts=4:sw=4:noet

#ifndef MON_KDE_H
#define MON_KDE_H

#include <kmainwindow.h>
#include <time.h>
#include <qdict.h>
#include <klistview.h>

template <class T> class QValueList;

class QCanvas;
class QCanvasText;
class QCanvasView;
class NodeItem;

class Job
{
public:
    enum State { WaitingForCS, Compiling, Finished, Failed };
    Job(unsigned int id = 0,
        const QString &client = QString::null,
        const QString &filename = QString::null,
        const QString &environment = QString::null,
        const QString &lang = QString::null)
    {
        m_id = id;
        m_fileName = filename;
        m_env = environment;
        m_lang = lang;
        m_state = WaitingForCS;
        m_client = client;
        real_msec = 0;
        user_msec = 0;
        sys_msec = 0;
        maxrss = 0;
        idrss = 0;
        majflt = 0;
        nswap = 0;
        exitcode = 0;
        in_compressed = in_uncompressed = out_compressed = out_uncompressed = 0;
    }

    bool operator==( const Job &rhs ) const { return m_id == rhs.m_id; }
    bool operator!=( const Job &rhs ) const { return m_id != rhs.m_id; }

    unsigned int jobId() const { return m_id; }
    QString fileName() const { return m_fileName; }
    QString client() const { return m_client; }
    QString server() const { return m_server; }
    State state() const { return m_state; }
    QString stateAsString() const;
    time_t stime() const { return m_stime; }

    void setServer( const QString & host ) {
        m_server = host;
    }
    void setStartTime( time_t t ) {
        m_stime = t;
    }
    void setState( State s ) {
        m_state = s;
    }
private:
    unsigned int m_id;
    QString m_fileName;
    QString m_server;
    QString m_client;
    QString m_lang;
    QString m_env;
    State m_state;
    time_t m_stime;
public:
    unsigned int real_msec;  /* real time it used */
    unsigned int user_msec;  /* user time used */
    unsigned int sys_msec;   /* system time used */
    unsigned int maxrss;     /* maximum resident set size (KB) */
    unsigned int idrss;      /* integral unshared data size (KB) */
    unsigned int majflt;     /* page faults */
    unsigned int nswap;      /* swaps */

    int exitcode;            /* exit code */

    unsigned int in_compressed;
    unsigned int in_uncompressed;
    unsigned int out_compressed;
    unsigned int out_uncompressed;

};

class JobList : public QMap<unsigned int, Job>
{
public:
    JobList() { }
};

class StatusView
{
public:
    virtual ~StatusView() {}
    virtual void update( const Job &job ) = 0;
    virtual QWidget *widget() = 0;
};

class ListStatusViewItem : public QListViewItem
{
public:
    ListStatusViewItem( QListView *parent, const Job &job );
    void updateText( const Job &job);
    int compare( QListViewItem *i, int col, bool ascending ) const;
private:
    Job job;
};

class ListStatusView :public KListView, public StatusView
{
    Q_OBJECT
public:
    ListStatusView( QWidget *parent, const char *name = 0 );
    virtual ~ListStatusView() {}
    virtual QWidget *widget() { return this; }
    virtual void update( const Job &job );

    typedef QMap<unsigned int, ListStatusViewItem*> ItemMap;
private:
    ItemMap items;
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
    void setupGanttView();

    void rememberJobs( const JobList &jobs );
    void slotCheckScheduler();
    void msgReceived();

private:
    void setupView( StatusView *view );
    void checkScheduler(bool deleteit = false);
    void handle_getcs( Msg *m );
    void handle_job_begin( Msg *m );
    void handle_job_done( Msg *m );
    void handle_stats( Msg *m );

    StatusView *m_view;
    JobList m_rememberedJobs;
    MsgChannel *scheduler;
    QSocketNotifier *scheduler_read;
    QString current_netname;
};

#endif // MON_KDE_H
// vim:ts=4:sw=4:noet

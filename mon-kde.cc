#include "mon-kde.h"

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdaction.h>

#include <qcanvas.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qvaluelist.h>
#include <math.h>

#include <iostream>
#include <comm.h>

using namespace std;

class NodeItem : public QCanvasText
{
	public:
		NodeItem( const QString &hostName, QCanvas *canvas )
			: QCanvasText( hostName, canvas ),
                          m_hostName( hostName ),
                          m_stateItem( 0 )
		{
		}

		void setState( Job::State state ) { m_state = state; }
		Job::State state() const { return m_state; }

		void setStateItem( QCanvasItem *item ) { m_stateItem = item; }
		QCanvasItem *stateItem() { return m_stateItem; }

		QString hostName() const { return m_hostName; }

	private:
		Job::State m_state;
		QString m_hostName;
		QCanvasItem *m_stateItem;
};

QString Job::stateAsString( State state )
{
#if 0
	switch ( state ) {
		case Compile: return QString::fromLatin1( STATE_COMPILE );
		case Connect: return QString::fromLatin1( STATE_CONNECT );
		case Send   : return QString::fromLatin1( STATE_SEND );
		case Receive: return QString::fromLatin1( STATE_RECEIVE );
		case Blocked: return QString::fromLatin1( STATE_BLOCKED );
		case Startup: return QString::fromLatin1( STATE_STARTUP );
		case CPP    : return QString::fromLatin1( STATE_CPP );
		case Unknown: return i18n( "Unknown" );
	}
#endif
	return QString::null;
}

Job::State Job::stringAsState( const QString &s )
{
#if 0
	if ( s == QString::fromLatin1( STATE_COMPILE ) )
		return Compile;
	else if ( s == QString::fromLatin1( STATE_CONNECT ) )
		return Connect;
	else if ( s == QString::fromLatin1( STATE_SEND ) )
		return Send;
	else if ( s == QString::fromLatin1( STATE_RECEIVE ) )
		return Receive;
	else if ( s == QString::fromLatin1( STATE_BLOCKED ) )
		return Blocked;
	else if ( s == QString::fromLatin1( STATE_STARTUP ) )
		return Startup;
	else if ( s == QString::fromLatin1( STATE_CPP ) )
		return CPP;
#endif
	return Unknown;
}

#if 0
Job::Job( const struct dcc_mon_list * const job )
	: m_pid( job->cpid ),
	m_fileName( QString::fromLatin1( job->file ) ),
	m_host( QString::fromLatin1( job->host ) ),
	m_state( stringAsState( QString::fromLatin1( job->state ) ) )
{
}
#endif

bool Job::operator==( const Job &rhs ) const
{
	return m_pid == rhs.m_pid &&
	       m_fileName == rhs.m_fileName &&
	       m_host == rhs.m_host &&
	       m_state == rhs.m_state;
}

#if 0
JobList::JobList( const struct dcc_mon_list * const list )
	: QValueList<Job>()
{
	for ( const struct dcc_mon_list *it = list; it != 0; it = it->next )
		append( Job( it ) );
}
#endif

bool JobList::operator==( const JobList &rhs ) const
{
	if ( count() != rhs.count() )
		return false;

	JobList::ConstIterator it = begin();
	for ( ; it != end(); ++it )
		if ( rhs.find( rhs.begin(), *it ) == rhs.end() )
			return false;

	return true;
}

StatusObserver::StatusObserver( QObject *parent, const char *name )
	: QObject( parent, name )
{
	setupErrorMessages();

	m_timer = new QTimer( this );
	connect( m_timer, SIGNAL( timeout() ), this, SLOT( pollTimeout() ) );
}

void StatusObserver::setupErrorMessages()
{
#if 0
	m_errorMessages[ EXIT_DISTCC_FAILED ] = i18n( "General failure" );
	m_errorMessages[ EXIT_BAD_ARGUMENTS ] = i18n( "Bad arguments" );
	m_errorMessages[ EXIT_BIND_FAILED ] = i18n( "Bind failed" );
	m_errorMessages[ EXIT_CONNECT_FAILED ] = i18n( "Connect failed" );
	m_errorMessages[ EXIT_COMPILER_CRASHED ] = i18n( "Compiler crashed" );
	m_errorMessages[ EXIT_OUT_OF_MEMORY ] = i18n( "Out of memory" );
	m_errorMessages[ EXIT_BAD_HOSTSPEC ] = i18n( "Bad hostspec" );
	m_errorMessages[ EXIT_IO_ERROR ] = i18n( "IO error" );
	m_errorMessages[ EXIT_TRUNCATED ] = i18n( "Truncated" );
	m_errorMessages[ EXIT_PROTOCOL_ERROR ] = i18n( "Protocol error" );
	m_errorMessages[ EXIT_COMPILER_MISSING ] =
		i18n( "Compiler missing - compiler executable not found!" );
	m_errorMessages[ EXIT_RECURSION ] =
		i18n( "Recursion error - distcc called itself!" );
	m_errorMessages[ EXIT_SETUID_FAILED ] =
		i18n( "setuid failed - could not discard privileges!" );
	m_errorMessages[ EXIT_ACCESS_DENIED ] =
		i18n( "Network access denied" );
	m_errorMessages[ EXIT_BUSY ] = i18n( "Busy" );
	m_errorMessages[ EXIT_NO_SUCH_FILE ] = i18n( "No such file" );
	m_errorMessages[ EXIT_NO_HOSTS ] = i18n( "No hosts" );
#endif
}

void StatusObserver::start()
{
	m_timer->start( 1000 );
}

void StatusObserver::stop()
{
	m_timer->stop();
}

void StatusObserver::pollTimeout()
{
#if 0
	struct dcc_mon_list *list;
	const int result = dcc_mon_poll( &list );
	if ( result != 0 ) {
		const dcc_exitcode exitcode = static_cast<dcc_exitcode>( result );
		KMessageBox::error( kapp->mainWidget(), m_errorMessages[ exitcode ] );
		return;
	}

	JobList jobs( list );
	dcc_mon_list_free( list );
	if ( m_prevJobs != jobs ) {
		emit statusChanged( jobs );
		m_prevJobs = jobs;
	}
#endif
}

StatusView::StatusView( QWidget *parent, const char *name, WFlags f )
	: QWidget( parent, name, f )
{
}

ListStatusView::ListStatusView( QWidget *parent, const char *name )
	: StatusView( parent, name, WRepaintNoErase | WResizeNoErase )
{
	QHBoxLayout *layout = new QHBoxLayout( this );
	layout->setMargin( 0 );

	m_listView = new KListView( this );
	m_listView->addColumn( i18n( "PID" ) );
	m_listView->addColumn( i18n( "Filename" ) );
	m_listView->addColumn( i18n( "Host" ) );
	m_listView->addColumn( i18n( "State" ) );
	layout->addWidget( m_listView );
}

void ListStatusView::update( const JobList &jobs )
{
	m_listView->setUpdatesEnabled( false );
	m_listView->clear();
	JobList::ConstIterator it = jobs.begin();
	for ( ; it != jobs.end(); ++it )
		new KListViewItem( m_listView, QString::number( ( *it ).pid() ),
		                   ( *it ).fileName(), ( *it ).host(),
		                   Job::stateAsString( ( *it ).state() ) );
	m_listView->setUpdatesEnabled( true );
	m_listView->triggerUpdate();
}

StarStatusView::StarStatusView( QWidget *parent, const char *name )
	: StatusView( parent, name, WRepaintNoErase | WResizeNoErase )
{
	m_canvas = new QCanvas( this );
	m_canvas->resize( width(), height() );

	QHBoxLayout *layout = new QHBoxLayout( this );
	layout->setMargin( 0 );

	m_canvasView = new QCanvasView( m_canvas, this );
	m_canvasView->setVScrollBarMode( QScrollView::AlwaysOff );
	m_canvasView->setHScrollBarMode( QScrollView::AlwaysOff );
	layout->addWidget( m_canvasView );

	m_localhostItem = new QCanvasText( i18n( "localhost" ), m_canvas );
	centerLocalhostItem();
	m_localhostItem->show();

	m_canvas->update();
}

void StarStatusView::update( const JobList &jobs )
{
	checkForNewNodes( jobs );
	updateNodeStatus( jobs );
	drawNodeStatus();
}

void StarStatusView::resizeEvent( QResizeEvent * )
{
	m_canvas->resize( width(), height() );
	centerLocalhostItem();
	arrangeNodeItems();
	m_canvas->update();
}

void StarStatusView::centerLocalhostItem()
{
	const QRect br = m_localhostItem->boundingRect();
	const int newX = ( width() - br.width() ) / 2;
	const int newY = ( height() - br.height() ) / 2;
	m_localhostItem->move( newX, newY );
}

void StarStatusView::arrangeNodeItems()
{
	const int radius = kMin( m_canvas->width() / 2, m_canvas->height() / 2 );
	const double step = 2 * M_PI / m_nodeItems.count();

	double angle = 0.0;
	QDictIterator<NodeItem> it( m_nodeItems );
	while ( it.current() != 0 ) {
		it.current()->move( m_localhostItem->x() + ( cos( angle ) * radius ),
		                    m_localhostItem->y() + ( sin( angle ) * radius ) );
		angle += step;
		++it;
	}
}

void StarStatusView::checkForNewNodes( const JobList &jobs )
{
	bool newNode = false;

	JobList::ConstIterator it = jobs.begin();
	for ( ; it != jobs.end(); ++it ) {
		if ( m_nodeItems.find( ( *it ).host() ) == 0 ) {
			NodeItem *nodeItem = new NodeItem( ( *it ).host(), m_canvas );
			m_nodeItems.insert( ( *it ).host(), nodeItem );
			nodeItem->show();
			newNode = true;
		}
	}

	if ( newNode ) {
		arrangeNodeItems();
		m_canvas->update();
	}
}

void StarStatusView::updateNodeStatus( const JobList &jobs )
{
	QDictIterator<NodeItem> it( m_nodeItems );
	while ( it.current() != 0 ) {
		JobList::ConstIterator jobIt = jobs.begin();
		it.current()->setState( Job::Unknown );
		for ( ; jobIt != jobs.end(); ++jobIt ) {
			if ( it.current()->hostName() == ( *jobIt ).host() )
				it.current()->setState( ( *jobIt ).state() );
		}
		++it;
	}
}

void StarStatusView::drawNodeStatus()
{
	QDictIterator<NodeItem> it( m_nodeItems );
	for ( ; it.current() != 0; ++it )
		drawState( it.current() );
	m_canvas->update();
}

void StarStatusView::drawState( NodeItem *node )
{
	delete node->stateItem();
	QCanvasItem *newItem = 0;

	const QPoint nodeCenter = node->boundingRect().center();
	const QPoint localCenter = m_localhostItem->boundingRect().center();

	switch ( node->state() ) {
		case Job::Compile: {
			QCanvasLine *line = new QCanvasLine( m_canvas );
			line->setPen( Qt::green );

			line->setPoints( nodeCenter.x(), nodeCenter.y(),
			                 localCenter.x(), localCenter.y() );
			line->show();
			newItem = line;
			break;
		}
		case Job::CPP: {
			QCanvasLine *line = new QCanvasLine( m_canvas );
			line->setPen( QPen( Qt::darkGreen, 0, QPen::DashLine ) );

			line->setPoints( nodeCenter.x(), nodeCenter.y(),
			                 localCenter.x(), localCenter.y() );
			line->show();
			newItem = line;
			break;
		}
		case Job::Send: {
			QPointArray points( 3 );
			points.setPoint( 0, localCenter.x() - 5, localCenter.y() );
			points.setPoint( 1, nodeCenter );
			points.setPoint( 2, localCenter.x() + 5, localCenter.y() );

			QCanvasPolygon *poly = new QCanvasPolygon( m_canvas );
			poly->setBrush( Qt::blue );
			poly->setPoints( points );
			poly->show();

			newItem = poly;
			break;
		}
		case Job::Receive: {
			QPointArray points( 3 );
			points.setPoint( 0, nodeCenter.x() - 5, nodeCenter.y() );
			points.setPoint( 1, localCenter );
			points.setPoint( 2, nodeCenter.x() + 5, nodeCenter.y() );

			QCanvasPolygon *poly = new QCanvasPolygon( m_canvas );
			poly->setBrush( Qt::blue );
			poly->setPoints( points );
			poly->show();

			newItem = poly;
			break;
		}
		default: {
			qDebug( "Unhandled state: %s", Job::stateAsString( node->state() ).latin1() );
			break;
		}
	}

	node->setStateItem( newItem );
}

MainWindow::MainWindow( QWidget *parent, const char *name )
	: KMainWindow( parent, name ), m_view( 0 )
{
	KRadioAction *a = new KRadioAction( i18n( "&List View" ), 0,
	                                    this, SLOT( setupListView() ),
	                                    actionCollection(), "view_list_view" );
	a->setExclusiveGroup( "viewmode" );

	a = new KRadioAction( i18n( "&Star View" ), 0,
	                      this, SLOT( setupStarView() ),
	                      actionCollection(), "view_star_view" );
	a->setExclusiveGroup( "viewmode" );

	KStdAction::quit( kapp, SLOT( quit() ), actionCollection() );

	createGUI();

	m_observer = new StatusObserver( this );
	connect( m_observer, SIGNAL( statusChanged( const JobList & ) ),
	         this, SLOT( rememberJobs( const JobList & ) ) );

	setupListView();

	m_observer->start();
}

void MainWindow::setupView( StatusView *view )
{
	delete m_view;
	m_view = view;
	connect( m_observer, SIGNAL( statusChanged( const JobList & ) ),
	         m_view, SLOT( update( const JobList & ) ) );
	m_view->update( m_rememberedJobs );
	setCentralWidget( m_view );
	m_view->show();
}

void MainWindow::setupListView()
{
	setupView( new ListStatusView( this ) );
}

void MainWindow::setupStarView()
{
	setupView( new StarStatusView( this ) );
}

void MainWindow::rememberJobs( const JobList &jobs )
{
	m_rememberedJobs = jobs;
}

const char * rs_program_name = "distccmon-kde";
const char * const appName = I18N_NOOP( "distccmon-kde" );
const char * const version = "0.1";
const char * const description = I18N_NOOP( "distcc monitor for KDE" );
const char * const copyright = I18N_NOOP( "(c) 2003, Frerich Raabe <raabe@kde.org>" );
const char * const bugsEmail = "raabe@kde.org";

int main( int argc, char **argv )
{
	KAboutData aboutData( rs_program_name, appName, version, description,
	                      KAboutData::License_BSD, copyright, bugsEmail );
	KCmdLineArgs::init( argc, argv, &aboutData );

	KApplication app;
	MainWindow *mainWidget = new MainWindow( 0 );
	app.setMainWidget( mainWidget );
	mainWidget->show();

	MsgChannel *scheduler = connect_scheduler ();
	if (scheduler
	    && scheduler->send_msg (MonLoginMsg()))
	  while (Msg *m = scheduler->get_msg ())
	    {
	      switch (m->type)
	        {
		case M_MON_GET_CS: cout << "GET_CS" << endl; break;
		case M_MON_JOB_BEGIN: cout << "JOB_BEGIN" << endl; break;
		case M_MON_JOB_END: cout << "JOB_END" << endl; break;
		case M_END: cout << "END" << endl; m = 0; break;
		default: cout << "UNKNOWN" << endl; break;
		}
	      if (!m)
	        break;
	    }
	delete scheduler;

	return app.exec();
}

#include "mon-kde.moc"

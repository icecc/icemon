#include "mon-kde.h"

#include <qsocketnotifier.h>
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

QString Job::stateAsString() const
{
    switch ( m_state ) {
    case WaitingForCS:
        return "Waiting";
        break;
    case Compiling:
        return "Compiling";
        break;
    case Finished:
        return "Finished";
        break;
    }
    return QString::null;
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
    m_listView->addColumn( i18n( "ID" ) );
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
        new KListViewItem( m_listView, QString::number( ( *it ).jobId() ),
                           ( *it ).fileName(), ( *it ).host(), ( *it ).stateAsString() );
    m_listView->setUpdatesEnabled( true );
    m_listView->triggerUpdate();
}

#if 0
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
	}

	node->setStateItem( newItem );
}
#endif

MainWindow::MainWindow( QWidget *parent, const char *name )
	: KMainWindow( parent, name ), m_view( 0 ),  scheduler( 0 ), scheduler_read( 0 )
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
    setupListView();
    checkScheduler();
}

void MainWindow::checkScheduler(bool deleteit)
{
    if ( deleteit ) {
        delete scheduler;
        scheduler = 0;
        delete scheduler_read;
        scheduler_read = 0;
    } else if ( scheduler )
        return;
    QTimer::singleShot( 1000, this, SLOT( slotCheckScheduler() ) );
}

void MainWindow::slotCheckScheduler()
{
    scheduler = connect_scheduler ();
    if ( scheduler ) {
        if ( !scheduler->send_msg (MonLoginMsg()) )
        {
            checkScheduler( true );
            return;
        }
        scheduler_read = new QSocketNotifier( scheduler->fd,
                                              QSocketNotifier::Read,
                                              this );
        QObject::connect( scheduler_read, SIGNAL(activated(int)),
                          SLOT( msgReceived()) );
    }
}

void MainWindow::msgReceived()
{
    Msg *m = scheduler->get_msg ();
    if ( !m ) {
        checkScheduler(true);
        return;
    }

    switch (m->type) {
    case M_MON_GET_CS:
        handle_getcs( m );
        break;
    case M_MON_JOB_BEGIN:
        handle_job_begin( m );
        break;
    case M_MON_JOB_END:
        handle_job_end( m );
        break;
    case M_END:
        cout << "END" << endl;
        checkScheduler( true );
        break;
    default:
        cout << "UNKNOWN" << endl;
        break;
    }
    delete m;
}

void MainWindow::handle_getcs(Msg *_m)
{
    MonGetCSMsg *m = dynamic_cast<MonGetCSMsg*>( _m );
    if ( !m )
        return;
    m_rememberedJobs.append( Job( m->job_id, m->filename.c_str(),
                                  m->version.c_str(),
                                  m->lang == CompileJob::Lang_C ? "C" : "C++" ) );
    m_view->update( m_rememberedJobs );
}

void MainWindow::handle_job_begin(Msg *_m)
{
    MonJobBeginMsg *m = dynamic_cast<MonJobBeginMsg*>( m );
    if ( !m )
        return;
}

void MainWindow::handle_job_end(Msg *m)
{
    MonJobEndMsg *msg = dynamic_cast<MonJobEndMsg*>( m );
    if ( !msg )
        return;
}

void MainWindow::setupView( StatusView *view )
{
    delete m_view;
    m_view = view;
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
//	setupView( new StarStatusView( this ) );
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

    	return app.exec();
}

#include "mon-kde.moc"

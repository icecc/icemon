/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>
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

#include "mon-kde.h"
#include "summaryview.h"
#include "ganttstatusview.h"
#include "listview.h"
#include "starview.h"
#include "hostinfo.h"

#include <services/logging.h>
#include <services/comm.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdaction.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>

#include <qsocketnotifier.h>
#include <qcanvas.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qvaluelist.h>

#include <math.h>
#include <iostream>
#include <cassert>

using namespace std;

MainWindow::MainWindow( QWidget *parent, const char *name )
	: KMainWindow( parent, name ), m_view( 0 ), m_scheduler( 0 ),
          m_scheduler_read( 0 )
{
    m_hostInfoManager = new HostInfoManager;

    KRadioAction *a = new KRadioAction( i18n( "&List View" ), 0,
                                        this, SLOT( setupListView() ),
                                        actionCollection(), "view_list_view" );
    a->setExclusiveGroup( "viewmode" );

    a = new KRadioAction( i18n( "&Star View" ), 0,
                          this, SLOT( setupStarView() ),
                          actionCollection(), "view_star_view" );
    a->setExclusiveGroup( "viewmode" );

    a = new KRadioAction( i18n( "&Gantt View" ), 0,
                          this, SLOT( setupGanttView() ),
                          actionCollection(), "view_gantt_view" );
    a->setExclusiveGroup( "viewmode" );

    a = new KRadioAction( i18n( "&Summary View" ), 0,
                          this, SLOT( setupSummaryView() ),
                          actionCollection(), "view_foo_view" );
    a->setExclusiveGroup( "viewmode" );

    KStdAction::quit( kapp, SLOT( quit() ), actionCollection() );

    new KAction( i18n("Stop"), 0, this, SLOT( stopView() ), actionCollection(),
                 "view_stop" );

    new KAction( i18n("Start"), 0, this, SLOT( startView() ),
                 actionCollection(), "view_start" );

    new KAction( i18n("Check Nodes"), 0, this, SLOT( checkNodes() ),
                 actionCollection(), "check_nodes" );

    new KAction( i18n("Configure View"), 0, this, SLOT( configureView() ),
                 actionCollection(), "configure_view" );

    createGUI();
    readSettings();
    checkScheduler();

    setAutoSaveSettings();
}

MainWindow::~MainWindow()
{
  writeSettings();

  delete m_hostInfoManager;
}

void MainWindow::readSettings()
{
  KConfig *cfg = KGlobal::config();
  cfg->setGroup( "View" );
  QString viewId = cfg->readEntry( "CurrentView", "star" );
  if ( viewId == "gantt" ) setupGanttView();
  else if ( viewId == "list" ) setupListView();
  else if ( viewId == "star" ) setupStarView();
  else setupSummaryView();
}

void MainWindow::writeSettings()
{
  KConfig *cfg = KGlobal::config();
  cfg->setGroup( "View" );
  cfg->writeEntry( "CurrentView", m_view->id() );
}

void MainWindow::checkScheduler(bool deleteit)
{
    if ( deleteit ) {
        m_rememberedJobs.clear();
        delete m_scheduler;
        m_scheduler = 0;
        delete m_scheduler_read;
        m_scheduler_read = 0;
    } else if ( m_scheduler )
        return;
    QTimer::singleShot( 1800, this, SLOT( slotCheckScheduler() ) );
}

void MainWindow::slotCheckScheduler()
{
    list<string> names = get_netnames (60);
    if ( names.empty() ) {
        checkScheduler( true );
        return;
    }

    if ( !m_current_netname.isEmpty() )
        names.push_front( m_current_netname.latin1() );

    for ( list<string>::const_iterator it = names.begin(); it != names.end(); ++it )
    {
        m_current_netname = it->c_str();
        m_scheduler = connect_scheduler ( m_current_netname.latin1() );
        if ( m_scheduler ) {
            if ( !m_scheduler->send_msg (MonLoginMsg()) )
            {
                delete m_scheduler;
            } else {
                m_scheduler_read = new QSocketNotifier( m_scheduler->fd,
                                                        QSocketNotifier::Read,
                                                        this );
                QObject::connect( m_scheduler_read, SIGNAL(activated(int)),
                                  SLOT( msgReceived()) );
                return;
            }
        }
    }
    checkScheduler( true );
}

void MainWindow::msgReceived()
{
    Msg *m = m_scheduler->get_msg ();
    if ( !m ) {
        kdDebug() << "lost connection to scheduler\n";
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
    case M_MON_JOB_DONE:
        handle_job_done( m );
        break;
    case M_END:
        cout << "END" << endl;
        checkScheduler( true );
        break;
    case M_MON_STATS:
        handle_stats( m );
        break;
    case M_MON_LOCAL_JOB_BEGIN:
        handle_local_begin( m );
        break;
    case M_MON_LOCAL_JOB_DONE:
        handle_local_done( m );
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
    m_rememberedJobs[m->job_id] = Job( m->job_id, m->clientid,
                                       m->filename.c_str(),
                                       m->lang == CompileJob::Lang_C ? "C" : "C++" );
    m_view->update( m_rememberedJobs[m->job_id] );
}

void MainWindow::handle_local_begin( Msg *_m )
{
    MonLocalJobBeginMsg *m = dynamic_cast<MonLocalJobBeginMsg*>( _m );
    if ( !m )
        return;

    m_rememberedJobs[m->job_id] = Job( m->job_id, m->hostid,
                                       m->file,
                                       "C++" );
    m_rememberedJobs[m->job_id].setState( Job::LocalOnly );
    m_view->update( m_rememberedJobs[m->job_id] );
}

void MainWindow::handle_local_done( Msg *_m )
{
    MonLocalJobDoneMsg *m = dynamic_cast<MonLocalJobDoneMsg*>( _m );
    if ( !m )
        return;
    JobList::iterator it = m_rememberedJobs.find( m->job_id );
    if ( it == m_rememberedJobs.end() ) // we started in between
        return;

    ( *it ).exitcode = m->exitcode;
    ( *it ).setState( Job::Finished );
    m_view->update( *it );
}

void MainWindow::handle_stats( Msg *_m )
{
    MonStatsMsg *m = dynamic_cast<MonStatsMsg*>( _m );
    if ( !m )
        return;

    QStringList statmsg = QStringList::split( '\n', m->statmsg.c_str() );
    HostInfo::StatsMap stats;
    for ( QStringList::ConstIterator it = statmsg.begin(); it != statmsg.end(); ++it )
    {
        QString key = *it;
        key = key.left( key.find( ':' ) );
        QString value = *it;
        value = value.mid( value.find( ':' ) + 1 );
        stats[key] = value;
    }

    HostInfo *hostInfo = m_hostInfoManager->checkNode( m->hostid, stats );
    if ( hostInfo->isOffline() ) {
      m_view->removeNode( m->hostid );
    } else {
      m_view->checkNode( m->hostid );
    }
}

void MainWindow::handle_job_begin(Msg *_m)
{
    MonJobBeginMsg *m = dynamic_cast<MonJobBeginMsg*>( _m );
    if ( !m )
        return;
    JobList::iterator it = m_rememberedJobs.find( m->job_id );
    if ( it == m_rememberedJobs.end() ) // we started in between
        return;
    ( *it ).setServer( m->hostid );
    ( *it ).setStartTime( m->stime );
    ( *it ).setState( Job::Compiling );

#if 0
    kdDebug() << "BEGIN: " << (*it).fileName() << " (" << (*it).jobId()
              << ")" << endl;
#endif

    m_view->update( *it );
}

void MainWindow::handle_job_done(Msg *_m)
{
    MonJobDoneMsg *m = dynamic_cast<MonJobDoneMsg*>( _m );
    if ( !m )
        return;
    JobList::iterator it = m_rememberedJobs.find( m->job_id );
    if ( it == m_rememberedJobs.end() ) // we started in between
        return;

    ( *it ).exitcode = m->exitcode;
    if ( m->exitcode )
        ( *it ).setState( Job::Failed );
    else {
        ( *it ).setState( Job::Finished );
        ( *it ).real_msec = m->real_msec;
        ( *it ).user_msec = m->user_msec;
        ( *it ).sys_msec = m->sys_msec;   /* system time used */
        ( *it ).maxrss = m->maxrss;     /* maximum resident set size (KB) */
        ( *it ).idrss = m->idrss;      /* integral unshared data size (KB) */
        ( *it ).majflt = m->majflt;     /* page faults */
        ( *it ).nswap = m->nswap;      /* swaps */

        ( *it ).in_compressed = m->in_compressed;
        ( *it ).in_uncompressed = m->in_uncompressed;
        ( *it ).out_compressed = m->out_compressed;
        ( *it ).out_uncompressed = m->out_uncompressed;
    }

#if 0
    kdDebug() << "DONE: " << (*it).fileName() << " (" << (*it).jobId()
              << ")" << endl;
#endif

    m_view->update( *it );
}

void MainWindow::setupView( StatusView *view, bool rememberJobs )
{
    delete m_view;
    m_view = view;
    if ( rememberJobs ) {
      JobList::ConstIterator it = m_rememberedJobs.begin();
      for ( ; it != m_rememberedJobs.end(); ++it )
          m_view->update( *it );
    }
    setCentralWidget( m_view->widget() );
    m_view->widget()->show();
}

void MainWindow::setupListView()
{
    setupView( new ListStatusView( m_hostInfoManager, this ), true );
}

void MainWindow::setupSummaryView()
{
    setupView( new SummaryView( m_hostInfoManager, this ), false );
}

void MainWindow::setupGanttView()
{
    setupView( new GanttStatusView( m_hostInfoManager, this ), false );
}

void MainWindow::setupStarView()
{
    setupView( new StarView( m_hostInfoManager, this ), false );
}

void MainWindow::stopView()
{
  m_view->stop();
}

void MainWindow::startView()
{
  m_view->start();
}

void MainWindow::checkNodes()
{
  m_view->checkNodes();
}

void MainWindow::configureView()
{
  m_view->configureView();
}

void MainWindow::setCurrentNet( const QString &netName )
{
  m_current_netname = netName;
}

const char * rs_program_name = "icemon";
const char * const appName = I18N_NOOP( "icemon" );
const char * const version = "0.1";
const char * const description = I18N_NOOP( "distcc monitor for KDE" );
const char * const copyright = I18N_NOOP( "(c) 2003, Frerich Raabe <raabe@kde.org>" );
const char * const bugsEmail = "raabe@kde.org";

static const KCmdLineOptions options[] =
{
  { "n", 0, 0 },
  { "netname <name>", "Icecream network name", 0 },
  KCmdLineLastOption
};

int main( int argc, char **argv )
{
 	setup_debug(Debug|Info|Warning|Error,"");
	KAboutData aboutData( rs_program_name, appName, version, description,
	                      KAboutData::License_BSD, copyright, bugsEmail );
	KCmdLineArgs::init( argc, argv, &aboutData );
        KCmdLineArgs::addCmdLineOptions( options );

	KApplication app;
	MainWindow *mainWidget = new MainWindow( 0 );

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        QString netName = QString::fromLocal8Bit( args->getOption( "netname" ) );
        if ( !netName.isEmpty() ) {
          mainWidget->setCurrentNet( netName );
        }

	app.setMainWidget( mainWidget );
	mainWidget->show();

    	return app.exec();
}

#include "mon-kde.moc"

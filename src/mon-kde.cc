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

#include "detailedhostview.h"
#include "ganttstatusview.h"
#include "hostinfo.h"
#include "listview.h"
#include "monitor.h"
#include "starview.h"
#include "summaryview.h"

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <kselectaction.h>
#include <kmenubar.h>

#include <QMenu>

MainWindow::MainWindow( QWidget *parent, const char *name )
  : KXmlGuiWindow( parent ), m_view( 0 )
{
    setObjectName( name );

    m_hostInfoManager = new HostInfoManager;

    m_monitor = new Monitor( m_hostInfoManager, this );

    m_viewMode = new KSelectAction(this);
    m_viewMode->setText(i18n("&Mode"));
    actionCollection()->addAction("view_mode", m_viewMode);

    QAction* action = m_viewMode->addAction(i18n( "&List View" ));
    connect( action, SIGNAL( triggered() ), this, SLOT( setupListView() ) );

    action = m_viewMode->addAction(i18n( "&Star View" ));
    connect( action, SIGNAL( triggered() ), this, SLOT( setupStarView() ) );

    action = m_viewMode->addAction(i18n( "&Gantt View" ));
    connect( action, SIGNAL( triggered() ), this, SLOT( setupGanttView() ) );

    action = m_viewMode->addAction(i18n( "Summary &View" ));
    connect( action, SIGNAL( triggered() ), this, SLOT( setupSummaryView() ) );

    action = m_viewMode->addAction(i18n( "&Detailed Host View" ));
    connect( action, SIGNAL( triggered() ), this, SLOT( setupDetailedHostView() ) );

    KStandardAction::quit( this, SLOT( close() ), actionCollection() );

    action = actionCollection()->addAction("view_stop");
    action->setText(i18n("Stop"));
    connect( action, SIGNAL( triggered() ), this, SLOT( stopView() ) );

    action = actionCollection()->addAction("view_start");
    action->setText(i18n("Start"));
    connect( action, SIGNAL( triggered() ), this, SLOT( startView() ) );

    action = actionCollection()->addAction("check_nodes");
    action->setText(i18n("Check Nodes"));
    connect( action, SIGNAL( triggered() ), this, SLOT( checkNodes() ) );

    action = actionCollection()->addAction("configure_view");
    action->setText(i18n("Configure View..."));
    connect( action, SIGNAL( triggered() ), this, SLOT( configureView() ) );

    setupGUI();
    readSettings();
}

MainWindow::~MainWindow()
{
  writeSettings();

  delete m_hostInfoManager;
}

void MainWindow::readSettings()
{
  KConfigGroup cfg(KGlobal::config(), "View" );
  QString viewId = cfg.readEntry( "CurrentView", "star" );

  m_viewMode->blockSignals(true);
  if ( viewId == "gantt" ) {
    setupGanttView();
    m_viewMode->setCurrentAction(m_viewMode->actions()[GanttViewType]);

  } else if ( viewId == "list" ) {
    setupListView();
    m_viewMode->setCurrentAction(m_viewMode->actions()[ListViewType]);

  } else if ( viewId == "star" ) {
    setupStarView();
    m_viewMode->setCurrentAction(m_viewMode->actions()[StarViewType]);

  } else if ( viewId == "detailedhost" ) {
    setupDetailedHostView();
    m_viewMode->setCurrentAction(m_viewMode->actions()[DetailedHostViewType]);

  } else {
    setupSummaryView();
    m_viewMode->setCurrentAction(m_viewMode->actions()[SummaryViewType]);
  }
  m_viewMode->blockSignals(false);
}

void MainWindow::writeSettings()
{
  KConfigGroup cfg(KGlobal::config(), "View" );
  cfg.writeEntry( "CurrentView", m_view->id() );
}

void MainWindow::setupView( StatusView *view, bool rememberJobs )
{
  delete m_view;
  m_view = view;
  m_monitor->setCurrentView( m_view, rememberJobs );
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
    QAction* radioAction = actionCollection()->action( "view_foo_view" );
    if ( radioAction )
        dynamic_cast<KToggleAction*>( radioAction )->setChecked( true );
}

void MainWindow::setupGanttView()
{
    setupView( new GanttStatusView( m_hostInfoManager, this ), false );
    QAction* radioAction = actionCollection()->action( "view_gantt_view" );
    if ( radioAction )
        dynamic_cast<KToggleAction*>( radioAction )->setChecked( true );
}

void MainWindow::setupStarView()
{
    setupView( new StarView( m_hostInfoManager, this ), false );
    QAction* radioAction = actionCollection()->action( "view_star_view" );
    if ( radioAction )
        dynamic_cast<KToggleAction*>( radioAction )->setChecked( true );
}

void MainWindow::setupDetailedHostView()
{
    setupView( new DetailedHostView( m_hostInfoManager, this ), false );
    QAction* radioAction = actionCollection()->action( "view_detailed_host_view" );
    if ( radioAction )
        dynamic_cast<KToggleAction*>( radioAction )->setChecked( true );
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
  m_monitor->setCurrentNet( netName );
}

const char * rs_program_name = "icemon";
const char * const appName = I18N_NOOP( "Icecream Monitor" );
const char * const version = "0.1";
const char * const description = I18N_NOOP( "Icecream monitor for KDE" );
const char * const copyright = I18N_NOOP( "(c) 2003,2004, The icecream developers" );

static const KCmdLineOptions options[] =
{
  { "n", 0, 0 },
  { "netname <name>", "Icecream network name", 0 },
  KCmdLineLastOption
};

int main( int argc, char **argv )
{
  KAboutData aboutData( rs_program_name, appName, version, description,
                        KAboutData::License_GPL_V2, copyright );
  aboutData.addAuthor( "Frerich Raabe", 0, "raabe@kde.org" );
  aboutData.addAuthor( "Stephan Kulow", 0, "coolo@kde.org" );
  aboutData.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication app;
  MainWindow *mainWidget = new MainWindow( 0 );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QString netName = QString::fromLocal8Bit( args->getOption( "netname" ) );
  if ( !netName.isEmpty() ) {
    mainWidget->setCurrentNet( netName );
  }

  mainWidget->show();

  return app.exec();
}

#include "mon-kde.moc"

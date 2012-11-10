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

#include "mainwindow.h"

#include "starview.h"
#include "summaryview.h"
#include "detailedhostview.h"
#include "ganttstatusview.h"
#include "listview.h"
//#include "poolview.h"
#include "flowtableview.h"
#include "hostinfo.h"
#include "monitor.h"
#include "version.h"
#include "fakemonitor.h"
#include "icecreammonitor.h"

#include <qdebug.h>

#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QSettings>

#include <QMenu>

MainWindow::MainWindow( QWidget *parent )
  : QMainWindow( parent ), m_view( 0 )
{
    QIcon appIcon = QIcon();
    appIcon.addFile(":/images/hi128-app-icemon.png", QSize(128, 128));
    appIcon.addFile(":/images/hi48-app-icemon.png", QSize(48, 48));
    appIcon.addFile(":/images/hi32-app-icemon.png", QSize(32, 32));
    appIcon.addFile(":/images/hi22-app-icemon.png", QSize(22, 22));
    appIcon.addFile(":/images/hi16-app-icemon.png", QSize(16, 16));
    setWindowIcon(appIcon);
    setWindowTitle(QApplication::translate("appName", Icemon::Version::appName));
    m_hostInfoManager = new HostInfoManager;

    m_monitor = new IcecreamMonitor( m_hostInfoManager, this );
    connect(m_monitor, SIGNAL( schedulerStateChanged( bool ) ), SLOT( setSchedulerState( bool ) ));

    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));

    m_currNetWidget = new QLabel;
    statusBar()->addPermanentWidget(m_currNetWidget);

    m_schedStatusWidget = new QLabel;
    statusBar()->addPermanentWidget(m_schedStatusWidget);

    QAction* action = fileMenu->addAction(tr("&Quit"), this, SLOT(close()), tr("Ctrl+Q"));
    action->setIcon(QIcon::fromTheme("application-exit"));
    action->setMenuRole(QAction::QuitRole);

    m_viewMode = new QActionGroup(this);

    QMenu* modeMenu = viewMenu->addMenu(tr("Mode"));

    action = m_viewMode->addAction(tr( "&List View" ));
    action->setCheckable(true);
    modeMenu->addAction(action);
    connect( action, SIGNAL( triggered() ), this, SLOT( setupListView() ) );

    action = m_viewMode->addAction(tr( "&Star View" ));
    action->setCheckable(true);
    modeMenu->addAction(action);
    connect( action, SIGNAL( triggered() ), this, SLOT( setupStarView() ) );

    action = m_viewMode->addAction(tr( "&Pool View" ));
    action->setDisabled(true); // FIXME
    action->setCheckable(true);
    modeMenu->addAction(action);
    connect( action, SIGNAL( triggered() ), this, SLOT( setupPoolView() ) );

    action = m_viewMode->addAction(tr( "&Gantt View" ));
    action->setCheckable(true);
    modeMenu->addAction(action);
    connect( action, SIGNAL( triggered() ), this, SLOT( setupGanttView() ) );

    action = m_viewMode->addAction(tr( "Summary &View" ));
    action->setCheckable(true);
    modeMenu->addAction(action);
    connect( action, SIGNAL( triggered() ), this, SLOT( setupSummaryView() ) );

    action = m_viewMode->addAction(tr( "&Flow View" ));
    action->setCheckable(true);
    modeMenu->addAction(action);
    connect( action, SIGNAL( triggered() ), this, SLOT( setupFlowTableView() ) );

    action = m_viewMode->addAction(tr( "&Detailed Host View" ));
    action->setCheckable(true);
    modeMenu->addAction(action);
    connect( action, SIGNAL( triggered() ), this, SLOT( setupDetailedHostView() ) );

    viewMenu->addSeparator();

    action = viewMenu->addAction(tr("Pause"));
    action->setIcon(QIcon::fromTheme("media-playback-pause"));
    action->setCheckable(true);
    connect( action, SIGNAL( triggered() ), this, SLOT( pauseView() ) );
    m_pauseViewAction = action;

    viewMenu->addSeparator();

    action =  viewMenu->addAction(tr("Check Nodes"));
    connect( action, SIGNAL( triggered() ), this, SLOT( checkNodes() ) );
    viewMenu->addAction(action);
    m_checkNodesAction = action;

    viewMenu->addSeparator();

    action = viewMenu->addAction(tr("Configure View..."));
    action->setIcon(QIcon::fromTheme("configure"));
    connect( action, SIGNAL( triggered() ), this, SLOT( configureView() ) );
    m_configureViewAction = action;

    action = helpMenu->addAction(tr("About Qt..."));
    connect(action, SIGNAL(triggered()), this, SLOT(aboutQt()));
    action->setMenuRole(QAction::AboutQtRole);

    action = helpMenu->addAction(tr("About..."));
    action->setIcon(appIcon);
    connect(action, SIGNAL(triggered()), this, SLOT(about()));
    action->setMenuRole(QAction::AboutRole);

    readSettings();

    setSchedulerState(m_monitor->schedulerState());
}

void MainWindow::closeEvent( QCloseEvent *e )
{
  writeSettings();
  delete m_hostInfoManager;
  e->accept();
}

void MainWindow::readSettings()
{
  QSettings settings;
  resize(600, 400);
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
  QString viewId = settings.value("currentView").toString();

  m_viewMode->blockSignals(true);
  if ( viewId == "gantt" ) {
    setupGanttView();
    (m_viewMode->actions()[GanttViewType])->setChecked(true);

  } else if ( viewId == "list" ) {
    setupListView();
    (m_viewMode->actions()[ListViewType])->setChecked(true);

  } else if ( viewId == "star" ) {
    setupStarView();
    (m_viewMode->actions()[StarViewType])->setChecked(true);

  } else if ( viewId == "pool" ) {
    setupPoolView();
    (m_viewMode->actions()[PoolViewType])->setChecked(true);

  } else if ( viewId == "detailedhost" ) {
    setupDetailedHostView();
    (m_viewMode->actions()[DetailedHostViewType])->setChecked(true);
  } else if ( viewId == "flow" ) {
    setupFlowTableView();
    (m_viewMode->actions()[FlowTableViewType])->setChecked(true);
  } else {
    setupSummaryView();
    (m_viewMode->actions()[SummaryViewType])->setChecked(true);
  }
  m_viewMode->blockSignals(false);
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("currentView", m_view->id());
    settings.sync();
}

void MainWindow::setupView( StatusView *view, bool rememberJobs )
{
    if (m_view != view) {
        delete m_view;
        m_view = view;
    }

    m_configureViewAction->setEnabled( view->isConfigurable() );
    m_pauseViewAction->setEnabled( view->isPausable() );
    m_checkNodesAction->setEnabled( view->canCheckNodes() );
    m_monitor->setCurrentView( m_view, rememberJobs );

    setCentralWidget( m_view->widget() );
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

void MainWindow::setupPoolView()
{
//    setupView( new PoolView( m_hostInfoManager, this ), false );
}

void MainWindow::setupStarView()
{
    setupView( new StarView( m_hostInfoManager, this ), false );
}

void MainWindow::setupFlowTableView()
{
    setupView( new FlowTableView( m_hostInfoManager, this ), false );
}

void MainWindow::setupDetailedHostView()
{
    setupView( new DetailedHostView( m_hostInfoManager, this ), false );
}

void MainWindow::pauseView()
{
  m_view->togglePause();
}

void MainWindow::checkNodes()
{
  m_view->checkNodes();
}

void MainWindow::configureView()
{
    m_view->configureView();
}

void MainWindow::about()
{
    QString about;
    about = tr("%1 %2\n\n%3\n\nAuthor: %4\n\nBased on Icemon for KDE written by:\n\n%5\n%6\n%7\n\nLicensed under GPLv2.\n")
            .arg(Icemon::Version::appName)
            .arg(Icemon::Version::version)
            .arg(Icemon::Version::description)
            .arg("Daniel Molkentin <molkentin@kde.org>")
            .arg("Frerich Raabe <raabe@kde.org>")
            .arg("Stephan Kulow <coolo@kde.org>")
            .arg("Cornelius Schumacher <schumacher@kde.org>")
            ;

    QMessageBox::about(this, tr("About %1").arg(Icemon::Version::appShortName), about);
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}


void MainWindow::setSchedulerState(bool online)
{
    m_schedStatusWidget->setText(online ?
                                     tr("Scheduler is online.") :
                                     tr("Scheduler is offline.")
                );
}

void MainWindow::setCurrentNet( const QByteArray &netName )
{
  m_monitor->setCurrentNet( netName );
  m_currNetWidget->setText(tr("Current Network: %1").arg(QString::fromLatin1(netName)));
}

// It's nasty that we have to hard-code the implementations of Monitor
// But we can't just add a setMonitor() method because we require the host info manager
void MainWindow::setTestModeEnabled(bool testMode)
{
    if (testMode) {
        delete m_monitor;
        m_monitor = new FakeMonitor(m_hostInfoManager, this);
    } else {
        delete m_monitor;
        m_monitor = new IcecreamMonitor(m_hostInfoManager, this);
    }
    setupView(m_view, false);

    connect(m_monitor, SIGNAL( schedulerStateChanged( bool ) ), SLOT( setSchedulerState( bool ) ));
    setSchedulerState(m_monitor->schedulerState());
}

#include "mainwindow.moc"

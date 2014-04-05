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
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "mainwindow.h"

#include "views/starview.h"
#include "views/summaryview.h"
#include "views/detailedhostview.h"
#include "views/ganttstatusview.h"
#include "views/listview.h"
//#include "views/poolview.h"
#include "views/flowtableview.h"

#include "hostinfo.h"
#include "monitor.h"
#include "version.h"
#include "fakemonitor.h"
#include "icecreammonitor.h"

#include <QDebug>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QSettings>

#include <QMenu>

namespace {

class ViewFactory
{
public:
    StatusView* create(const QString &id, HostInfoManager* manager,
                       QWidget* parent = 0) const
    {
        if (id == "list") {
            return new ListStatusView(manager, parent);
        } else if (id == "gantt") {
            return new GanttStatusView(manager, parent);
        } else if (id == "star") {
            return new StarView(manager, parent);
        } else if (id == "pool") {
            //return new PoolView(manager, parent);
        } else if (id == "flow") {
            return new FlowTableView(manager, parent);
        } else if (id == "detailedhost") {
            return new DetailedHostView(manager, parent);
        }

        return new SummaryView(manager, parent);
    }
};

const ViewFactory s_viewFactory;

}

MainWindow::MainWindow( QWidget *parent )
    : QMainWindow(parent)
    , m_view(0)
{
    QIcon appIcon = QIcon();
    appIcon.addFile(":/images/hi128-app-icemon.png", QSize(128, 128));
    appIcon.addFile(":/images/hi48-app-icemon.png", QSize(48, 48));
    appIcon.addFile(":/images/hi32-app-icemon.png", QSize(32, 32));
    appIcon.addFile(":/images/hi22-app-icemon.png", QSize(22, 22));
    appIcon.addFile(":/images/hi16-app-icemon.png", QSize(16, 16));
    setWindowIcon(appIcon);
    setWindowTitle(QApplication::translate("appName", Icemon::Version::appName));

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
    action = m_viewMode->addAction(tr( "&List View" ));
    action->setCheckable(true);
    action->setData("list");
    action = m_viewMode->addAction(tr( "&Star View" ));
    action->setCheckable(true);
    action->setData("star");
#if 0 // Disabled for now, does not work
    action = m_viewMode->addAction(tr( "&Pool View" ));
    action->setCheckable(true);
    action->setData("pool");
#endif
    action = m_viewMode->addAction(tr( "&Gantt View" ));
    action->setCheckable(true);
    action->setData("gantt");
    action = m_viewMode->addAction(tr( "Summary &View" ));
    action->setCheckable(true);
    action->setData("summary");
    action = m_viewMode->addAction(tr( "&Flow View" ));
    action->setCheckable(true);
    action->setData("flow");
    action = m_viewMode->addAction(tr( "&Detailed Host View" ));
    action->setCheckable(true);
    action->setData("detailedhost");
    connect(m_viewMode, SIGNAL(triggered(QAction*)), this, SLOT(handleViewModeActionTriggered(QAction*)));

    QMenu* modeMenu = viewMenu->addMenu(tr("Mode"));
    modeMenu->addActions(m_viewMode->actions());

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

    m_hostInfoManager = new HostInfoManager;
    setMonitor(new IcecreamMonitor(m_hostInfoManager, this));

    resize(600, 400);
    readSettings();
}

MainWindow::~MainWindow()
{
    delete m_hostInfoManager;
}

void MainWindow::closeEvent( QCloseEvent *e )
{
    writeSettings();

    QMainWindow::closeEvent(e);
}

void MainWindow::readSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    QString viewId = settings.value("currentView").toString();

    StatusView* view = s_viewFactory.create(viewId, m_hostInfoManager, this);
    setView(view);
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("currentView", (m_view ? m_view->id() : QString()));
    settings.sync();
}

Monitor *MainWindow::monitor() const
{
    return m_monitor;
}

void MainWindow::setMonitor(Monitor* monitor)
{
    if (m_monitor == monitor)
        return;

    m_monitor = monitor;

    if (m_monitor) {
        m_monitor->setCurrentView(m_view);
        connect(m_monitor, SIGNAL( schedulerStateChanged( bool ) ), SLOT( setSchedulerState( bool ) ));
        setSchedulerState(m_monitor->schedulerState());
    }
}

StatusView* MainWindow::view() const
{
    return m_view;
}

void MainWindow::setView(StatusView *view)
{
    if (m_view == view)
        return;

    delete m_view;

    m_view = view;

    if (m_view) {
        m_configureViewAction->setEnabled(m_view->isConfigurable());
        m_pauseViewAction->setEnabled(m_view->isPausable());
        m_checkNodesAction->setEnabled(m_view->canCheckNodes());
        m_monitor->setCurrentView(m_view);

        setCentralWidget(m_view->widget());
    }

    // update action-group
    const QString viewId = (m_view ? m_view->id() : QString());
    foreach (QAction* action, m_viewMode->actions()) {
        if (action->data().toString() == viewId) {
            action->setChecked(true);
            break;
        }
    }
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
    QString about = tr("<strong>%1</strong><br/>"
        "Version: %2<br/><br/>"
        "<strong>%3</strong><br/><br/>"
        "Maintainers:<br/>"
        "Daniel Molkentin &lt;molkentin@kde.org&gt;<br/>"
        "Kevin Funk &lt;kfunk@kde.org&gt;<br/><br/>"
        "Based on Icemon for KDE written by:<br/>"
        "Frerich Raabe &lt;raabe@kde.org&gt;<br/>"
        "Stephan Kulow &lt;coolo@kde.org&gt;<br/>"
        "Cornelius Schumacher &lt;schumacher@kde.org&gt;<br/><br/>"
        "Homepage: <a href=\"%4\">%4</a><br/><br/>"
        "Licensed under the GPLv2.<br/>")
            .arg(Icemon::Version::appName)
            .arg(Icemon::Version::version)
            .arg(Icemon::Version::description)
            .arg(Icemon::Version::homePage);

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

void MainWindow::handleViewModeActionTriggered(QAction* action)
{
    const QString viewId = action->data().toString();
    Q_ASSERT(!viewId.isEmpty());
    setView(s_viewFactory.create(viewId, m_hostInfoManager, this));
}

// It's nasty that we have to hard-code the implementations of Monitor
// But we can't just add a setMonitor() method because we require the host info manager
void MainWindow::setTestModeEnabled(bool testMode)
{
    if (testMode) {
        setMonitor(new FakeMonitor(m_hostInfoManager, this));
    } else {
        setMonitor(new IcecreamMonitor(m_hostInfoManager, this));
    }
}

#include "mainwindow.moc"

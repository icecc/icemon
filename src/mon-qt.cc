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

#include "mon-qt.h"

#include <stdio.h>

#include "starview.h"
#include "summaryview.h"
//#include "detailedhostview.h"
//#include "ganttstatusview.h"
//#include "listview.h"
//#include "poolview.h"

#include "hostinfo.h"
#include "monitor.h"

#include <qdebug.h>

#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QSettings>

#include <QMenu>


const char * const appName = QT_TRANSLATE_NOOP("appName", "Icecream Monitor" );
const char * const appShortName = "icemon";
const char * const version = "2.0";
const char * const description = QT_TRANSLATE_NOOP( "description", "Icecream monitor for Qt" );
const char * const copyright = QT_TRANSLATE_NOOP( "copyright", "(c) 2003,2004, 2011 The icecream developers" );


MainWindow::MainWindow( QWidget *parent )
  : QMainWindow( parent ), m_view( 0 )
{
    setWindowIcon(QIcon(":/images/hi128-app-icemon.png"));
    setWindowTitle(QApplication::translate("appName", appName));
    m_hostInfoManager = new HostInfoManager;

    m_monitor = new Monitor( m_hostInfoManager, this );

    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction* action = fileMenu->addAction(tr("&Quit"), this, SLOT(close()), tr("Ctrl+Q"));
    action->setMenuRole(QAction::QuitRole);

    m_viewMode = new QActionGroup(this);

    QMenu* modeMenu = viewMenu->addMenu(tr("Mode"));

    action = m_viewMode->addAction(tr( "&List View" ));
    action->setDisabled(true); // FIXME
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
    action->setDisabled(true); // FIXME
    action->setCheckable(true);
    modeMenu->addAction(action);
    connect( action, SIGNAL( triggered() ), this, SLOT( setupGanttView() ) );

    action = m_viewMode->addAction(tr( "Summary &View" ));
    action->setCheckable(true);
    modeMenu->addAction(action);
    connect( action, SIGNAL( triggered() ), this, SLOT( setupSummaryView() ) );

    action = m_viewMode->addAction(tr( "&Detailed Host View" ));
    action->setDisabled(true); // FIXME
    action->setCheckable(true);
    modeMenu->addAction(action);
    connect( action, SIGNAL( triggered() ), this, SLOT( setupDetailedHostView() ) );

    viewMenu->addSeparator();

    action = viewMenu->addAction(tr("Stop"));
    connect( action, SIGNAL( triggered() ), this, SLOT( stopView() ) );

    action = viewMenu->addAction(tr("Start"));
    connect( action, SIGNAL( triggered() ), this, SLOT( startView() ) );

    viewMenu->addSeparator();

    action =  viewMenu->addAction(tr("Check Nodes"));
    connect( action, SIGNAL( triggered() ), this, SLOT( checkNodes() ) );
    viewMenu->addAction(action);

    viewMenu->addSeparator();

    action = viewMenu->addAction(tr("Configure View..."));
    connect( action, SIGNAL( triggered() ), this, SLOT( configureView() ) );

    action = helpMenu->addAction(tr("About Qt..."));
    connect(action, SIGNAL(triggered()), this, SLOT(aboutQt()));
    action->setMenuRole(QAction::AboutQtRole);
    action = helpMenu->addAction(tr("About..."));
    connect(action, SIGNAL(triggered()), this, SLOT(about()));
    action->setMenuRole(QAction::AboutRole);


    setupStarView(); // TODO: Hack!

    readSettings();
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
  delete m_view;
  m_view = view;
  m_monitor->setCurrentView( m_view, rememberJobs );
  setCentralWidget( m_view->widget() );
}

void MainWindow::setupListView()
{
//    setupView( new ListStatusView( m_hostInfoManager, this ), true );
}

void MainWindow::setupSummaryView()
{
    setupView( new SummaryView( m_hostInfoManager, this ), false );
}

void MainWindow::setupGanttView()
{
//    setupView( new GanttStatusView( m_hostInfoManager, this ), false );
}

void MainWindow::setupPoolView()
{
//    setupView( new PoolView( m_hostInfoManager, this ), false );
}

void MainWindow::setupStarView()
{
    setupView( new StarView( m_hostInfoManager, this ), false );
}

void MainWindow::setupDetailedHostView()
{
//    setupView( new DetailedHostView( m_hostInfoManager, this ), false );
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

void MainWindow::about()
{
    QString about;
    about = tr("%1\n\n%2\n\nAuthor: %3\n\nBased on Icemon for KDE written by:\n\n%4\n%5\n%6\n\nLicensed under GPLv2.\n")
            .arg(description)
            .arg(copyright)
            .arg("Daniel Molkentin <molkentin@kde.org>")
            .arg("Frerich Raabe <raabe@kde.org>")
            .arg("Stephan Kulow <coolo@kde.org>")
            .arg("Cornelius Schumacher <schumacher@kde.org>")
            ;

    QMessageBox::about(this, tr("About %1").arg(appShortName), about);
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}


void MainWindow::setCurrentNet( const QByteArray &netName )
{
  m_monitor->setCurrentNet( netName );
}

void printHelp() {
    QString help;
    help += QApplication::translate("Usage", "Usage: %1 [options]").arg(appShortName);
    help += '\n';
    help += '\n';
    help += QApplication::translate("Description", description);
    help += '\n';
    help += '\n';
    help += QApplication::translate("Options", "Options:");
    help += '\n';
    help += QApplication::translate("Netname Options",  "\t-n, netname <name>\tIcecream network name");
    help += '\n';
    help += '\n';
    fputs(qPrintable(help), stdout);
}

int main( int argc, char **argv )
{
    QByteArray netName;

    for (int i = 1; i < argc; ++i ) {
        if (qstrcmp(argv[i], "-help") == 0 || qstrcmp(argv[i], "-h")  == 0) {
            printHelp();
            return 0;
        }
        if (qstrcmp(argv[i], "-netname") == 0 || qstrcmp(argv[i], "-n")  == 0) {
            if (i+1 < argc)
                netName = argv[++i];
        }
    }


  QApplication app(argc, argv);
  QApplication::setOrganizationDomain("kde.org");
  QApplication::setApplicationName(appShortName);
  QApplication::setApplicationVersion(version);

  MainWindow *mainWidget = new MainWindow;

  if ( !netName.isEmpty() ) {
    mainWidget->setCurrentNet( netName );
  }

  mainWidget->show();

  return app.exec();
}

#include "mon-qt.moc"

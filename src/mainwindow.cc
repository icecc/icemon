/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2014 Kevin Funk <kfunk@kde.org>

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

#include "hostinfo.h"
#include "version.h"
#include "fakemonitor.h"
#include "icecreammonitor.h"
#include "statusview.h"
#include "statusviewfactory.h"

#include "utils.h"

#include <QCloseEvent>
#include <QDebug>
#include <QLabel>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QSettings>
#include <QMenu>
#include <QActionGroup>

#include <algorithm>

namespace {

struct PlatformStat
{
    PlatformStat()
        : jobs(0)
        , maxJobs(0) {}

    unsigned int jobs;
    unsigned int maxJobs;
};

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_view(nullptr)
    , m_systemTrayIcon(nullptr)
{
    QIcon appIcon = QIcon();
    appIcon.addFile(QStringLiteral(":/images/128-apps-icemon.png"), QSize(128, 128));
    appIcon.addFile(QStringLiteral(":/images/48-apps-icemon.png"), QSize(48, 48));
    appIcon.addFile(QStringLiteral(":/images/32-apps-icemon.png"), QSize(32, 32));
    appIcon.addFile(QStringLiteral(":/images/22-apps-icemon.png"), QSize(22, 22));
    appIcon.addFile(QStringLiteral(":/images/16-apps-icemon.png"), QSize(16, 16));
    setWindowIcon(appIcon);
    setWindowTitle(QApplication::translate("appName", Icemon::Version::appName));

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    m_schedStatusWidget = new QLabel;
    statusBar()->addPermanentWidget(m_schedStatusWidget);

    m_jobStatsWidget = new QLabel;
    m_jobStatsWidget->setVisible(false);
    statusBar()->addPermanentWidget(m_jobStatsWidget);

    QAction *action = nullptr;

    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        action = fileMenu->addAction(tr("Show in System Tray"));
        action->setIcon(QIcon::fromTheme(QStringLiteral("systemtray")));
        action->setCheckable(true);
        connect(action, &QAction::triggered, this, &MainWindow::updateSystemTrayVisible);
        m_showInSystemTrayAction = action;

        QMenu *systrayMenu = new QMenu(this);
        QAction *quitAction = systrayMenu->addAction(tr("&Quit"), this, SLOT(quit()), tr("Ctrl+Q"));
        quitAction->setIcon(QIcon::fromTheme(QStringLiteral("application-exit")));
        quitAction->setMenuRole(QAction::QuitRole);

        m_systemTrayIcon = new QSystemTrayIcon(this);
        m_systemTrayIcon->setIcon(appIcon);
        m_systemTrayIcon->setToolTip(windowTitle());
        m_systemTrayIcon->setContextMenu(systrayMenu);
        connect(m_systemTrayIcon, &QSystemTrayIcon::activated, this, &MainWindow::systemTrayIconActivated);

        fileMenu->addSeparator();
    }

    action = fileMenu->addAction(tr("&Quit"), this, &QWidget::close, tr("Ctrl+Q"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("application-exit")));
    action->setMenuRole(QAction::QuitRole);

    m_viewMode = new QActionGroup(this);
    action = m_viewMode->addAction(tr("&List View"));
    action->setCheckable(true);
    action->setData(QStringLiteral("list"));
    action = m_viewMode->addAction(tr("&Star View"));
    action->setCheckable(true);
    action->setData(QStringLiteral("star"));
    action = m_viewMode->addAction(tr("&Gantt View"));
    action->setCheckable(true);
    action->setData(QStringLiteral("gantt"));
    action = m_viewMode->addAction(tr("Summary &View"));
    action->setCheckable(true);
    action->setData(QStringLiteral("summary"));
    action = m_viewMode->addAction(tr("&Flow View"));
    action->setCheckable(true);
    action->setData(QStringLiteral("flow"));
    action = m_viewMode->addAction(tr("&Detailed Host View"));
    action->setCheckable(true);
    action->setData(QStringLiteral("detailedhost"));
    connect(m_viewMode, &QActionGroup::triggered, this, &MainWindow::handleViewModeActionTriggered);
    viewMenu->addActions(m_viewMode->actions());

    viewMenu->addSeparator();

    action = viewMenu->addAction(tr("Pause"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause")));
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &MainWindow::pauseView);
    m_pauseViewAction = action;

    viewMenu->addSeparator();

    action = viewMenu->addAction(tr("Configure View..."));
    action->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    connect(action, &QAction::triggered, this, &MainWindow::configureView);
    m_configureViewAction = action;

    action = helpMenu->addAction(tr("About Qt..."));
    connect(action, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    action->setMenuRole(QAction::AboutQtRole);

    action = helpMenu->addAction(tr("About..."));
    action->setIcon(appIcon);
    connect(action, &QAction::triggered, this, &MainWindow::about);
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

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (m_systemTrayIcon && m_systemTrayIcon->isVisible())
    {
        QSettings settings;
        const bool shownBefore = settings.value(QStringLiteral("programWillKeepRunningWarningShown")).toBool();
        if (!shownBefore) {
            QMessageBox::information(this, tr("Systray"),
                                    tr("The program will keep running in the "
                                        "system tray. To terminate the program, "
                                        "choose <b>Quit</b> in the context menu "
                                        "of the system tray entry."));
            settings.setValue(QStringLiteral("programWillKeepRunningWarningShown"), true);
            settings.sync();
        }

        hide();
        e->ignore();
        return;
    }

    writeSettings();

    QMainWindow::closeEvent(e);
}

void MainWindow::readSettings()
{
    QSettings settings;
    restoreGeometry(settings.value(QStringLiteral("geometry")).toByteArray());
    restoreState(settings.value(QStringLiteral("windowState")).toByteArray());
    bool showSystemTray = settings.value(QStringLiteral("showSystemTray")).toBool();
    QString viewId = settings.value(QStringLiteral("currentView")).toString();

    auto view = StatusViewFactory::create(viewId, this);
    setView(view);

    if (m_systemTrayIcon)
    {
        m_showInSystemTrayAction->setChecked(showSystemTray);
        updateSystemTrayVisible();
    }
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue(QStringLiteral("geometry"), saveGeometry());
    settings.setValue(QStringLiteral("windowState"), saveState());
    if (m_systemTrayIcon)
    {
        settings.setValue(QStringLiteral("showSystemTray"), m_systemTrayIcon->isVisible());
    }
    settings.setValue(QStringLiteral("currentView"), (m_view ? m_view->id() : QString()));
    settings.sync();
}

Monitor *MainWindow::monitor() const
{
    return m_monitor;
}

void MainWindow::setMonitor(Monitor *monitor)
{
    if (m_monitor == monitor) {
        return;
    }

    if (m_monitor) {
        disconnect(m_monitor.data(), &Monitor::schedulerStateChanged,
                   this, &MainWindow::updateSchedulerState);
        disconnect(m_monitor.data(), &Monitor::jobUpdated, this, &MainWindow::updateJob);
        disconnect(m_monitor->hostInfoManager(), &HostInfoManager::hostMapChanged, this, &MainWindow::updateJobStats);
    }

    m_monitor = monitor;

    if (m_monitor) {
        connect(m_monitor.data(), &Monitor::schedulerStateChanged,
                this, &MainWindow::updateSchedulerState);
        connect(m_monitor.data(), &Monitor::jobUpdated, this, &MainWindow::updateJob);
        connect(m_monitor->hostInfoManager(), &HostInfoManager::hostMapChanged, this, &MainWindow::updateJobStats);
    }

    if (m_view) {
        m_view->setMonitor(m_monitor);
    }
    updateSchedulerState(m_monitor ? m_monitor->schedulerState() : Monitor::Offline);
}

StatusView *MainWindow::view() const
{
    return m_view;
}

void MainWindow::setView(StatusView *view)
{
    if (m_view == view) {
        return;
    }

    delete m_view;

    m_view = view;

    if (m_view) {
        m_configureViewAction->setEnabled(m_view->isConfigurable());
        m_pauseViewAction->setEnabled(m_view->isPausable());
        m_view->setMonitor(m_monitor);

        setCentralWidget(m_view->widget());
    }

    // update action-group
    const QString viewId = (m_view ? m_view->id() : QString());
    foreach(QAction * action, m_viewMode->actions()) {
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

void MainWindow::configureView()
{
    m_view->configureView();
}

void MainWindow::updateSystemTrayVisible()
{
    if (!m_systemTrayIcon)
    {
        return;
    }
    if (m_showInSystemTrayAction->isChecked()) {
        m_systemTrayIcon->show();
    } else {
        m_systemTrayIcon->hide();
    }
}

void MainWindow::quit()
{
    writeSettings();
    qApp->quit();
}

void MainWindow::systemTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        if (isVisible()) {
            if (isMinimized()) {
                showMaximized();
            } else {
                hide();
            }
        } else {
            show();
        }
        break;

    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::MiddleClick:
    case QSystemTrayIcon::Unknown:
    default:
        ;
    }
}

void MainWindow::about()
{
    QString about = tr("<p><strong>%1</strong><br/>"
                       "Version: %2</p>"
                       "<p><strong>%3</strong></p>"
                       "<p>Maintainer:<br/>"
                       "Kevin Funk &lt;kfunk@kde.org&gt;</p>"
                       "<p>Contributors:<br/>"
                       "Daniel Molkentin &lt;molkentin@kde.org&gt;"
                       "<p>Based on Icemon for KDE written by:<br/>"
                       "Frerich Raabe &lt;raabe@kde.org&gt;<br/>"
                       "Stephan Kulow &lt;coolo@kde.org&gt;<br/>"
                       "Cornelius Schumacher &lt;schumacher@kde.org&gt;</p>"
                       "Homepage: <a href=\"%4\">%4</a><br/><br/>"
                       "Licensed under the GPLv2.<br/>").arg(
                            QLatin1String(Icemon::Version::appName),
                            QLatin1String(Icemon::Version::version),
                            QLatin1String(Icemon::Version::description),
                            QLatin1String(Icemon::Version::homePage));

    QMessageBox::about(this, tr("About %1")
        .arg(QLatin1String(Icemon::Version::appShortName)), about);
}

void MainWindow::updateSchedulerState(Monitor::SchedulerState state)
{
    if (state == Monitor::Online) {
        QString statusText = m_hostInfoManager->schedulerName();

        if (!m_hostInfoManager->networkName().isEmpty()) {
            statusText.append(QStringLiteral(" @ ")).append(m_hostInfoManager->networkName());
        }

        m_schedStatusWidget->setText(statusText.isEmpty() ? tr("Scheduler is online.") : statusText);
    } else
    {
        m_schedStatusWidget->setText(tr("Scheduler is offline."));
    }

    m_activeJobs.clear();
    updateJobStats();
}

void MainWindow::updateJob(const Job &job)
{
    if (job.isActive()) {
        m_activeJobs[job.id] = job;
        updateJobStats();
    } else if (job.isDone()) {
        m_activeJobs.remove(job.id);
        updateJobStats();
    }
}

void MainWindow::updateJobStats()
{
    if (!m_monitor->schedulerState()) {
        m_jobStatsWidget->clear();
        m_jobStatsWidget->setVisible(false);
        if (m_systemTrayIcon)
        {
            m_systemTrayIcon->setToolTip(tr("Scheduler is offline."));
        }
        return;
    }

    QMap<QString, PlatformStat> perPlatformStats;
    const HostInfoManager::HostMap hostMap = m_monitor->hostInfoManager()->hostMap();
    for (auto i = hostMap.begin(); i != hostMap.end(); ++i) {
        if (!i.value()->isOffline() && !i.value()->noRemote()) {
            perPlatformStats[i.value()->platform()].maxJobs += i.value()->maxJobs();
        }
    }
    for (JobList::const_iterator i = m_activeJobs.constBegin(); i != m_activeJobs.constEnd(); ++i) {
        const HostInfo *server = hostMap[i.value().server != 0 ? i.value().server : i.value().client];
        if (server && !server->isOffline() && !server->noRemote()) {
            ++perPlatformStats[server->platform()].jobs;
        }
    }

    // Turn into something we can sort differently
    QVector<QPair<QString, PlatformStat>> statistics;
    for (auto it = perPlatformStats.constBegin(); it != perPlatformStats.constEnd(); ++it) {
        statistics << qMakePair(it.key(), it.value());
    }

    // Sort, move the platform with the highest max jobs count to the front
    std::sort(statistics.begin(), statistics.end(), [](const QPair<QString, PlatformStat>& a,
                                                       const QPair<QString, PlatformStat>& b) {
        return a.second.maxJobs > b.second.maxJobs;
    });

    // Compose the text
    QString text;
    QString textNoHTML;
    foreach (auto pair, statistics) {
        const QString& platform = pair.first;
        const PlatformStat& stat = pair.second;
        if (stat.maxJobs == 0) {
            continue;
        }

        if (!text.isEmpty()) {
            text.append(QStringLiteral(" - "));
            textNoHTML.append(QStringLiteral(" - "));
        }

        text.append(QStringLiteral("<strong>%2/%3</strong> on %1").arg(platform).arg(stat.jobs).arg(stat.maxJobs));
        textNoHTML.append(QStringLiteral("%2/%3 on %1").arg(platform).arg(stat.jobs).arg(stat.maxJobs));
    }

    m_jobStatsWidget->setText(tr("| Active jobs: %1").arg(text));
    m_jobStatsWidget->setVisible(true);
    if (m_systemTrayIcon)
    {
        m_systemTrayIcon->setToolTip(tr("Active jobs: %1").arg(textNoHTML));
    }
}

void MainWindow::setCurrentNet(const QByteArray &netname)
{
    m_monitor->setCurrentNetname(netname);
}

void MainWindow::setCurrentSched(const QByteArray &schedname)
{
    m_monitor->setCurrentSchedname(schedname);
}

void MainWindow::setCurrentPort(uint schedport)
{
    m_monitor->setCurrentSchedport(schedport);
}

void MainWindow::handleViewModeActionTriggered(QAction *action)
{
    const QString viewId = action->data().toString();
    Q_ASSERT(!viewId.isEmpty());
    setView(StatusViewFactory::create(viewId, this));
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

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

#ifndef ICEMON_MAINWINDOW_H
#define ICEMON_MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include <QSystemTrayIcon>

#include "monitor.h"
#include "job.h"

class HostInfoManager;
class StatusView;

class QActionGroup;
class QLabel;

class MainWindow
    : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void setCurrentNet(const QByteArray &netname);
    void setCurrentSched(const QByteArray &schedname);
    void setCurrentPort(uint schedport);

    Monitor *monitor() const;
    StatusView *view() const;

    void setTestModeEnabled(bool testMode);

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void pauseView();
    void configureView();
    void updateSystemTrayVisible();
    void systemTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void quit();

    void about();

    void updateSchedulerState(Monitor::SchedulerState state);
    void updateJob(const Job &);
    void updateJobStats();

    void handleViewModeActionTriggered(QAction *action);

private:
    void readSettings();
    void writeSettings();

    /// Does *not* take ownership over @p monitor
    void setMonitor(Monitor *monitor);
    /// Takes ownership over @p view
    void setView(StatusView *view);

    HostInfoManager *m_hostInfoManager;
    QPointer<Monitor> m_monitor;
    StatusView *m_view;
    QSystemTrayIcon* m_systemTrayIcon;

    QLabel *m_schedStatusWidget;
    QLabel *m_jobStatsWidget;

    QActionGroup *m_viewMode;
    QAction *m_configureViewAction;
    QAction *m_pauseViewAction;
    QAction *m_showInSystemTrayAction;

    JobList m_activeJobs;
};

#endif // ICEMON_MAINWINDOW_H
// vim:ts=4:sw=4:et

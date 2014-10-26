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

#include "job.h"

class HostInfoManager;
class Monitor;
class StatusView;

class QActionGroup;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( QWidget *parent = nullptr );
    virtual ~MainWindow();

    void setCurrentNet(const QByteArray &netname);

    Monitor *monitor() const;
    StatusView *view() const;

    void setTestModeEnabled(bool testMode);

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void pauseView();
    void configureView();

    void about();
    void aboutQt();

    void setSchedulerState(bool online);
    void updateJob( const Job& );
    void updateJobStats();

    void handleViewModeActionTriggered(QAction* action);

private:
    void readSettings();
    void writeSettings();

    /// Does *not* take ownership over @p monitor
    void setMonitor(Monitor *monitor);
    /// Takes ownership over @p view
    void setView(StatusView *view);

    HostInfoManager *m_hostInfoManager;
    QPointer<Monitor> m_monitor;
    StatusView* m_view;

    QLabel *m_schedStatusWidget;
    QLabel *m_jobStatsWidget;

    QActionGroup* m_viewMode;
    QAction *m_configureViewAction;
    QAction *m_pauseViewAction;

    JobList m_activeJobs;
};

#endif // ICEMON_MAINWINDOW_H
// vim:ts=4:sw=4:et

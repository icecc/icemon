/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004,2006-2007 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004,2011 Daniel Molkentin <molkentin@kde.org> <daniel.molkentin@nokia.com> <daniel@molkentin.de>
    Copyright (c) 2004 Luboš Luňák <l.lunak@kde.org>
    Copyright (c) 2004 Scott Wheeler <wheeler@kde.org>
    Copyright (c) 2004 Andre Wöbbeking <woebbeking@kde.org>
    Copyright (c) 2006 Hamish Rodda <hamish@kde.org>
    Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
    Copyright (c) 2007 Bruno Virlet <bruno.virlet@gmail.com>
    Copyright (c) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (c) 2008 Urs Wolfer <uwolfer@kde.org>
    Copyright (c) 2012 Kevin Funk <kevin@kfunk.org>

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

#include <QtGui/QMainWindow>

class HostInfoManager;
class Monitor;
class StatusView;

class QActionGroup;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
  public:
    MainWindow( QWidget *parent = 0 );

    void setCurrentNet(const QByteArray &netname);

    void setTestModeEnabled(bool testMode);

protected:
    void closeEvent(QCloseEvent *e);

  private slots:
    void setupListView();
    void setupStarView();
    void setupPoolView();
    void setupSummaryView();
    void setupGanttView();
    void setupDetailedHostView();
    void setupFlowTableView();

    void pauseView();
    void checkNodes();
    void configureView();

    void about();
    void aboutQt();

    void setSchedulerState(bool online);

  private:
    void readSettings();
    void writeSettings();

    void setupView( StatusView *view, bool rememberJobs );

    HostInfoManager *m_hostInfoManager;
    QLabel *m_schedStatusWidget;
    QLabel *m_currNetWidget;
    Monitor *m_monitor;
    StatusView *m_view;

    enum views {
        ListViewType,
        StarViewType,
        PoolViewType,
        GanttViewType,
        SummaryViewType,
        FlowTableViewType,
        DetailedHostViewType
    };

    QActionGroup* m_viewMode;
    QAction *m_configureViewAction;
    QAction *m_pauseViewAction;
    QAction *m_checkNodesAction;
};

#endif // ICEMON_MAINWINDOW_H
// vim:ts=4:sw=4:et

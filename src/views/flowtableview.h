/*
    This file is part of Icecream.

    Copyright (c) 2011 Daniel Molkentin <molkentin@kde.org>

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

#ifndef FLOWTABLEVIEW_H
#define FLOWTABLEVIEW_H

#include "job.h"
#include "hostinfo.h"
#include "statusview.h"

#include <QScopedPointer>
#include <QTableWidget>
#include <QTableWidgetItem>

class Job;

typedef QHash<int, int> HostIdRowMap;

class ProgressWidget : public QWidget
{
    Q_OBJECT
public:
    ProgressWidget(HostInfo *info, StatusView *statusView, QWidget *parent = 0);

    void setCurrentJob(const Job &job) { m_currentJob = job; }
    Job currentJob() const { return m_currentJob; }

    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
private:
    HostInfo *m_hostInfo;
    StatusView *m_statusView;
    Job m_currentJob;
    QImage m_backingStore;
    bool m_isVirgin;
};

class FlowTableView : public StatusView
{
    Q_OBJECT
public:
    explicit FlowTableView(QObject* parent);
    
    virtual QWidget* widget() const;

    void update( const Job &job);
    void checkNode( unsigned int hostid );
    void removeNode( unsigned int hostid );

    void checkNodes() {}

    QString id() const { return "flow"; }

    void stop() {}
    void start() {}

    bool canCheckNodes() { return false; }
    bool isPausable() { return false; }
    bool isConfigurable() { return false; }

private:
    QScopedPointer<QTableWidget> m_widget;
    QString hostInfoText(HostInfo *hostInfo, int runningProcesses = 0);
    HostIdRowMap m_idToRowMap;
    QTimer *m_updateTimer;
    
};

#endif // FLOWTABLEVIEW_H

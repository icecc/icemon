/*
    This file is part of Icecream.

    Copyright (c) 2004-2006 Andre Wöbbeking <Woebbeking@web.de>

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

#ifndef ICEMON_DETAILEDHOSTVIEW_H
#define ICEMON_DETAILEDHOSTVIEW_H

#include "statusview.h"

#include <QScopedPointer>
#include <QWidget>

class HostListModel;
class JobListView;
class JobListModel;
class HostListView;
class QSortFilterProxyModel;

class DetailedHostView : public StatusView
{
    Q_OBJECT

public:
    DetailedHostView(QObject* parent);

    virtual void setMonitor(Monitor* monitor);

    QWidget* widget() const;

    QString id() const { return "detailedhost"; }

    void checkNode( unsigned int hostid );

private:
    void createKnownHosts();

    QScopedPointer<QWidget> m_widget;

    HostListModel* mHostListModel;
    HostListView* mHostListView;
    QSortFilterProxyModel* mSortedHostListModel;

    JobListModel* mLocalJobsModel;
    JobListView* mLocalJobsView;
    QSortFilterProxyModel* mSortedLocalJobsModel;

    JobListModel* mRemoteJobsModel;
    JobListView* mRemoteJobsView;
    QSortFilterProxyModel* mSortedRemoteJobsModel;
};


#endif

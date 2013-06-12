/*
    This file is part of Icecream.

    Copyright (c) 2004-2006 Andre Wöbbeking <Woebbeking@web.de>
    Copyright (c) 2007 Stephan Kulow <coolo@kde.org>
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

#ifndef ICEMON_DETAILEDHOSTVIEW_H
#define ICEMON_DETAILEDHOSTVIEW_H

#include "statusview.h"

#include <QWidget>

class HostListModel;
class JobListView;
class JobListModel;
class HostListView;

class DetailedHostView : public QWidget, public StatusView
{
    Q_OBJECT

public:

    DetailedHostView( HostInfoManager* manager, QWidget *parent );

    void update( const Job &job );

    QWidget* widget();

    QString id() const { return "detailedhost"; }

    void checkNode( unsigned int hostid );

    void removeNode( unsigned int hostid );

    void updateSchedulerState( bool online );

private slots:
    void slotNodeActivated();

private:

    void createKnownHosts();

    HostListModel* mHostListModel;
    HostListView* mHostListView;

    JobListModel* mLocalJobsModel;
    JobListView* mLocalJobsView;

    JobListModel* mRemoteJobsModel;
    JobListView* mRemoteJobsView;
};


#endif

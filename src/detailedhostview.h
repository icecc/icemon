/*
    This file is part of Icecream.

    Copyright (c) 2004-2006 Andre Wöbbeking <Woebbeking@web.de>

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
#ifndef ICEMON_DETAILEDHOSTVIEW_H
#define ICEMON_DETAILEDHOSTVIEW_H


#include "statusview.h"

#include <QWidget>

class JobTreeWidget;
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

    HostListView* mHostListView;
    JobTreeWidget* mLocalJobsView;
    JobTreeWidget* mRemoteJobsView;
};


#endif

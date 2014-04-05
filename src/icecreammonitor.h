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

#ifndef ICEMON_ICECREAMMONITOR_H
#define ICEMON_ICECREAMMONITOR_H

#include "monitor.h"
#include "job.h"

#include <QtCore/QSocketNotifier>

class HostInfoManager;
class Msg;
class MsgChannel;
class StatusView;
class DiscoverSched;
class QSocketNotifier;

class IcecreamMonitor : public Monitor
{
    Q_OBJECT

  public:
    IcecreamMonitor(HostInfoManager *, QObject *parent);
    ~IcecreamMonitor();

    void setCurrentView( StatusView* view );

    void setSchedulerState( bool );
    void setCurrentNet( const QByteArray & );

    bool schedulerState() const { return m_schedulerState; }

  private slots:
    void slotCheckScheduler();
    void msgReceived();

  private:
    void checkScheduler(bool deleteit = false);
    void registerNotify(int fd, QSocketNotifier::Type type, const char* slot);
    void setupDebug();

    bool handle_activity();
    void handle_getcs( Msg *m );
    void handle_job_begin( Msg *m );
    void handle_job_done( Msg *m );
    void handle_stats( Msg *m );
    void handle_local_begin( Msg *m );
    void handle_local_done( Msg *m );

    StatusView *m_view;
    JobList m_rememberedJobs;
    MsgChannel *m_scheduler;
    QByteArray m_current_netname;
    bool m_schedulerState;

    DiscoverSched *m_discover;
    QSocketNotifier *m_fd_notify;
    QSocketNotifier::Type m_fd_type;
};

#endif // ICEMON_ICECREAMMONITOR_H

// vim:ts=4:sw=4:noet

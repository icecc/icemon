/*
    This file is part of Icecream.

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

#include "monitor.h"

#include "statusview.h"

Monitor::Monitor(HostInfoManager *manager, QObject *parent)
    : QObject(parent)
    , m_hostInfoManager(manager)
    , m_currentSchedport(0)
    , m_schedulerState(Offline)
{
}

QByteArray Monitor::currentNetname() const
{
    return m_currentNetname;
}

void Monitor::setCurrentNetname(const QByteArray &netname)
{
    m_currentNetname = netname;
}

QByteArray Monitor::currentSchedname() const
{
    return m_currentSchedname;
}

void Monitor::setCurrentSchedname(const QByteArray &schedname)
{
    m_currentSchedname = schedname;
}

uint Monitor::currentSchedport() const
{
    return m_currentSchedport;
}
void Monitor::setCurrentSchedport(uint port)
{
    m_currentSchedport = port;
}

Monitor::SchedulerState Monitor::schedulerState() const
{
    return m_schedulerState;
}

void Monitor::setSchedulerState(SchedulerState state)
{
    if (m_schedulerState == state) {
        return;
    }

    m_schedulerState = state;
    emit schedulerStateChanged(state);
}

QList<Job> Monitor::jobHistory() const
{
    return QList<Job>();
}

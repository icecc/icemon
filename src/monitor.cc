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
    , m_schedulerState(false)
{
}

QByteArray Monitor::currentNetname() const
{
    return m_currentNetname;
}

void Monitor::setCurrentNetname(const QByteArray& netname)
{
    m_currentNetname = netname;
}

bool Monitor::schedulerState() const
{
    return m_schedulerState;
}

void Monitor::setSchedulerState( bool online )
{
    if (m_schedulerState == online)
        return;

    m_schedulerState = online;
    emit schedulerStateChanged( online );
}

QList<Job> Monitor::jobHistory() const
{
    return QList<Job>();
}

#include "monitor.moc"

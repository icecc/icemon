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

#ifndef ICEMON_MONITOR_H
#define ICEMON_MONITOR_H

#include "job.h"
#include "types.h"

#include <QObject>

class StatusView;
class HostInfoManager;
class Job;

/**
 * Abstract base class for monitoring a icecream-like scheduler
 */
class Monitor
    : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SchedulerState schedulerState READ schedulerState WRITE setSchedulerState NOTIFY schedulerStateChanged)
    Q_ENUMS(SchedulerState)

public
        : enum SchedulerState {
        Offline,
        Online,
    };

    explicit Monitor(HostInfoManager *manager, QObject *parent = nullptr);

    QByteArray currentNetname() const;
    void setCurrentNetname(const QByteArray &);

    QByteArray currentSchedname() const;
    void setCurrentSchedname(const QByteArray &);

    SchedulerState schedulerState() const;

    virtual QList<Job> jobHistory() const;

    HostInfoManager *hostInfoManager() const { return m_hostInfoManager; }

protected:
    void setSchedulerState(SchedulerState online);

Q_SIGNALS:
    void schedulerStateChanged(Monitor::SchedulerState);

    void jobUpdated(const Job &job);
    void nodeRemoved(HostId id);
    void nodeUpdated(HostId id);

private:
    HostInfoManager *m_hostInfoManager;
    QByteArray m_currentNetname;
    QByteArray m_currentSchedname;
    SchedulerState m_schedulerState;
};

#endif // ICEMON_MONITOR_H

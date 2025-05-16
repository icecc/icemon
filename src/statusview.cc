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

#include "statusview.h"

#include "hostinfo.h"
#include "job.h"
#include "utils.h"

#include <QDebug>
#include <QTime>

StatusView::StatusView(QObject *parent)
    : QObject(parent)
    , m_paused(false)
{
}

StatusView::~StatusView()
{
}

StatusView::Options StatusView::options() const
{
    return NoOptions;
}

Monitor *StatusView::monitor() const
{
    return m_monitor;
}

void StatusView::setMonitor(Monitor *monitor)
{
    if (m_monitor == monitor) {
        return;
    }

    if (m_monitor) {
        disconnect(m_monitor.data(), &Monitor::jobUpdated, this, &StatusView::update);
        disconnect(m_monitor.data(), &Monitor::nodeRemoved, this, &StatusView::removeNode);
        disconnect(m_monitor.data(), &Monitor::nodeUpdated, this, &StatusView::checkNode);
        disconnect(m_monitor.data(), &Monitor::schedulerStateChanged,
                   this, &StatusView::updateSchedulerState);
    }

    m_monitor = monitor;

    if (m_monitor) {
        connect(m_monitor.data(), &Monitor::jobUpdated, this, &StatusView::update);
        connect(m_monitor.data(), &Monitor::nodeRemoved, this, &StatusView::removeNode);
        connect(m_monitor.data(), &Monitor::nodeUpdated, this, &StatusView::checkNode);
        connect(m_monitor.data(), &Monitor::schedulerStateChanged,
                this, &StatusView::updateSchedulerState);

        if (options().testFlag(RememberJobsOption)) {
            foreach(const Job &job, m_monitor->jobHistory()) {
                update(job);
            }
        }
    }
}

HostInfoManager *StatusView::hostInfoManager() const
{
    return (m_monitor ? m_monitor->hostInfoManager() : nullptr);
}

void StatusView::update(const Job &)
{
}

void StatusView::checkNode(HostId)
{
}

void StatusView::removeNode(HostId)
{
}

void StatusView::updateSchedulerState(Monitor::SchedulerState state)
{
    Q_UNUSED(state);
}

QString StatusView::nameForHost(HostId id)
{
    if (!m_monitor) {
        return QString();
    }

    return m_monitor->hostInfoManager()->nameForHost(id);
}

QColor StatusView::hostColor(HostId id)
{
    if (!m_monitor) {
        return QColor();
    }

    return m_monitor->hostInfoManager()->hostColor(id);
}

unsigned int StatusView::processor(const Job &job)
{
    unsigned int ret = 0;
    if (job.state == Job::LocalOnly || job.state == Job::WaitingForCS) {
        ret = job.client;
    } else {
        ret = job.server;
        if (!ret) {
            //            Q_ASSERT( job.m_state == Job::Finished );
            ret = job.client;
        }
    }
    Q_ASSERT(ret);
    return ret;
}

void StatusView::togglePause()
{
    if (m_paused) {
        start();
    } else {
        stop();
    }

    m_paused = !m_paused;
}

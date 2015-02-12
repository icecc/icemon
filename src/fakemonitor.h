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

#ifndef ICEMON_FAKEMONITOR_H
#define ICEMON_FAKEMONITOR_H

#include "monitor.h"

#include "job.h"

class HostInfoManager;
class StatusView;

class QTimer;

class FakeMonitor
    : public Monitor
{
    Q_OBJECT

public:
    explicit FakeMonitor(HostInfoManager *manager, QObject *parent = nullptr);

private Q_SLOTS:
    void update();

private:
    void createHostInfo(HostId id);

    QList<Job> m_activeJobs;

    QTimer *m_updateTimer;
};

#endif // ICEMON_FAKEMONITOR_H

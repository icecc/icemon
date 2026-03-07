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

#include "fakemonitor.h"

#include "statusview.h"
#include "job.h"
#include "hostinfo.h"

#include <QDebug>
#include <QStringList>
#include <QTime>
#include <QTimer>
#include <QRandomGenerator>

namespace {
// counter variable
int JOB_ID = 0;

const int MAX_HOST_COUNT = 20;
const int MAX_JOB_COUNT = 32;
const int MAX_TOTAL_JOB_COUNT = MAX_HOST_COUNT * MAX_JOB_COUNT / 2;
const int MAX_JOB_SIZE = 1024 * 1024 * 24;

const QStringList JOB_FILENAMES(QStringList()
    << QStringLiteral("/tmp/filename.cc")
    << QStringLiteral("/some/very/long/path/containing/filename.cc")
    << QStringLiteral("/some/very/long/path/containing/averyverylongfilename.cc")
    );

const QStringList HOST_NAMES(QStringList()
    << QStringLiteral("Hostname")
    << QStringLiteral("VeryLongHostname")
    << QStringLiteral("VeryLongHostname.localdomain")
    );

QString randomPlatform()
{
    static const QStringList hostNames = {QStringLiteral("x86_64"), QStringLiteral("arm64")};
    return hostNames[QRandomGenerator::global()->generate() % hostNames.size()];
}
}

FakeMonitor::FakeMonitor(HostInfoManager *manager, QObject *parent)
    : Monitor(manager, parent)
    , m_updateTimer(new QTimer(this))
{
    m_updateTimer->setInterval(500);
    m_updateTimer->start();
    connect(m_updateTimer, &QTimer::timeout, this, &FakeMonitor::update);

    setSchedulerState(SchedulerState::Online);

    for (HostId i = 0; i < MAX_HOST_COUNT; ++i) {
        createHostInfo(i + 1);
    }
}

void FakeMonitor::createHostInfo(HostId id)
{
    // Set some servers as not accepting remote jobs
    const float speed = (QRandomGenerator::global()->generate() % 5) * 100.0;

    HostInfo info(id);
    info.setIp(QStringLiteral("1.0.0.%1").arg(id));
    info.setMaxJobs(MAX_JOB_COUNT);
    info.setName(HOST_NAMES[id % HOST_NAMES.length()] + QString::number(id));
    info.setColor(info.createColor(info.name()));
    info.setOffline(false);
    info.setNoRemote(qFuzzyIsNull(speed));
    info.setPlatform(randomPlatform());
    info.setProtocol(0);
    info.setFeatures(id % 2 == 0 ? QStringLiteral("env_xz") : QStringLiteral("env_zstd"));
    info.setServerLoad(1.0);
    info.setServerSpeed(speed);
    hostInfoManager()->checkNode(info);
}

void FakeMonitor::update()
{
    // create jobs
    for (int i = 0; i < 10; ++i) {
        const int jobId = JOB_ID++;

        const int clientId = (jobId % MAX_HOST_COUNT) + 1;
        const QString fileName = JOB_FILENAMES[jobId % JOB_FILENAMES.length()];
        const auto hostInfo = hostInfoManager()->find(clientId);
        assert(hostInfo);
        if (qFuzzyIsNull(hostInfo->serverSpeed())) {
            continue;
        }

        Job job(jobId, clientId, fileName);
        time_t rawtime;
        time(&rawtime);
        job.startTime = rawtime;
        job.state = Job::Compiling;
        job.in_compressed = QRandomGenerator::global()->generate() % MAX_JOB_SIZE * 0.75; // random factor
        job.in_uncompressed = QRandomGenerator::global()->generate() % MAX_JOB_SIZE;
        job.in_compressed = QRandomGenerator::global()->generate() % MAX_JOB_SIZE ;
        job.in_uncompressed = QRandomGenerator::global()->generate() % MAX_JOB_SIZE * 0.75; // random factor
        job.real_msec = 200;
        const int serverId = ((jobId + 1) % MAX_HOST_COUNT) + 1;
        job.server = serverId;
        emit jobUpdated(job);
        m_activeJobs << job;
    }

    // clean up old jobs
    if (m_activeJobs.size() > MAX_TOTAL_JOB_COUNT) {
        Job job = m_activeJobs.first();
        m_activeJobs.removeFirst();
        job.state = Job::Finished;
        emit jobUpdated(job);
    }

    Q_FOREACH(const HostInfo * info, hostInfoManager()->hostMap().values()) {
        if (info->isOffline()) {
            emit nodeRemoved(info->id());
        } else {
            emit nodeUpdated(info->id());
        }
    }
}

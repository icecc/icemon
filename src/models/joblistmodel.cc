/*
    This file is part of Icecream.

    Copyright (c) 2012 Kevin Funk <kfunk@kde.org>

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

#include "joblistmodel.h"

#include "hostinfo.h"
#include "monitor.h"

#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>

#include <algorithm>
#include <functional>

static QString formatByteSize(unsigned int value)
{
    static const QStringList units = {
        QStringLiteral("B"),
        QStringLiteral("KiB"),
        QStringLiteral("MiB")
    };

    int unit = 0;
    while (unit < units.size() && value > 1024) {
        ++unit;
        value /= 1024;
    }
    return QCoreApplication::tr("%1 %2").arg(QLocale::system().toString(value), units[unit]);
}

/**
 * Remove some of the parts of a file path
 *
 * @param numberOfFilePathParts
 *     If 0 => Return the file name, excluding the path
 *     If 1 => Return '.../ancestorDir/file.ext' if possible
 *     If 2 => ...
 */
static QString trimFilePath(const QString &filePath, int numberOfFilePathParts)
{
    const QChar separator = QDir::separator();
    if (numberOfFilePathParts == 0) {
        return filePath.mid(filePath.lastIndexOf(separator) + 1);
    }

    int counter = numberOfFilePathParts;
    int index = 0;
    do {
        index = filePath.lastIndexOf(separator, index - 1);
    } while (counter-- && index > 0);

    if (index > 0) {
        return QLatin1String("...") + filePath.mid(index);
    }

    return filePath;
}

JobListModel::JobListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_numberOfFilePathParts(2)
    , m_expireDuration(-1)
    , m_expireTimer(new QTimer(this))
    , m_jobType(AllJobs)
    , m_hostid(0)
{
    connect(m_expireTimer, SIGNAL(timeout()),
            this, SLOT(slotExpireFinishedJobs()));
}

Monitor *JobListModel::monitor() const
{
    return m_monitor;
}

void JobListModel::setMonitor(Monitor *monitor)
{
    if (m_monitor == monitor) {
        return;
    }

    if (m_monitor) {
        disconnect(m_monitor.data(), SIGNAL(jobUpdated(Job)), this, SLOT(updateJob(Job)));
    }
    m_monitor = monitor;
    if (m_monitor) {
        connect(m_monitor.data(), SIGNAL(jobUpdated(Job)), this, SLOT(updateJob(Job)));
    }
}

void JobListModel::setHostId(unsigned int hostid)
{
    if (m_hostid == hostid)
        return;
    m_hostid = hostid;
    clear();
}

void JobListModel::updateJob(const Job &job)
{
    const int index = m_jobs.indexOf(job);
    if (index != -1) {
        m_jobs[index] = job;
        emit dataChanged(indexForJob(job, 0), indexForJob(job, _JobColumnCount - 1));
    } else {
        if (m_hostid && m_jobType == RemoteJobs && job.server() != m_hostid)
            return;
        if (m_hostid && m_jobType == LocalJobs && job.client() != m_hostid)
            return;
        beginInsertRows(QModelIndex(), m_jobs.size(), m_jobs.size());
        m_jobs << job;
        endInsertRows();
    }

    const bool finished = (job.state() == Job::Finished || job.state() == Job::Failed);
    if (finished) {
        expireItem(job);
    }
}

void JobListModel::clear()
{
    beginResetModel();
    m_jobs.clear();
    m_finishedJobs.clear();
    endResetModel();
}

int JobListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _JobColumnCount;
}

Job JobListModel::jobForIndex(const QModelIndex &index) const
{
    return m_jobs.value(index.row());
}

QModelIndex JobListModel::indexForJob(const Job &job, int column)
{
    const int i = m_jobs.indexOf(job);
    return index(i, column);
}

QVariant JobListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case JobColumnID:
            return tr("ID");
        case JobColumnFilename:
            return tr("File name");
        case JobColumnClient:
            return tr("Client");
        case JobColumnServer:
            return tr("Server");
        case JobColumnState:
            return tr("State");
        case JobColumnReal:
            return tr("Real");
        case JobColumnUser:
            return tr("User");
        case JobColumnFaults:
            return tr("Faults");
        case JobColumnSizeIn:
            return tr("Size In");
        case JobColumnSizeOut:
            return tr("Size Out");
        }
    }
    return QAbstractListModel::headerData(section, orientation, role);
}

QVariant JobListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const Job job = jobForIndex(index);
    const int column = index.column();
    Q_ASSERT(m_monitor);
    const HostInfoManager *manager = m_monitor->hostInfoManager();
    if (role == Qt::DisplayRole) {
        switch (column) {
        case JobColumnID:
            return job.jobId();
        case JobColumnFilename:
            return trimFilePath(job.fileName(), m_numberOfFilePathParts);
        case JobColumnClient:
            return manager->nameForHost(job.client());
        case JobColumnServer:
            return manager->nameForHost(job.server());
        case JobColumnState:
            return job.stateAsString();
        case JobColumnReal:
            return job.real_msec;
        case JobColumnUser:
            return job.user_msec;
        case JobColumnFaults:
            return job.pfaults;
        case JobColumnSizeIn:
            return formatByteSize(job.in_uncompressed);
        case JobColumnSizeOut:
            return formatByteSize(job.out_uncompressed);
        default:
            break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        switch (column) {
        case JobColumnID:
            return Qt::AlignRight;
        case JobColumnReal:
            return Qt::AlignRight;
        case JobColumnUser:
            return Qt::AlignRight;
        case JobColumnFaults:
            return Qt::AlignRight;
        case JobColumnSizeIn:
            return Qt::AlignRight;
        case JobColumnSizeOut:
            return Qt::AlignRight;
        default:
            break;
        }
    }
    return QVariant();
}

QModelIndex JobListModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

int JobListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_jobs.size();
}

struct find_jobid
    : public std::unary_function<Job, bool>
{
public:
    find_jobid(unsigned int jobId)
        : m_jobId(jobId) {}

    bool operator()(const Job &job) const
    {
        return job.jobId() == m_jobId;
    }

private:
    unsigned int m_jobId;
};

void JobListModel::slotExpireFinishedJobs()
{
    const uint currentTime = QDateTime::currentDateTime().toTime_t();

    // this list is sorted by the age of the finished jobs, the oldest is the first
    // so we've to find the first job which isn't old enough to expire
    FinishedJobs::iterator it = m_finishedJobs.begin();
    for (const FinishedJobs::iterator itEnd = m_finishedJobs.end(); it != itEnd; ++it) {
        if (currentTime - (*it).time < ( uint )m_expireDuration) {
            break;
        }

        unsigned int jobId = (*it).jobId;
        removeItemById(jobId);
    }

    m_finishedJobs.erase(m_finishedJobs.begin(), it);

    if (m_finishedJobs.empty()) {
        m_expireTimer->stop();
    }
}

void JobListModel::removeItem(const Job &job)
{
    removeItemById(job.jobId());
}

void JobListModel::removeItemById(unsigned int jobId)
{
    QVector<Job>::iterator it = std::find_if(m_jobs.begin(), m_jobs.end(), find_jobid(jobId));
    int index = std::distance(m_jobs.begin(), it);
    beginRemoveRows(QModelIndex(), index, index);
    m_jobs.erase(it);
    endRemoveRows();
}

void JobListModel::expireItem(const Job &job)
{
    if (m_expireDuration == 0) {
        removeItem(job);
        return;
    }

    const uint currentTime = QDateTime::currentDateTime().toTime_t();
    m_finishedJobs.push_back(FinishedJob(currentTime, job.jobId()));

    if (!m_expireTimer->isActive()) {
        m_expireTimer->start(1000);
    }
}

JobListSortFilterProxyModel::JobListSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool JobListSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if ((left.column() != JobListModel::JobColumnSizeIn && left.column() != JobListModel::JobColumnSizeOut)
        || (right.column() != JobListModel::JobColumnSizeIn && right.column() != JobListModel::JobColumnSizeOut)) {
        return QSortFilterProxyModel::lessThan(left, right);
    }
    // Sort file sizes correctly, the view shows them already formatted in a way that wouldn't be correctly numerically
    // compared.
    const JobListModel *model = static_cast<JobListModel *>(sourceModel());
    Job jobLeft = model->jobForIndex(left);
    Job jobRight = model->jobForIndex(right);
    unsigned int leftValue = left.column() == JobListModel::JobColumnSizeIn ? jobLeft.in_uncompressed : jobLeft.out_uncompressed;
    unsigned int rightValue = right.column() == JobListModel::JobColumnSizeIn ? jobRight.in_uncompressed : jobRight.out_uncompressed;
    return leftValue < rightValue;
}

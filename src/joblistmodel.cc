/*
    This file is part of Icecream.

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

#include "joblistmodel.h"

#include "hostinfo.h"

#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>

#include <algorithm>
#include <functional>

// TODO: Find a suitable replace method for KLocale's formatByteSize
static QString formatByteSize(int bytes)
{
    return QCoreApplication::tr("%1 B").arg( QLocale::system().toString(bytes) );
}

/**
 * Remove some of the parts of a file path
 *
 * @param numberOfFilePathParts
 *     If 0 => Return the file name, excluding the path
 *     If 1 => Return '.../ancestorDir/file.ext' if possible
 *     If 2 => ...
 */
static QString trimFilePath(const QString& filePath, int numberOfFilePathParts)
{
    const QChar separator = QDir::separator();
    if (numberOfFilePathParts == 0) {
        return filePath.mid(filePath.lastIndexOf( separator ) + 1);
    }

    int counter = numberOfFilePathParts;
    int index = 0;
    do {
        index = filePath.lastIndexOf( separator, index - 1);
    } while ( counter-- && index > 0 );

    if ( index > 0 )
        return QString::fromLatin1( "..." ) + filePath.mid( index );

    return filePath;
}

JobListModel::JobListModel(HostInfoManager* manager, QObject* parent)
    : QAbstractListModel(parent)
    , m_hostInfoManager(manager)
    , m_numberOfFilePathParts(2)
    , m_expireDuration(-1)
    , m_expireTimer(new QTimer(this))
{
     connect(m_expireTimer, SIGNAL(timeout()),
             this, SLOT(slotExpireFinishedJobs()));
}

void JobListModel::update(const Job& job)
{
    const int index = m_jobs.indexOf(job);
    if (index != -1) {
        m_jobs[index] = job;
        QModelIndex index = indexForJob(job);
        emit dataChanged(index, index);
    } else {
        beginInsertRows(QModelIndex(), m_jobs.size(), m_jobs.size());
        m_jobs << job;
        endInsertRows();
    }

    const bool finished = (job.state() == Job::Finished || job.state() == Job::Failed);
    if (finished)
        expireItem(job);
}

void JobListModel::clear()
{
    m_jobs.clear();
    m_finishedJobs.clear();
    reset();
}

int JobListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return _JobColumnCount;
}

Job JobListModel::jobForIndex(const QModelIndex& index) const
{
    return m_jobs.value(index.row());
}

QModelIndex JobListModel::indexForJob(const Job& job)
{
    const int i = m_jobs.indexOf(job);
    return index(i);
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

QVariant JobListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const Job job = jobForIndex(index);
    const int column = index.column();
    const HostInfoManager* manager = m_hostInfoManager;
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
            return QString::number(job.real_msec);
        case JobColumnUser:
            return QString::number(job.user_msec);
        case JobColumnFaults:
            return QString::number(job.pfaults);
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

QModelIndex JobListModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

int JobListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_jobs.size();
}

struct find_jobid : public std::unary_function<Job, bool>
{
public:
    find_jobid(unsigned int jobId) : m_jobId(jobId) {}

    bool operator()(const Job& job) const
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
    for (const FinishedJobs::iterator itEnd = m_finishedJobs.end(); it != itEnd; ++it)
    {
        if (currentTime - (*it).time < (uint)m_expireDuration)
            break;

        unsigned int jobId = (*it).jobId;
        removeItemById(jobId);
    }

    m_finishedJobs.erase(m_finishedJobs.begin(), it);

    if (m_finishedJobs.empty())
        m_expireTimer->stop();
}

void JobListModel::removeItem(const Job& job)
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

void JobListModel::expireItem(const Job& job)
{
    if (m_expireDuration == 0) {
        removeItem( job );
        return;
    }

    const uint currentTime = QDateTime::currentDateTime().toTime_t();
    m_finishedJobs.push_back(FinishedJob(currentTime, job.jobId()));

    if (!m_expireTimer->isActive())
        m_expireTimer->start(1000);
}

#include "joblistmodel.moc"

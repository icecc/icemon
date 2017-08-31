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

#include "hostlistmodel.h"
#include "monitor.h"

#include <QLocale>
#include <QApplication>
#include <QPalette>

#include <algorithm>

HostListModel::HostListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

Monitor *HostListModel::monitor() const
{
    return m_monitor;
}

void HostListModel::setMonitor(Monitor *monitor)
{
    if (m_monitor == monitor) {
        return;
    }

    if (m_monitor) {
        disconnect(m_monitor.data(), SIGNAL(nodeRemoved(HostId)), this, SLOT(removeNodeById(HostId)));
        disconnect(m_monitor.data(), SIGNAL(nodeUpdated(HostId)), this, SLOT(checkNode(HostId)));
    }

    beginResetModel();
    m_hostInfos.clear();
    m_monitor = monitor;
    fill();
    endResetModel();

    if (m_monitor) {
        connect(m_monitor.data(), SIGNAL(nodeRemoved(HostId)), this, SLOT(removeNodeById(HostId)));
        connect(m_monitor.data(), SIGNAL(nodeUpdated(HostId)), this, SLOT(checkNode(HostId)));
    }
}

QVariant HostListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ColumnID:
            return tr("ID");
        case ColumnName:
            return tr("Name");
        case ColumnNoRemote:
            return tr("No remote?");
        case ColumnColor:
            return tr("Color");
        case ColumnIP:
            return tr("IP");
        case ColumnPlatform:
            return tr("Platform");
        case ColumnMaxJobs:
            return tr("Max Jobs");
        case ColumnSpeed:
            return tr("Speed");
        case ColumnLoad:
            return tr("Load");
        default:
            break;
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant HostListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const HostInfo info = hostInfoForIndex(index);
    const int column = index.column();
    if (role == HostIdRole) {
        return info.id();
    } else if (role == Qt::DisplayRole) {
        switch (column) {
        case ColumnID:
            return info.id();
        case ColumnName:
            return info.name();
        case ColumnNoRemote:
            return info.noRemote() ? tr("Yes") : QLatin1String("");
        case ColumnColor:
            return HostInfo::colorName(info.color());
        case ColumnIP:
            return info.ip();
        case ColumnPlatform:
            return info.platform();
        case ColumnMaxJobs:
            return info.maxJobs();
        case ColumnSpeed:
            return int( info.serverSpeed());
        case ColumnLoad:
            return info.serverLoad();
        default:
            break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        switch (column) {
        case ColumnID:
            return Qt::AlignRight;
        case ColumnNoRemote:
            return Qt::AlignCenter;
        case ColumnMaxJobs:
            return Qt::AlignRight;
        case ColumnSpeed:
            return Qt::AlignRight;
        case ColumnLoad:
            return Qt::AlignRight;
        default:
            break;
        }
    } else if (role == Qt::BackgroundRole) {
        if (info.noRemote()) {
            return QApplication::palette().color(QPalette::Disabled, QPalette::Base);
        }
    } else if (role == Qt::ForegroundRole) {
        if (info.noRemote()) {
            return QApplication::palette().color(QPalette::Disabled, QPalette::Text);
        }
    }
    return QVariant();
}

int HostListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _ColumnCount;
}

int HostListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_hostInfos.size();
}

QModelIndex HostListModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

HostInfo HostListModel::hostInfoForIndex(const QModelIndex &index) const
{
    return m_hostInfos.value(index.row());
}

QModelIndex HostListModel::indexForHostInfo(const HostInfo &info, int column) const
{
    const int i = m_hostInfos.indexOf(info);
    return index(i, column);
}

void HostListModel::checkNode(unsigned int hostid)
{
    Q_ASSERT(m_monitor);

    const HostInfo *info = m_monitor->hostInfoManager()->find(hostid);
    if (!info) {
        return;
    }

    const int index = m_hostInfos.indexOf(*info);
    if (index != -1) {
        if (info->isOffline()) {
            removeNodeById(hostid);
        } else {
            m_hostInfos[index] = *info;
            emit dataChanged(indexForHostInfo(*info, 0), indexForHostInfo(*info, _ColumnCount - 1));
        }
    } else if (!info->isOffline()) {
        beginInsertRows(QModelIndex(), m_hostInfos.size(), m_hostInfos.size());
        m_hostInfos << *info;
        endInsertRows();
    }
}

struct find_hostid
    : public std::unary_function<HostInfo, bool>
{
public:
    explicit find_hostid(unsigned int hostId)
        : m_hostId(hostId) {}

    bool operator()(const HostInfo &info) const
    {
        return info.id() == m_hostId;
    }

private:
    unsigned int m_hostId;
};

void HostListModel::removeNodeById(unsigned int hostId)
{
    QVector<HostInfo>::iterator it = std::find_if(m_hostInfos.begin(), m_hostInfos.end(), find_hostid(hostId));
    int index = std::distance(m_hostInfos.begin(), it);
    beginRemoveRows(QModelIndex(), index, index);
    m_hostInfos.erase(it);
    endRemoveRows();
}

void HostListModel::fill()
{
    if (!m_monitor) {
        return;
    }

    const HostInfoManager *manager = m_monitor->hostInfoManager();
    const HostInfoManager::HostMap hosts(manager->hostMap());
    foreach(int hostid, hosts.keys()) {
        const HostInfo *info = m_monitor->hostInfoManager()->find(hostid);
        if (!info) {
            continue;
        }

        m_hostInfos << *info;
    }
}

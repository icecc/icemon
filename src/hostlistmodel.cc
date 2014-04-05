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

#include <algorithm>
#include <QLocale>

HostListModel::HostListModel(HostInfoManager* manager, QObject* parent)
    : QAbstractListModel(parent)
    , m_hostInfoManager(manager)
{
}

QVariant HostListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case ColumnID:
            return tr("ID");
        case ColumnName:
            return tr("Name");
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

QVariant HostListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

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
        case ColumnColor:
            return HostInfo::colorName(info.color());
        case ColumnIP:
            return info.ip();
        case ColumnPlatform:
            return info.platform();
        case ColumnMaxJobs:
            return info.maxJobs();
        case ColumnSpeed:
            return int(info.serverSpeed());
        case ColumnLoad:
            return info.serverLoad();
        default:
            break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        switch (column) {
        case ColumnID:
            return Qt::AlignRight;
        case ColumnMaxJobs:
            return Qt::AlignRight;
        case ColumnSpeed:
            return Qt::AlignRight;
        case ColumnLoad:
            return Qt::AlignRight;
        default:
            break;
        }
    }
    return QVariant();
}

int HostListModel::columnCount(const QModelIndex& parent) const
{
    return _ColumnCount;
}

int HostListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_hostInfos.size();
}

QModelIndex HostListModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

HostInfo HostListModel::hostInfoForIndex(const QModelIndex& index) const
{
    return m_hostInfos.value(index.row());
}

QModelIndex HostListModel::indexForHostInfo(const HostInfo& info, int column) const
{
    const int i = m_hostInfos.indexOf(info);
    return index(i, column);
}

void HostListModel::checkNode(unsigned int hostid)
{
    const HostInfo* info = m_hostInfoManager->find( hostid );
    if (!info)
        return;

    const int index = m_hostInfos.indexOf(*info);
    if (index != -1) {
        m_hostInfos[index] = *info;
        emit dataChanged(indexForHostInfo(*info, 0), indexForHostInfo(*info, _ColumnCount - 1));
    } else {
        beginInsertRows(QModelIndex(), m_hostInfos.size(), m_hostInfos.size());
        m_hostInfos << *info;
        endInsertRows();
    }
}

struct find_hostid : public std::unary_function<HostInfo, bool>
{
public:
    find_hostid(unsigned int hostId) : m_hostId(hostId) {}

    bool operator()(const HostInfo& info) const
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

void HostListModel::clear()
{
    m_hostInfos.clear();
    reset();
}


#include "hostlistmodel.moc"

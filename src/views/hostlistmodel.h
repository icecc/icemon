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

#ifndef ICEMON_HOSTLISTMODEL_H
#define ICEMON_HOSTLISTMODEL_H

#include <QAbstractItemModel>

#include "hostinfo.h"

class HostListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Column
    {
        ColumnID,
        ColumnName,
        ColumnNoRemote,
        ColumnColor,
        ColumnIP,
        ColumnPlatform,
        ColumnMaxJobs,
        ColumnSpeed,
        ColumnLoad,
        _ColumnCount
    };

    enum Role
    {
        HostIdRole = Qt::UserRole
    };

    explicit HostListModel(HostInfoManager* manager, QObject* parent = 0);

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual int columnCount(const QModelIndex& parent) const;
    virtual int rowCount(const QModelIndex& parent) const;
    virtual QModelIndex parent(const QModelIndex& child) const;

    void checkNode(unsigned int hostId);
    void removeNodeById(unsigned int hostId);
    void clear();

    HostInfo hostInfoForIndex(const QModelIndex& index) const;
    QModelIndex indexForHostInfo(const HostInfo& info, int column) const;

private:
    const HostInfoManager* m_hostInfoManager;

    QVector<HostInfo> m_hostInfos;

};

#endif // ICEMON_HOSTLISTMODEL_H

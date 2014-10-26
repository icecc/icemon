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
#include <QPointer>

#include "hostinfo.h"
#include "types.h"

class Monitor;

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

    explicit HostListModel(QObject* parent = nullptr);

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual int columnCount(const QModelIndex& parent) const override;
    virtual int rowCount(const QModelIndex& parent) const override;
    virtual QModelIndex parent(const QModelIndex& child) const override;

    Monitor* monitor() const;
    void setMonitor(Monitor* monitor);

    HostInfo hostInfoForIndex(const QModelIndex& index) const;
    QModelIndex indexForHostInfo(const HostInfo& info, int column) const;

private Q_SLOTS:
    void checkNode(HostId hostId);
    void removeNodeById(HostId hostId);

private:
    void fill();

    QPointer<Monitor> m_monitor;
    QVector<HostInfo> m_hostInfos;
};

#endif // ICEMON_HOSTLISTMODEL_H

class Monitor;

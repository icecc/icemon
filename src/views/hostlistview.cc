/*
    This file is part of Icecream.

    Copyright (c) 2004-2006 Andre Wöbbeking <Woebbeking@web.de>
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

#include "hostlistview.h"

#include "models/hostlistmodel.h"

#include <QHeaderView>

HostListView::HostListView(QWidget *parent)
    : QTreeView(parent)
{
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setWindowTitle(tr("Hosts"));
}

void HostListView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
    if (model) {
        sortByColumn(HostListModel::ColumnID);
        header()->setStretchLastSection(false);
        header()->setSectionResizeMode(HostListModel::ColumnName, QHeaderView::Stretch);
        header()->setSectionResizeMode(HostListModel::ColumnColor, QHeaderView::ResizeToContents);
        header()->setSectionResizeMode(HostListModel::ColumnIP, QHeaderView::ResizeToContents);
        header()->setSectionResizeMode(HostListModel::ColumnPlatform, QHeaderView::ResizeToContents);
    }
}

/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2004, 2005 Andre Wöbbeking <Woebbeking@web.de>
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

#include "joblistview.h"

#include "models/joblistmodel.h"

#include <QDebug>
#include <QHeaderView>

JobListView::JobListView(QWidget *parent)
    : QTreeView(parent)
{
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setWindowTitle(tr("Jobs"));
}

JobListModel *JobListView::jobListModel() const
{
    return qobject_cast<JobListModel *>(model());
}

void JobListView::setModel(QAbstractItemModel *model)
{
    if (model) {
        sortByColumn(JobListModel::JobColumnID);
    }

    QTreeView::setModel(model);
}

bool JobListView::isClientColumnVisible() const
{
    return !isColumnHidden(JobListModel::JobColumnClient);
}

void JobListView::setClientColumnVisible(bool visible)
{
    if (visible == isClientColumnVisible()) {
        return;
    }

    if (visible) {
        setColumnHidden(JobListModel::JobColumnClient, false);
        resizeColumnToContents(JobListModel::JobColumnClient);
    } else {
        setColumnHidden(JobListModel::JobColumnClient, true);
    }
}

bool JobListView::isServerColumnVisible() const
{
    return !isColumnHidden(JobListModel::JobColumnServer);
}

void JobListView::setServerColumnVisible(bool visible)
{
    if (visible == isServerColumnVisible()) {
        return;
    }

    if (visible) {
        setColumnHidden(JobListModel::JobColumnServer, false);
        resizeColumnToContents(JobListModel::JobColumnServer);
    } else {
        setColumnHidden(JobListModel::JobColumnServer, true);
    }
}

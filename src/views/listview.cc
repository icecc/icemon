/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2004 Andre Wöbbeking <Woebbeking@web.de>
    Copyright (c) 2014 Allan Sandfeld Jensen <sandfeld@kde.org>

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

#include "listview.h"

#include "joblistview.h"
#include "models/joblistmodel.h"

#include <QBoxLayout>
#include <QSortFilterProxyModel>

ListStatusView::ListStatusView(QObject *parent)
    : StatusView(parent)
    , m_widget(new QWidget)
    , mJobsListView(new JobListView(m_widget.data()))
{
    mJobsListModel = new JobListModel(this);
    mSortedJobsListModel = new QSortFilterProxyModel(this);
    mSortedJobsListModel->setDynamicSortFilter(true);
    mSortedJobsListModel->setSourceModel(mJobsListModel);

    mJobsListView->setModel(mSortedJobsListModel);

    QVBoxLayout *topLayout = new QVBoxLayout(m_widget.data());
    topLayout->setMargin(0);
    topLayout->addWidget(mJobsListView);
}

StatusView::Options ListStatusView::options() const
{
    return RememberJobsOption;
}

QWidget *ListStatusView::widget() const
{
    return m_widget.data();
}

void ListStatusView::setMonitor(Monitor *monitor)
{
    StatusView::setMonitor(monitor);

    mJobsListModel->setMonitor(monitor);
}

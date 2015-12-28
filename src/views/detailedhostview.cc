/*
    This file is part of Icecream.

    Copyright (c) 2004-2006 Andre Wöbbeking <Woebbeking@web.de>
              (c) 2014 Allan Sandfeld Jensen <sandfeld@kde.org>

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

#include <QLabel>
#include <QBoxLayout>
#include <QSortFilterProxyModel>
#include <QSplitter>

#include "detailedhostview.h"

#include "joblistview.h"
#include "hostinfo.h"
#include "hostlistview.h"
#include "models/joblistmodel.h"
#include "models/hostlistmodel.h"

#include <sys/utsname.h>

static QString myHostName()
{
    struct utsname uname_buf;
    if (::uname(&uname_buf) == 0) {
        return QLatin1String(uname_buf.nodename);
    } else {
        return QString();
    }
}

DetailedHostView::DetailedHostView(QObject *parent)
    :  StatusView(parent)
    , m_widget(new QWidget)
{
    QBoxLayout *topLayout = new QVBoxLayout(m_widget.data());
    topLayout->setMargin(10);

    auto viewSplitter = new QSplitter(Qt::Vertical);
    topLayout->addWidget(viewSplitter);

    auto hosts = new QWidget(viewSplitter);
    auto dummy = new QVBoxLayout(hosts);
    dummy->setSpacing(10);
    dummy->setMargin(0);

    mHostListModel = new HostListModel(this);

    mSortedHostListModel = new QSortFilterProxyModel(this);
    mSortedHostListModel->setDynamicSortFilter(true);
    mSortedHostListModel->setSourceModel(mHostListModel);

    dummy->addWidget(new QLabel(tr("Hosts"), hosts));
    mHostListView = new HostListView(hosts);
    mHostListView->setModel(mSortedHostListModel);
    dummy->addWidget(mHostListView);
    //connect(mHostListView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
    //        SLOT(slotNodeActivated()));

    auto locals = new QWidget(viewSplitter);
    dummy = new QVBoxLayout(locals);
    dummy->setSpacing(10);
    dummy->setMargin(0);

    mLocalJobsModel = new JobListModel(this);
    mLocalJobsModel->setExpireDuration(5);
    mSortedLocalJobsModel = new JobListSortFilterProxyModel(this);
    mSortedLocalJobsModel->setDynamicSortFilter(true);
    mSortedLocalJobsModel->setSourceModel(mLocalJobsModel);

    dummy->addWidget(new QLabel(tr("Outgoing jobs"), locals));
    mLocalJobsView = new JobListView(locals);
    mLocalJobsView->setModel(mSortedLocalJobsModel);
    mLocalJobsView->setClientColumnVisible(false);
    dummy->addWidget(mLocalJobsView);

    auto remotes = new QWidget(viewSplitter);
    dummy = new QVBoxLayout(remotes);
    dummy->setSpacing(10);
    dummy->setMargin(0);

    mRemoteJobsModel = new JobListModel(this);
    mRemoteJobsModel->setExpireDuration(5);
    mSortedRemoteJobsModel = new JobListSortFilterProxyModel(this);
    mSortedRemoteJobsModel->setDynamicSortFilter(true);
    mSortedRemoteJobsModel->setSourceModel(mRemoteJobsModel);

    dummy->addWidget(new QLabel(tr("Incoming jobs"), remotes));
    mRemoteJobsView = new JobListView(remotes);
    mRemoteJobsView->setModel(mSortedRemoteJobsModel);
    mRemoteJobsView->setServerColumnVisible(false);
    dummy->addWidget(mRemoteJobsView);

    createKnownHosts();
}

void DetailedHostView::setMonitor(Monitor *monitor)
{
    StatusView::setMonitor(monitor);

    mHostListModel->setMonitor(monitor);
    mLocalJobsModel->setMonitor(monitor);
    mRemoteJobsModel->setMonitor(monitor);

    createKnownHosts();
}

void DetailedHostView::checkNode(unsigned int hostid)
{
    if (!hostid) {
        return;
    }

    if (!mHostListView->selectionModel()->hasSelection()) {
        HostInfo *info = hostInfoManager()->find(hostid);
        if (info->name() == myHostName()) {
            mHostListView->setCurrentIndex(mSortedHostListModel->mapFromSource(mHostListModel->indexForHostInfo(*info, 0)));
        }
    }
}

void DetailedHostView::createKnownHosts()
{
    if (!hostInfoManager()) {
        return;
    }

    const HostInfoManager::HostMap hosts(hostInfoManager()->hostMap());
    foreach(int hostid, hosts.keys()) {
        checkNode(hostid);
    }
}

QWidget *DetailedHostView::widget() const
{
    return m_widget.data();
}

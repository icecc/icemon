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
#include "joblistmodel.h"

#include <QBoxLayout>
#include <QSortFilterProxyModel>

ListStatusView::ListStatusView( HostInfoManager* manager,
                                QWidget* parent )
    : QWidget( parent ),
      StatusView( manager ),
      mJobsListView( new JobListView(this) )
{
    mJobsListModel = new JobListModel(manager, this);
    mSortedJobsListModel = new QSortFilterProxyModel(this);
    mSortedJobsListModel->setSourceModel(mJobsListModel);

    mJobsListView->setModel(mSortedJobsListModel);

    QVBoxLayout* topLayout = new QVBoxLayout( this );
    topLayout->setMargin(0);
    topLayout->addWidget( mJobsListView );
}

void ListStatusView::update( const Job &job )
{
    mJobsListModel->update( job );
    mSortedJobsListModel->invalidate();
}

#include "listview.moc"

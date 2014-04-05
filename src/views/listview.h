/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2004 Andre Wöbbeking <Woebbeking@web.de>

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
#ifndef ICEMON_LISTVIEW_H
#define ICEMON_LISTVIEW_H

#include "statusview.h"

#include <QWidget>

class JobListModel;
class JobListView;
class QSortFilterProxyModel;

class ListStatusView :public QWidget, public StatusView
{
    Q_OBJECT
public:
    ListStatusView( HostInfoManager* manager, QWidget* parent );

    virtual Options options() const;

    virtual QWidget* widget() { return this; }

    virtual void update( const Job& job );

    virtual QString id() const { return "list"; }

private:
    JobListView* mJobsListView;
    JobListModel* mJobsListModel;
    QSortFilterProxyModel* mSortedJobsListModel;
};

#endif
// vim:ts=4:sw=4:noet

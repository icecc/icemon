/*
    This file is part of Icecream.

    Copyright (c) 2015 Kevin Funk <kfunk@kde.org>

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

#include "statusviewfactory.h"

#include "views/starview.h"
#include "views/summaryview.h"
#include "views/detailedhostview.h"
#include "views/ganttstatusview.h"
#include "views/listview.h"
#include "views/flowtableview.h"

StatusViewFactory::StatusViewFactory()
{
}

StatusView *StatusViewFactory::create(const QString &id, QObject *parent) const
{
    if (id == QLatin1String("list")) {
        return new ListStatusView(parent);
    } else if (id == QLatin1String("gantt")) {
        return new GanttStatusView(parent);
    } else if (id == QLatin1String("summary")) {
        return new SummaryView(parent);
    } else if (id == QLatin1String("flow")) {
        return new FlowTableView(parent);
    } else if (id == QLatin1String("detailedhost")) {
        return new DetailedHostView(parent);
    }

    return new StarView(parent);
}

/*
    This file is part of Icecream.

    Copyright (c) 2004 Andre Wöbbeking <Woebbeking@web.de>
    Copyright (c) 2006 Luboš Luňák <l.lunak@kde.org>
    Copyright (c) 2006 Dirk Mueller <mueller@kde.org>
    Copyright (c) 2006 Aaron Seigo <aseigo@kde.org>
    Copyright (c) 2007 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2008 Urs Wolfer <uwolfer@kde.org>
    Copyright (c) 2012 Kevin Funk <kevin@kfunk.org>

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

#ifndef ICEMON_HOSTLISTVIEW_H
#define ICEMON_HOSTLISTVIEW_H

#include "hostinfo.h"

#include <QTreeView>

class HostListView :public QTreeView
{
    Q_OBJECT

public:
    HostListView( HostInfoManager* manager, QWidget* parent );

    virtual void setModel(QAbstractItemModel* model);

private:
    HostInfoManager* mHostInfoManager;
};

#endif

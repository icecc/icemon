/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef ICEMON_LISTVIEW_H
#define ICEMON_LISTVIEW_H

#include "mon-kde.h"

#include <klistview.h>

class ListStatusViewItem : public KListViewItem
{
public:
    ListStatusViewItem( KListView *parent, const Job &job );
    void updateText( const Job &job);
    int compare( QListViewItem *i, int col, bool ascending ) const;
private:
    Job job;
};

class ListStatusView :public KListView, public StatusView
{
    Q_OBJECT
public:
    ListStatusView( HostInfoManager *, QWidget *parent, const char *name = 0 );
    virtual ~ListStatusView() {}
    virtual QWidget *widget() { return this; }
    virtual void update( const Job &job );

    QString id() const { return "list"; }

    void removeJob( const Job& job );

private:
    typedef QMap<unsigned int, ListStatusViewItem*> ItemMap;
    ItemMap items;
};

#endif
// vim:ts=4:sw=4:noet

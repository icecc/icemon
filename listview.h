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

#include <qdatetime.h>
#include <qpair.h>

class QTimer;

class ListStatusViewItem : public KListViewItem
{
public:
    ListStatusViewItem( KListView *parent, const Job &job );

    const Job& job() const { return mJob; }

    void updateText( const Job &job);

    void updateFileName();

    virtual int compare( QListViewItem *i, int col, bool ascending ) const;
private:
    Job mJob;
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

    int numberOfFilePathParts() const;
    void setNumberOfFilePathParts( int number );

    bool isClientColumnVisible() const;
    void setClientColumnVisible( bool visible );

    bool isServerColumnVisible() const;
    void setServerColumnVisible( bool visible );

    int expireDuration() const;
    void setExpireDuration( int duration );

private slots:

    void slotExpireFinishedJobs();

private:

    void expireItem( ListStatusViewItem* item );

    void removeItem( ListStatusViewItem* item );

    typedef QMap<unsigned int, ListStatusViewItem*> ItemMap;
    ItemMap items;

    /**
     * Number of parts (directories) of the file path which should be displayed.
     * -   < 0 for complete file path
     * -  == 0 for the pure file name without path
     * -   > 0 for only parts of the file path. If there are not enough parts
     *    the complete file path is displayed else .../partN/.../part1/fileName.
     * Default is 2.
     */
    int mNumberOfFilePathParts;

    /**
     * The number of seconds after which finished jobs should be expired.
     * -  < 0 never
     * - == 0 at once
     * -  > 0 after some seconds.
     */
    int mExpireDuration;

    QTimer* mExpireTimer;

    typedef QPair<QTime, ListStatusViewItem*> FinishedJob;
    typedef QValueList<FinishedJob> FinishedJobs;
    FinishedJobs mFinishedJobs;
};

#endif
// vim:ts=4:sw=4:noet

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
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef ICEMON_JOBLISTVIEW_H
#define ICEMON_JOBLISTVIEW_H


#include "job.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPair>
#include <QList>


class HostInfoManager;

class QTimer;


class JobListViewItem : public QTreeWidgetItem
{
public:

    JobListViewItem( QTreeWidget* parent, const Job& job );

    const Job& job() const { return mJob; }

    void updateText( const Job& job);

    void updateFileName();

    virtual int compare( QTreeWidgetItem* item, int column, bool ascending ) const;

private:

    Job mJob;
};


class JobTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:

    JobTreeWidget( const HostInfoManager* manager, QWidget* parent = 0 );

    void update( const Job& job );

    int numberOfFilePathParts() const;
    void setNumberOfFilePathParts( int number );

    bool isClientColumnVisible() const;
    void setClientColumnVisible( bool visible );

    bool isServerColumnVisible() const;
    void setServerColumnVisible( bool visible );

    int expireDuration() const;
    void setExpireDuration( int duration );

    const HostInfoManager* hostInfoManager() const { return mHostInfoManager; }

    virtual void clear();

private slots:

    void slotExpireFinishedJobs();

private:

    void expireItem( JobListViewItem* item );

    void removeItem( JobListViewItem* item );

    const HostInfoManager* mHostInfoManager;

    typedef QMap<unsigned int, JobListViewItem*> ItemMap;
    ItemMap mItems;

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
     * Default is -1.
     */
    int mExpireDuration;

    QTimer* mExpireTimer;

    typedef QPair<uint, JobListViewItem*> FinishedJob;
    typedef QList<FinishedJob> FinishedJobs;
    FinishedJobs mFinishedJobs;
};


#endif

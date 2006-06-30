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

#include "listview.h"

#include "joblistview.h"

#include <qlayout.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

ListStatusView::ListStatusView( HostInfoManager* manager,
                                QWidget* parent,
                                const char* name )
    : QWidget( parent, name ),
      StatusView( manager ),
      mJobsListView( new JobListView( manager, this, "Jobs" ) )
{
    Q3VBoxLayout* topLayout = new Q3VBoxLayout( this );
    topLayout->addWidget( mJobsListView );
}

void ListStatusView::update( const Job &job )
{
    mJobsListView->update( job );
}

#include "listview.moc"

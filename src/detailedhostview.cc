/*
    This file is part of Icecream.

    Copyright (c) 2004-2006 Andre Wöbbeking <Woebbeking@web.de>

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
#include <QSplitter>

#include "detailedhostview.h"

#include "hostinfo.h"
#include "hostlistview.h"
#include "hostlistmodel.h"
#include "joblistview.h"
#include "joblistmodel.h"

#include <sys/utsname.h>


static QString myHostName()
{
    struct utsname uname_buf;
    if ( ::uname( &uname_buf ) == 0 )
        return uname_buf.nodename;
    else
        return QString();
}

DetailedHostView::DetailedHostView( HostInfoManager* manager,
                                    QWidget* parent )
    : QWidget( parent ),
      StatusView( manager )
{
  QBoxLayout* topLayout = new QVBoxLayout( this );
  topLayout->setMargin( 10 );

  QSplitter* viewSplitter = new QSplitter( Qt::Vertical, this );
  topLayout->addWidget( viewSplitter );

  QWidget *hosts = new QWidget( viewSplitter );
  QVBoxLayout *dummy = new QVBoxLayout( hosts );
  dummy->setSpacing( 10 );
  dummy->setMargin( 0 );

  mHostListModel = new HostListModel(manager, this);

  dummy->addWidget(new QLabel( tr("Hosts" ), hosts ));
  mHostListView = new HostListView( manager, hosts );
  mHostListView->setModel(mHostListModel);
  dummy->addWidget(mHostListView);
  connect(mHostListView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
          SLOT(slotNodeActivated()));

  QWidget *locals = new QWidget( viewSplitter );
  dummy = new QVBoxLayout( locals );
  dummy->setSpacing( 10 );
  dummy->setMargin( 0 );

  mLocalJobsModel = new JobListModel(manager, this);
  mLocalJobsModel->setExpireDuration(5);

  dummy->addWidget(new QLabel( tr("Outgoing jobs" ), locals ));
  mLocalJobsView = new JobListView(locals);
  mLocalJobsView->setModel(mLocalJobsModel);
  mLocalJobsView->setClientColumnVisible( false );
  dummy->addWidget(mLocalJobsView);

  QWidget* remotes = new QWidget( viewSplitter );
  dummy = new QVBoxLayout( remotes );
  dummy->setSpacing( 10 );
  dummy->setMargin( 0 );

  mRemoteJobsModel = new JobListModel(manager, this);
  mRemoteJobsModel->setExpireDuration(5);

  dummy->addWidget(new QLabel( tr("Incoming jobs" ), remotes ));
  mRemoteJobsView = new JobListView(remotes);
  mRemoteJobsView->setModel(mRemoteJobsModel);
  mRemoteJobsView->setServerColumnVisible( false );
  dummy->addWidget(mRemoteJobsView);

  createKnownHosts();
}


void DetailedHostView::update( const Job &job )
{
    const unsigned int hostid = mHostListView->currentIndex().data(HostListModel::HostIdRole).value<unsigned int>();
    if ( !hostid )
        return;

    if ( job.client() != hostid && job.server() != hostid )
        return;

    if ( job.client() == hostid )
        mLocalJobsView->update( job );
    if ( job.server() == hostid )
        mRemoteJobsView->update( job );
}


void DetailedHostView::checkNode( unsigned int hostid )
{
    if ( !hostid )
        return;

    mHostListModel->checkNode( hostid );

    if ( !mHostListView->selectionModel()->hasSelection() ) {
        HostInfo* info = hostInfoManager()->find( hostid );
        if ( info->name() == myHostName() ) {
            const QModelIndex index = mHostListModel->indexForHostInfo(*info);
            mHostListView->selectionModel()->select( index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
}


void DetailedHostView::removeNode( unsigned int hostid )
{
    mHostListModel->removeNodeById( hostid );
}


void DetailedHostView::updateSchedulerState( bool online )
{
    if ( !online )
    {
        mHostListModel->clear();
        mLocalJobsModel->clear();
        mRemoteJobsModel->clear();
    }
}


void DetailedHostView::slotNodeActivated()
{
    mLocalJobsModel->clear();
    mRemoteJobsModel->clear();
}


void DetailedHostView::createKnownHosts()
{
    const HostInfoManager::HostMap& hosts(hostInfoManager()->hostMap());

    for (HostInfoManager::HostMap::ConstIterator it( hosts.begin() ),
                                                 itEnd( hosts.end() );
         it != itEnd; ++it )
    {
        const unsigned int hostid( (*it)->id() );

        checkNode( hostid );
    }
}


QWidget* DetailedHostView::widget()
{
  return this;
}

#include "detailedhostview.moc"

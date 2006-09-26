/*
    This file is part of Icecream.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "hostview.h"

#include "hostinfo.h"
#include "job.h"

#include <kled.h>
#include <klocale.h>

#include <QDebug>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>

#include <sys/utsname.h>

using namespace std;

HostViewConfigDialog::HostViewConfigDialog( QWidget *parent )
  : QDialog( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  QLabel *label = new QLabel( i18n("Host name:"), this );
  topLayout->addWidget( label );

  mHostNameEdit = new QLineEdit( this );
  topLayout->addWidget( mHostNameEdit );

  mHostNameEdit->setText( myHostName() );

  QBoxLayout *buttonLayout = new QHBoxLayout();
  topLayout->addLayout( buttonLayout );

  buttonLayout->addStretch( 1 );

  QPushButton *button = new QPushButton( i18n("&OK"), this );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotOk() ) );
}

QString HostViewConfigDialog::myHostName() const
{
  struct utsname uname_buf;
  if ( uname( &uname_buf ) == 0 ) {
    return uname_buf.nodename;
  } else {
    return QString::null;
  }
}

void HostViewConfigDialog::slotOk()
{
  hide();
  emit configChanged();
}

QString HostViewConfigDialog::hostName() const
{
  return mHostNameEdit->text();
}


HostView::HostView( bool detailed, HostInfoManager *m, QWidget *parent,
                    const char *name )
  : QWidget( parent, Qt::WNoAutoErase | Qt::WResizeNoErase ), StatusView( m ),
    mHostId( 0 )
{
  setObjectName( name );

  mConfigDialog = new HostViewConfigDialog( this );
  connect( mConfigDialog, SIGNAL( configChanged() ),
           SLOT( slotConfigChanged() ) );

  QBoxLayout *topLayout = new QVBoxLayout( this );

  QBoxLayout *statusLayout = new QVBoxLayout();
  topLayout->addLayout( statusLayout );

  QBoxLayout *marginLayout = new QVBoxLayout();
  statusLayout->addLayout( marginLayout );
  marginLayout->addStretch( 1 );

  QBoxLayout *ledLayout = new QHBoxLayout();
  marginLayout->addLayout( ledLayout );
  ledLayout->setMargin( 4 );
  ledLayout->setSpacing( 4 );

  ledLayout->addStretch( 1 );

  mOwnLed = new KLed( QColor( Qt::red ), this );
  ledLayout->addWidget( mOwnLed );

  mOthersLed = new KLed( QColor( Qt::green ), this );
  ledLayout->addWidget( mOthersLed );

  ledLayout->addStretch( 1 );

  marginLayout->addStretch( 1 );

  mHostNameLabel = new QLabel( this );
  mHostNameLabel->setAlignment( Qt::AlignCenter );
  statusLayout->addWidget( mHostNameLabel, 1 );

  QWidget *jobWidget = new QWidget( this );
  topLayout->addWidget( jobWidget );

  QGridLayout *jobLayout = new QGridLayout( jobWidget );

  QLabel *label = new QLabel( i18n("Local jobs:"), jobWidget );
  jobLayout->addWidget( label, 0, 0 );
  mLocalJobsLabel = new QLabel( jobWidget );
  jobLayout->addWidget( mLocalJobsLabel, 0, 1 );

  label = new QLabel( i18n("Remote jobs:"), jobWidget );
  jobLayout->addWidget( label, 1, 0 );
  mRemoteJobsLabel = new QLabel( jobWidget );
  jobLayout->addWidget( mRemoteJobsLabel, 1, 1 );

  label = new QLabel( i18n("Compile jobs:"), jobWidget );
  jobLayout->addWidget( label, 2, 0 );
  mCompileJobsLabel = new QLabel( jobWidget );
  jobLayout->addWidget( mCompileJobsLabel, 2, 1 );

  if ( !detailed ) jobWidget->hide();

  slotConfigChanged();

  updateJobLabels();
}

void HostView::update( const Job &job )
{
#if 0
  kDebug() << "HostView::update() " << job.jobId()
            << " server: " << job.server() << " client: " << job.client()
            << " state: " << job.stateAsString() << endl;
#endif

  if ( job.client() != mHostId && job.server() != mHostId ) return;

  bool finished = job.state() == Job::Finished || job.state() == Job::Failed;

  if ( finished ) {
    if ( mLocalJobs.contains( job.jobId() ) )
      mLocalJobs.removeAll( job.jobId() );

    if ( mRemoteJobs.contains( job.jobId() ) )
      mRemoteJobs.removeAll( job.jobId() );

    if ( mCompileJobs.contains( job.jobId() ) )
      mCompileJobs.removeAll( job.jobId() );

    updateJobLabels();
    return;
  }

  if ( job.state() == Job::LocalOnly ) {
    if ( job.client() != mHostId ) return;
    mLocalJobs.append( job.jobId() );
  } else if ( job.state() == Job::Compiling ) {
    if ( job.client() == mHostId ) {
      mRemoteJobs.append( job.jobId() );
    }
    if ( job.server() == mHostId ) {
      mCompileJobs.append( job.jobId() );
    }
  } else {
    return;
  }

  updateJobLabels();
}

void HostView::updateJobLabels()
{
  mLocalJobsLabel->setText( QString::number( mLocalJobs.count() ) );
  mRemoteJobsLabel->setText( QString::number( mRemoteJobs.count() ) );
  mCompileJobsLabel->setText( QString::number( mCompileJobs.count() ) );

  if ( mLocalJobs.count() > 0 ) {
    mOwnLed->setColor( QColor( "organge" ) );
    mOwnLed->on();
  } else if ( mRemoteJobs.count() > 0 ) {
    mOwnLed->setColor( QColor( Qt::green ) );
    mOwnLed->on();
  } else {
    mOwnLed->off();
  }

  if ( mCompileJobs.count() > 0 ) {
    mOthersLed->on();
  } else {
    mOthersLed->off();
  }
}

void HostView::checkNode( unsigned int hostid )
{
  if ( !hostid ) return;

//  kDebug() << "HostView::checkNode(): " << hostid << endl;

  if ( mHostId == 0 ) {
    HostInfo *info = hostInfoManager()->find( hostid );
    if ( info->name() == mConfigDialog->hostName() ) {
      mHostId = hostid;
      mHostNameLabel->setText( mConfigDialog->hostName() );
      QPalette pal = palette();
      pal.setColor( backgroundRole(), info->color() );
      setPalette( pal );
      
      pal = mHostNameLabel->palette();
      pal.setColor( mHostNameLabel->backgroundRole(), info->color() );
      mHostNameLabel->setPalette( pal );

      pal = mOwnLed->palette();
      pal.setColor( mOwnLed->backgroundRole(), info->color() );
      mOwnLed->setPalette( pal );

      pal = mOthersLed->palette();
      pal.setColor( mOthersLed->backgroundRole(), info->color() );
      mOthersLed->setPalette( pal );

      pal = mHostNameLabel->palette();
      pal.setColor( mHostNameLabel->foregroundRole(), textColor( info->color() ) );
      mHostNameLabel->setPalette( pal );
      repaint();
    }
  }
}

void HostView::removeNode( unsigned int hostid )
{
  qDebug() << "HostView::removeNode(): " << hostid;

  if ( hostid != mHostId ) return;
}

void HostView::updateSchedulerState( bool online )
{
  if ( online ) {
    mOwnLed->show();
    mOthersLed->show();
  } else {
    mOwnLed->hide();
    mOthersLed->hide();
  }
}

QWidget *HostView::widget()
{
  return this;
}

void HostView::slotConfigChanged()
{
  mHostNameLabel->setText( mConfigDialog->hostName() );
}

void HostView::configureView()
{
  mConfigDialog->show();
  mConfigDialog->raise();
}

#include "hostview.moc"

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

#include <services/logging.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <kled.h>

#include <qlayout.h>
#include <qvaluelist.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>

#include <sys/utsname.h>

using namespace std;

HostViewConfigDialog::HostViewConfigDialog( QWidget *parent )
  : QDialog( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n("Host name:"), this );
  topLayout->addWidget( label );

  mHostNameEdit = new QLineEdit( this );
  topLayout->addWidget( mHostNameEdit );

  mHostNameEdit->setText( myHostName() );

  QBoxLayout *buttonLayout = new QHBoxLayout( topLayout );

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
  : QWidget( parent, name, WRepaintNoErase | WResizeNoErase ), StatusView( m ),
    mHostId( 0 )
{
  mConfigDialog = new HostViewConfigDialog( this );
  connect( mConfigDialog, SIGNAL( configChanged() ),
           SLOT( slotConfigChanged() ) );

  QBoxLayout *topLayout = new QVBoxLayout( this );

  QBoxLayout *statusLayout = new QVBoxLayout( topLayout );

  QBoxLayout *marginLayout = new QVBoxLayout( statusLayout );
  marginLayout->addStretch( 1 );

  QBoxLayout *ledLayout = new QHBoxLayout( marginLayout );
  ledLayout->setMargin( 4 );
  ledLayout->setSpacing( 4 );

  ledLayout->addStretch( 1 );

  mOwnLed = new KLed( "red", this );
  ledLayout->addWidget( mOwnLed );

  mOthersLed = new KLed( "green", this );
  ledLayout->addWidget( mOthersLed );

  ledLayout->addStretch( 1 );

  marginLayout->addStretch( 1 );

  mHostNameLabel = new QLabel( this );
  mHostNameLabel->setAlignment( AlignCenter );
  statusLayout->addWidget( mHostNameLabel, 1 );

  QWidget *jobWidget = new QWidget( this );
  topLayout->addWidget( jobWidget );

  QGridLayout *jobLayout = new QGridLayout( jobWidget );
  jobLayout->setSpacing( KDialog::spacingHint() );
  jobLayout->setMargin( KDialog::marginHint() );

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
  kdDebug() << "HostView::update() " << job.jobId()
            << " server: " << job.server() << " client: " << job.client()
            << " state: " << job.stateAsString() << endl;
#endif

  if ( job.client() != mHostId && job.server() != mHostId ) return;

  bool finished = job.state() == Job::Finished || job.state() == Job::Failed;

  if ( finished ) {
    QValueList<unsigned int>::Iterator it;

    it = mLocalJobs.find( job.jobId() );
    if ( it != mLocalJobs.end() ) mLocalJobs.remove( it );

    it = mRemoteJobs.find( job.jobId() );
    if ( it != mRemoteJobs.end() ) mRemoteJobs.remove( it );

    it = mCompileJobs.find( job.jobId() );
    if ( it != mCompileJobs.end() ) mCompileJobs.remove( it );

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
    mOwnLed->setColor( "orange" );
    mOwnLed->on();
  } else if ( mRemoteJobs.count() > 0 ) {
    mOwnLed->setColor( "red" );
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

//  kdDebug() << "HostView::checkNode(): " << hostid << endl;

  if ( mHostId == 0 ) {
    HostInfo *info = hostInfoManager()->find( hostid );
    if ( info->name() == mConfigDialog->hostName() ) {
      mHostId = hostid;
      mHostNameLabel->setText( mConfigDialog->hostName() );
      setBackgroundColor( info->color() );
      mHostNameLabel->setBackgroundColor( info->color() );
      mOwnLed->setBackgroundColor( info->color() );
      mOthersLed->setBackgroundColor( info->color() );
      mHostNameLabel->setPaletteForegroundColor( textColor( info->color() ) );
      repaint();
    }
  }
}

void HostView::removeNode( unsigned int hostid )
{
  kdDebug() << "HostView::removeNode(): " << hostid << endl;

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

void HostView::resizeEvent( QResizeEvent *e )
{
  QWidget::resizeEvent( e );
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

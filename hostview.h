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
#ifndef ICEMON_HOSTVIEW_H
#define ICEMON_HOSTVIEW_H

#include "statusview.h"

#include <qdialog.h>

class KLed;

class QLabel;
class QLineEdit;
class QSlider;

class HostViewConfigDialog : public QDialog
{
    Q_OBJECT
  public:
    HostViewConfigDialog( QWidget *parent );

    QString hostName() const;

  protected slots:
    void slotOk();
  
    QString myHostName() const;
  
  signals:
    void configChanged();

  private:
    QLineEdit *mHostNameEdit;
};


class HostView : public QWidget, public StatusView
{
    Q_OBJECT
  public:
    HostView( bool detailed, HostInfoManager *, QWidget *parent,
              const char *name = 0 );

    void update( const Job &job );
    QWidget *widget();

    QString id() const { return "host"; }

    void checkNode( unsigned int hostid );

    void removeNode( unsigned int hostid );

    void updateSchedulerState( bool online );

    void configureView();

  protected:
    virtual void resizeEvent( QResizeEvent *e );

    void updateJobLabels();

  protected slots:
    void slotConfigChanged();

  private:
    HostViewConfigDialog *mConfigDialog;

    QLabel *mHostNameLabel;

    KLed *mOwnLed;
    KLed *mOthersLed;

    QLabel *mLocalJobsLabel;
    QLabel *mRemoteJobsLabel;
    QLabel *mCompileJobsLabel;
    
    unsigned int mHostId;

    QValueList<unsigned int> mLocalJobs;
    QValueList<unsigned int> mRemoteJobs;
    QValueList<unsigned int> mCompileJobs;
};

#endif
// vim:ts=4:sw=4:noet

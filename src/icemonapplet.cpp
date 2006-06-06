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

#include "hostinfo.h"
#include "monitor.h"
#include "hostview.h"

#include <qlabel.h>
#include <qfont.h>
#include <qstringlist.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kurifilter.h>
#include <kdialog.h>
#include <krun.h>
#include <kmessagebox.h>

#include "icemonapplet.h"

extern "C"
{
  KPanelApplet* init(QWidget *parent, const QString& configFile)
  {
    return new IcemonApplet( configFile, KPanelApplet::Stretch, 0, parent,
                             "icemonapplet");
  }
}

IcemonApplet::IcemonApplet( const QString &configFile, Type type, int actions,
                            QWidget *parent, const char *name )
  : KPanelApplet( configFile, type, actions, parent, name )
{
  setBackgroundOrigin( AncestorOrigin );

  mHostInfoManager = new HostInfoManager;

  mMonitor = new Monitor( mHostInfoManager, this );

  QBoxLayout *topLayout = new QVBoxLayout( this );

  mHostView = new HostView( false, mHostInfoManager, this );
  topLayout->addWidget( mHostView );

  mMonitor->setCurrentView( mHostView, false );
}

IcemonApplet::~IcemonApplet()
{
  delete mHostInfoManager;
}

#include "icemonapplet.moc"

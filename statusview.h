/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef ICEMON_STATUSVIEW_H
#define ICEMON_STATUSVIEW_H

class HostInfoManager;
class Job;

class QColor;
class QString;
class QWidget;

class StatusView
{
  public:
    StatusView( HostInfoManager * );
    virtual ~StatusView();

    HostInfoManager *hostInfoManager() const { return mHostInfoManager; }

    virtual void update( const Job &job ) = 0;
    virtual QWidget *widget() = 0;
    virtual void checkNode( unsigned int hostid );
    virtual void removeNode( unsigned int hostid );
    virtual void updateSchedulerState( bool online );

    virtual void stop() {}
    virtual void start() {}
    virtual void checkNodes() {}
    virtual void configureView() {}

    virtual QString id() const = 0;

    unsigned int processor( const Job & );

    QString nameForHost( unsigned int hostid );
    QColor hostColor( unsigned int hostid );

    static QColor textColor( const QColor &backGroundColor );

  private:
    HostInfoManager *mHostInfoManager;
};

#endif
// vim:ts=4:sw=4:noet

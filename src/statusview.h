/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2014 Kevin Funk <kfunk@kde.org>

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
#ifndef ICEMON_STATUSVIEW_H
#define ICEMON_STATUSVIEW_H

#include "types.h"

#include <QObject>
#include <QPointer>

class Monitor;
class HostInfoManager;
class Job;

class QColor;
class QString;
class QWidget;

class StatusView : public QObject
{
    Q_OBJECT

public:
    enum Option {
        NoOptions = 0, ///< No option
        RememberJobsOption = 1 ///< Show old jobs in case this view gets reactivated
    };
    Q_DECLARE_FLAGS(Options, Option);

    StatusView(QObject* parent = nullptr);
    virtual ~StatusView();

    virtual Options options() const;

    Monitor* monitor() const;
    virtual void setMonitor(Monitor* monitor);

    /// Convenience function to access the Monitor's host info manager
    HostInfoManager *hostInfoManager() const;

    virtual QWidget *widget() const = 0;

    virtual bool isPausable() { return false; }
    virtual bool isConfigurable() { return false; }

    virtual void checkNodes() {}
    virtual void configureView() {}

    virtual void stop() {}
    virtual void start() {}
    void togglePause();

    virtual QString id() const = 0;

    unsigned int processor( const Job & );

    QString nameForHost( unsigned int hostid );
    QColor hostColor( unsigned int hostid );

protected Q_SLOTS:
    virtual void update(const Job &job);
    virtual void checkNode(HostId hostid);
    virtual void removeNode(HostId hostid);
    virtual void updateSchedulerState( bool online );


private:
    QPointer<Monitor> m_monitor;
    bool m_paused;
};

#endif
// vim:ts=4:sw=4:noet

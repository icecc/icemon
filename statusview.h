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

#include <qstring.h>
#include <qwidget.h>
#include <qmap.h>

class Job
{
  public:
    enum State { WaitingForCS, LocalOnly, Compiling, Finished, Failed, Idle };
    Job(unsigned int id = 0,
        const QString &client = QString::null,
        const QString &filename = QString::null,
        const QString &environment = QString::null,
        const QString &lang = QString::null)
    {
        m_id = id;
        m_fileName = filename;
        m_env = environment;
        m_lang = lang;
        m_state = WaitingForCS;
        m_client = client;
        real_msec = 0;
        user_msec = 0;
        sys_msec = 0;
        maxrss = 0;
        idrss = 0;
        majflt = 0;
        nswap = 0;
        exitcode = 0;
        in_compressed = in_uncompressed = out_compressed = out_uncompressed = 0;
    }

    bool operator==( const Job &rhs ) const { return m_id == rhs.m_id; }
    bool operator!=( const Job &rhs ) const { return m_id != rhs.m_id; }

    unsigned int jobId() const { return m_id; }
    QString fileName() const { return m_fileName; }
    QString client() const { return m_client; }
    QString server() const { return m_server; }
    State state() const { return m_state; }
    QString stateAsString() const;
    time_t stime() const { return m_stime; }

    void setServer( const QString & host ) {
        m_server = host;
    }
    void setStartTime( time_t t ) {
        m_stime = t;
    }
    void setState( State s ) {
        m_state = s;
    }

  private:
    unsigned int m_id;
    QString m_fileName;
    QString m_server;
    QString m_client;
    QString m_lang;
    QString m_env;
    State m_state;
    time_t m_stime;

  public:
    unsigned int real_msec;  /* real time it used */
    unsigned int user_msec;  /* user time used */
    unsigned int sys_msec;   /* system time used */
    unsigned int maxrss;     /* maximum resident set size (KB) */
    unsigned int idrss;      /* integral unshared data size (KB) */
    unsigned int majflt;     /* page faults */
    unsigned int nswap;      /* swaps */

    int exitcode;            /* exit code */

    unsigned int in_compressed;
    unsigned int in_uncompressed;
    unsigned int out_compressed;
    unsigned int out_uncompressed;
};

class IdleJob : public Job
{
  public:
    IdleJob() : Job() { setState( Job::Idle ); }
};

class JobList : public QMap<unsigned int, Job>
{
  public:
    JobList() { }
};


class StatusView
{
  public:
    virtual ~StatusView() {}
    virtual void update( const Job &job ) = 0;
    virtual QWidget *widget() = 0;
    virtual void checkNode( const QString &host, unsigned int max_kids );
    virtual void stop() {}
    virtual void start() {}
    virtual void checkNodes() {}

    virtual QString id() const = 0;

    QString nameForIp( const QString &ip );

    QColor hostColor( const QString &ip );

  private:
    QMap<QString,QString> mHostNameMap;
    QMap<QString,QColor> mHostColorMap;
};

#endif
// vim:ts=4:sw=4:noet

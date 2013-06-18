/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004,2006-2007 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2008 Urs Wolfer <uwolfer@kde.org>
    Copyright (c) 2011 Daniel Molkentin <daniel.molkentin@nokia.com>
    Copyright (c) 2012 Kevin Funk <kevin@kfunk.org>

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
#ifndef ICEMON_JOB_H
#define ICEMON_JOB_H

#include <QString>
#include <time.h>
#include <QMap>
#include <qdebug.h>

class Job
{
  public:
    enum State { WaitingForCS, LocalOnly, Compiling, Finished, Failed, Idle };
    explicit Job(unsigned int id = 0,
        unsigned int client = 0,
        const QString &filename = QString(),
        const QString &lang = QString())
    {
        m_id = id;
        m_fileName = filename;
        m_lang = lang;
        m_state = WaitingForCS;
        m_client = client;
        real_msec = 0;
        user_msec = 0;
        sys_msec = 0;
        pfaults = 0;
        exitcode = 0;
        m_server = 0;
        in_compressed = in_uncompressed = out_compressed = out_uncompressed = 0;
    }

    bool operator==( const Job &rhs ) const { return m_id == rhs.m_id; }
    bool operator!=( const Job &rhs ) const { return m_id != rhs.m_id; }
    int operator<( const Job &rhs ) const{ return m_id < rhs.m_id; }

    unsigned int jobId() const { return m_id; }
    QString fileName() const { return m_fileName; }
    unsigned int client() const { return m_client; }
    unsigned int server() const { return m_server; }
    State state() const { return m_state; }
    QString stateAsString() const;
    time_t stime() const { return m_stime; }

    void setServer( unsigned int hostid ) {
        m_server = hostid;
    }
    void setStartTime( time_t t ) {
        m_stime = t;
    }
    void setState( State ss ) {
        m_state = ss;
    }

  private:
    unsigned int m_id;
    QString m_fileName;
    unsigned int m_server;
    unsigned int m_client;
    QString m_lang;
    State m_state;
    time_t m_stime;

  public:
    unsigned int real_msec;  /* real time it used */
    unsigned int user_msec;  /* user time used */
    unsigned int sys_msec;   /* system time used */
    unsigned int pfaults;    /* page faults */

    int exitcode;            /* exit code */

    unsigned int in_compressed;
    unsigned int in_uncompressed;
    unsigned int out_compressed;
    unsigned int out_uncompressed;
};

QDebug operator<<(QDebug dbg, const Job& job);

class IdleJob : public Job
{
  public:
    IdleJob() : Job() { setState( Job::Idle ); }
};

class JobList : public QMap<unsigned int, Job>
{
  public:
    JobList() {}
};

#endif
// vim:ts=4:sw=4:noet

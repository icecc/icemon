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
                 const QString &lang = QString());

    bool operator==(const Job &rhs) const { return id == rhs.id; }
    bool operator!=(const Job &rhs) const { return id != rhs.id; }
    int operator<(const Job &rhs) const { return id < rhs.id; }

    QString stateAsString() const;
    bool isDone() const { return state == Finished || state == Failed; }
    bool isActive() const { return state == LocalOnly || state == Compiling; }

    unsigned int id;
    QString fileName;
    unsigned int server;
    unsigned int client;
    QString lang;
    State state;
    time_t startTime;

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

QDebug operator<<(QDebug dbg, const Job &job);

class IdleJob
    : public Job
{
public:
    IdleJob()
        : Job() { state = Job::Idle; }
};

typedef QMap<unsigned int, Job> JobList;

#endif
// vim:ts=4:sw=4:noet

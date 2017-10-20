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

#include "job.h"

#include <QObject>
#include <QApplication>

Job::Job(unsigned int id, unsigned int client, const QString &filename, const QString &lang)
    : id(id)
    , fileName(filename)
    , server(0)
    , client(client)
    , lang(lang)
    , state(WaitingForCS)
    , startTime{}
    , real_msec(0)
    , user_msec(0)
    , sys_msec(0)
    , pfaults(0)
    , exitcode(0)
    , in_compressed(0)
    , in_uncompressed(0)
    , out_compressed(0)
    , out_uncompressed(0)
{
}

QString Job::stateAsString() const
{
    switch (state) {
    case WaitingForCS:
        return QApplication::tr("Waiting");
        break;
    case Compiling:
        return QApplication::tr("Compiling");
        break;
    case Finished:
        return QApplication::tr("Finished");
        break;
    case Failed:
        return QApplication::tr("Failed");
        break;
    case Idle:
        return QApplication::tr("Idle");
        break;
    case LocalOnly:
        return QApplication::tr("Local Only");
        break;
    }
    return QString();
}

QDebug operator<<(QDebug dbg, const Job &job)
{
    return dbg.nospace() << "Job[id=" << job.id
           << ", client=" << job.client
           << ", server=" << job.server
           << ", fileName=" << job.fileName
           << ", state=" << job.stateAsString()
           << "]";
}

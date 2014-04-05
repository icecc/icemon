/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2007 Dirk Mueller <mueller@kde.org>

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

#include "icecreammonitor.h"

#include "hostinfo.h"
#include "statusview.h"

#include <config-icemon.h>

#include <icecc/comm.h>
#ifdef ICECC_HAVE_LOGGING_H
#include <icecc/logging.h>
#endif

#include <qdebug.h>

#include <qsocketnotifier.h>
#include <qtimer.h>

#include <list>
#include <iostream>

using namespace std;

IcecreamMonitor::IcecreamMonitor( HostInfoManager *manager, QObject *parent)
    : Monitor(manager, parent)
    , m_scheduler( 0 )
    , m_schedulerState( false )
    , m_discover( 0 )
    , m_fd_notify( 0 )
    , m_fd_type(QSocketNotifier::Exception)
{
    setupDebug();
    checkScheduler();
}

IcecreamMonitor::~IcecreamMonitor()
{
    delete m_scheduler;
    delete m_discover;
}

void IcecreamMonitor::checkScheduler(bool deleteit)
{
#if 0
    qDebug() << "checkScheduler " << deleteit << endl;
#endif
    if ( deleteit ) {
        m_rememberedJobs.clear();
        delete m_scheduler;
        m_scheduler = 0;
        delete m_fd_notify;
        m_fd_notify = 0;
        m_fd_type = QSocketNotifier::Exception;
        delete m_discover;
        m_discover = 0;
        setSchedulerState(false);
    } else if ( m_scheduler )
        return;
    QTimer::singleShot( 1000+(qrand()&1023), this, SLOT( slotCheckScheduler() ) ); // TODO: check if correct
}

void IcecreamMonitor::registerNotify(int fd, QSocketNotifier::Type type, const char* slot)
{
    if (m_fd_notify) {
        m_fd_notify->disconnect(this);
        m_fd_notify->deleteLater();
    }
    m_fd_notify = new QSocketNotifier(fd, type, this);
    m_fd_type = type;
    QObject::connect(m_fd_notify, SIGNAL(activated(int)), slot);
}

void IcecreamMonitor::slotCheckScheduler()
{
    if ( m_scheduler )
        return;

    list<string> names;

    if ( !m_current_netname.isEmpty() )
        names.push_front( m_current_netname.data() );
    else
        names.push_front("ICECREAM");

    if (!qgetenv("USE_SCHEDULER").isEmpty())
        names.push_front(""); // try $USE_SCHEDULER

    for ( list<string>::const_iterator it = names.begin(); it != names.end();
          ++it ) {

        m_current_netname = it->c_str();
        if (!m_discover
            || m_discover->timed_out()) {
            delete m_discover;
            m_discover = new DiscoverSched ( m_current_netname.data() );
        }

        m_scheduler = m_discover->try_get_scheduler ();

        if ( m_scheduler ) {
            hostInfoManager()->setSchedulerName( QString::fromLatin1(m_discover->schedulerName().data()) );
            hostInfoManager()->setNetworkName( QString::fromLatin1(m_discover->networkName().data()) );
            m_scheduler->setBulkTransfer();
            delete m_discover;
            m_discover = 0;
            registerNotify(m_scheduler->fd,
                    QSocketNotifier::Read, SLOT(msgReceived()));

            if ( !m_scheduler->send_msg ( MonLoginMsg() ) ) {
                checkScheduler(true);
                QTimer::singleShot(0, this, SLOT(slotCheckScheduler()));
            }
            else {
                setSchedulerState( true );
            }
            return;
        }

        if (m_fd_type != QSocketNotifier::Write
            && m_discover->connect_fd() >= 0) {
            registerNotify(m_discover->connect_fd(),
                    QSocketNotifier::Write, SLOT(slotCheckScheduler()));
            return;
        }
        else if (m_fd_type != QSocketNotifier::Read
                && m_discover->listen_fd() >= 0) {
                registerNotify(m_discover->listen_fd(),
                        QSocketNotifier::Read, SLOT(slotCheckScheduler()));
        }
        if (m_fd_type == QSocketNotifier::Read)
            QTimer::singleShot(1000+(qrand()&1023), this, SLOT(slotCheckScheduler()));

    }
    setSchedulerState( false );
}

void IcecreamMonitor::msgReceived()
{
    while (!m_scheduler->read_a_bit() || m_scheduler->has_msg())
        if (!handle_activity())
            break;
}

bool IcecreamMonitor::handle_activity()
{
    Msg *m = m_scheduler->get_msg ();
    if ( !m ) {
#if 0
        qDebug() << "lost connection to scheduler\n";
#endif
        checkScheduler( true );
        setSchedulerState( false );
        return false;
    }

    switch ( m->type ) {
    case M_MON_GET_CS:
        handle_getcs( m );
        break;
    case M_MON_JOB_BEGIN:
        handle_job_begin( m );
        break;
    case M_MON_JOB_DONE:
        handle_job_done( m );
        break;
    case M_END:
        std::cout << "END" << endl;
        checkScheduler( true );
        break;
    case M_MON_STATS:
        handle_stats( m );
        break;
    case M_MON_LOCAL_JOB_BEGIN:
        handle_local_begin( m );
        break;
    case M_JOB_LOCAL_DONE:
        handle_local_done( m );
        break;
    default:
        cout << "UNKNOWN" << endl;
        break;
    }
    delete m;
    return true;
}

void IcecreamMonitor::handle_getcs( Msg *_m )
{
    MonGetCSMsg *m = dynamic_cast<MonGetCSMsg*>( _m );
    if ( !m ) return;
    m_rememberedJobs[m->job_id] = Job( m->job_id, m->clientid,
                                       m->filename.c_str(),
                                       m->lang == CompileJob::Lang_C ? "C" :
                                       "C++" );
    m_view->update( m_rememberedJobs[m->job_id] );
}

void IcecreamMonitor::handle_local_begin( Msg *_m )
{
    MonLocalJobBeginMsg *m = dynamic_cast<MonLocalJobBeginMsg*>( _m );
    if ( !m ) return;

    m_rememberedJobs[m->job_id] = Job( m->job_id, m->hostid,
                                       m->file.c_str(),
                                       "C++" );
    m_rememberedJobs[m->job_id].setState( Job::LocalOnly );
    m_view->update( m_rememberedJobs[m->job_id] );
}

void IcecreamMonitor::handle_local_done( Msg *_m )
{
    JobLocalDoneMsg *m = dynamic_cast<JobLocalDoneMsg*>( _m );
    if ( !m ) return;

    JobList::iterator it = m_rememberedJobs.find( m->job_id );
    if ( it == m_rememberedJobs.end() ) {
        // we started in between
        return;
    }

    ( *it ).setState( Job::Finished );
    m_view->update( *it );

    if ( m_rememberedJobs.size() > 3000 ) { // now remove 1000
        uint count = 1000;

        while ( --count )
            m_rememberedJobs.erase( m_rememberedJobs.begin() );
    }
}

void IcecreamMonitor::handle_stats( Msg *_m )
{
    MonStatsMsg *m = dynamic_cast<MonStatsMsg*>( _m );
    if ( !m ) return;

    QStringList statmsg = QString( m->statmsg.c_str() ).split( '\n' );
    HostInfo::StatsMap stats;
    for ( QStringList::ConstIterator it = statmsg.constBegin(); it != statmsg.constEnd();
          ++it ) {
        QString key = *it;
        key = key.left( key.indexOf( ':' ) );
        QString value = *it;
        value = value.mid( value.indexOf( ':' ) + 1 );
        stats[key] = value;
    }

    HostInfo *hostInfo = hostInfoManager()->checkNode( m->hostid, stats );

    if ( hostInfo->isOffline() ) {
        m_view->removeNode( m->hostid );
    } else {
        m_view->checkNode( m->hostid );
    }
}

void IcecreamMonitor::handle_job_begin( Msg *_m )
{
    MonJobBeginMsg *m = dynamic_cast<MonJobBeginMsg*>( _m );
    if ( !m ) return;

    JobList::iterator it = m_rememberedJobs.find( m->job_id );
    if ( it == m_rememberedJobs.end() ) {
        // we started in between
        return;
    }

    ( *it ).setServer( m->hostid );
    ( *it ).setStartTime( m->stime );
    ( *it ).setState( Job::Compiling );

#if 0
    qDebug() << "BEGIN: " << (*it).fileName() << " (" << (*it).jobId()
             << ")" << endl;
#endif

    m_view->update( *it );
}

void IcecreamMonitor::handle_job_done( Msg *_m )
{
    MonJobDoneMsg *m = dynamic_cast<MonJobDoneMsg*>( _m );
    if ( !m ) return;

    JobList::iterator it = m_rememberedJobs.find( m->job_id );
    if ( it == m_rememberedJobs.end() ) {
        // we started in between
        return;
    }

    ( *it ).exitcode = m->exitcode;
    if ( m->exitcode ) {
        ( *it ).setState( Job::Failed );
    } else {
        ( *it ).setState( Job::Finished );
        ( *it ).real_msec = m->real_msec;
        ( *it ).user_msec = m->user_msec;
        ( *it ).sys_msec = m->sys_msec;   /* system time used */
        ( *it ).pfaults = m->pfaults;     /* page faults */

        ( *it ).in_compressed = m->in_compressed;
        ( *it ).in_uncompressed = m->in_uncompressed;
        ( *it ).out_compressed = m->out_compressed;
        ( *it ).out_uncompressed = m->out_uncompressed;
    }

#if 0
    qDebug() << "DONE: " << (*it).fileName() << " (" << (*it).jobId()
             << ")" << endl;
#endif

    m_view->update( *it );
}

void IcecreamMonitor::setCurrentView(StatusView *view)
{
    if (m_view == view)
        return;

    m_view = view;

    if (m_view) {
        m_view->updateSchedulerState( m_schedulerState );

        if (m_view->options().testFlag(StatusView::RememberJobsOption)) {
            JobList::ConstIterator it = m_rememberedJobs.constBegin();
            for ( ; it != m_rememberedJobs.constEnd(); ++it )
                m_view->update( *it );
        }
    }
}

void IcecreamMonitor::setCurrentNet( const QByteArray &netName )
{
    m_current_netname = netName;
}

void IcecreamMonitor::setSchedulerState( bool online )
{
    if (m_schedulerState == online)
        return;
    m_schedulerState = online;
    emit schedulerStateChanged( online );
    m_view->updateSchedulerState( online );
}

void IcecreamMonitor::setupDebug()
{
#ifdef ICECC_HAVE_LOGGING_H
    char *env = getenv("ICECC_DEBUG");
    int debug_level = Error;

    if (env) {
        if (!strcasecmp(env, "info"))  {
            debug_level |= Info | Warning;
        } else if (!strcasecmp(env, "warnings")) {
            debug_level |= Warning; // taking out warning
        } else { // any other value
            debug_level |= Info | Debug | Warning;
        }
    }

    std::string logfile;

    if (const char *logfileEnv = getenv("ICECC_LOGFILE")) {
        logfile = logfileEnv;
    }

    setup_debug(debug_level, logfile, "ICEMON");
#endif
}

#include "icecreammonitor.moc"

#include "monitor.h"

#include "statusview.h"

Monitor::Monitor(HostInfoManager *manager, QObject *parent)
    : QObject(parent)
    , m_hostInfoManager(manager)
    , m_schedulerState(false)
{
}

QByteArray Monitor::currentNetname() const
{
    return m_currentNetname;
}

void Monitor::setCurrentNetname(const QByteArray& netname)
{
    m_currentNetname = netname;
}

bool Monitor::schedulerState() const
{
    return m_schedulerState;
}

void Monitor::setSchedulerState( bool online )
{
    if (m_schedulerState == online)
        return;

    m_schedulerState = online;
    emit schedulerStateChanged( online );
}

QList<Job> Monitor::jobHistory() const
{
    return QList<Job>();
}

#include "monitor.moc"

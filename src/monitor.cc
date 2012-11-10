#include "monitor.h"

#include "statusview.h"

Monitor::Monitor(HostInfoManager *manager, QObject *parent)
    : QObject(parent)
    , m_hostInfoManager(manager)
{
}

#include "monitor.moc"

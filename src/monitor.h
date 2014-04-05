#ifndef ICEMON_MONITOR_H
#define ICEMON_MONITOR_H

#include "job.h"
#include "types.h"

#include <QObject>

class StatusView;
class HostInfoManager;
class Job;

/**
 * Abstract base class for monitoring a icecream-like scheduler
 */
class Monitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool schedulerState READ schedulerState WRITE setSchedulerState NOTIFY schedulerStateChanged)

public:

    explicit Monitor(HostInfoManager *manager, QObject* parent = 0);

    QByteArray currentNetname() const;
    void setCurrentNetname(const QByteArray &);

    void setSchedulerState(bool online);
    bool schedulerState() const;

    virtual QList<Job> jobHistory() const;

    HostInfoManager *hostInfoManager() const { return m_hostInfoManager; }

Q_SIGNALS:
    void schedulerStateChanged(bool);

    void jobUpdated(const Job& job);
    void nodeRemoved(HostId id);
    void nodeUpdated(HostId id);

private:
    HostInfoManager *m_hostInfoManager;
    QByteArray m_currentNetname;
    bool m_schedulerState;
};

#endif // ICEMON_MONITOR_H

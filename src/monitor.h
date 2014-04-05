#ifndef ICEMON_MONITOR_H
#define ICEMON_MONITOR_H

#include <QObject>

class StatusView;
class HostInfoManager;

/**
 * Abstract base class for monitoring a icecream-like scheduler
 */
class Monitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool schedulerState READ schedulerState WRITE setSchedulerState NOTIFY schedulerStateChanged)

public:
    explicit Monitor(HostInfoManager *manager, QObject* parent = 0);

    virtual void setCurrentView(StatusView *view) = 0;
    virtual void setCurrentNet(const QByteArray &) = 0;

    virtual void setSchedulerState(bool online) = 0;
    virtual bool schedulerState() const = 0;

    HostInfoManager *hostInfoManager() const { return m_hostInfoManager; }

Q_SIGNALS:
    void schedulerStateChanged(bool);

private:
    HostInfoManager *m_hostInfoManager;
};

#endif // ICEMON_MONITOR_H

#ifndef ICEMON_FAKEMONITOR_H
#define ICEMON_FAKEMONITOR_H

#include "monitor.h"

#include "job.h"

class HostInfoManager;
class StatusView;

class QTimer;

class FakeMonitor : public Monitor
{
    Q_OBJECT

public:
    explicit FakeMonitor(HostInfoManager* manager, QObject* parent = 0);

    virtual void setCurrentView(StatusView* view);
    virtual void setCurrentNet(const QByteArray& net) { Q_UNUSED(net); }

    virtual void setSchedulerState(bool online) { Q_UNUSED(online); }
    virtual bool schedulerState() const { return true; }

private Q_SLOTS:
    void update();

private:
    void init();

    StatusView* m_view;

    QList<Job> m_activeJobs;

    QTimer* m_updateTimer;
};

#endif // ICEMON_FAKEMONITOR_H

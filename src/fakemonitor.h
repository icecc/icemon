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

private Q_SLOTS:
    void update();

private:
    void init();

    QList<Job> m_activeJobs;

    QTimer* m_updateTimer;
};

#endif // ICEMON_FAKEMONITOR_H

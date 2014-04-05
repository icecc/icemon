#include "fakemonitor.h"

#include "statusview.h"
#include "job.h"
#include "hostinfo.h"

#include <QDebug>
#include <QStringList>
#include <QTimer>

// counter variable
static int JOB_ID = 0;

static const int MAX_JOB_COUNT = 10;

static const QVector<int> HOST_IDS(QVector<int>() << 1 << 2 << 3);

static const QStringList JOB_FILENAMES(QStringList()
    << QLatin1String("/tmp/filename.cc")
    << QLatin1String("/some/very/long/path/containing/filename.cc")
    << QLatin1String("/some/very/long/path/containing/averyverylongfilename.cc")
);

FakeMonitor::FakeMonitor(HostInfoManager* manager, QObject* parent)
    : Monitor(manager, parent)
    , m_view(0)
    , m_updateTimer(new QTimer(this))
{
    m_updateTimer->setInterval(1000);
    m_updateTimer->start();
    connect(m_updateTimer, SIGNAL(timeout()), SLOT(update()));

    init();
}

void FakeMonitor::init()
{
    {
    HostInfo info(1);
    info.setIp("1.0.0.1");
    info.setColor(Qt::red);
    info.setMaxJobs(5);
    info.setName("Host1");
    info.setOffline(false);
    info.setPlatform("Linux 3.6");
    info.setServerLoad(1.0);
    info.setServerSpeed(10);
    hostInfoManager()->checkNode(info);
    }

    {
    HostInfo info(2);
    info.setIp("1.0.0.2");
    info.setColor(Qt::blue);
    info.setMaxJobs(10);
    info.setName("Host2");
    info.setOffline(false);
    info.setPlatform("Linux 3.4");
    info.setServerLoad(1.0);
    info.setServerSpeed(10);
    hostInfoManager()->checkNode(info);
    }

    {
    HostInfo info(3);
    info.setIp("1.0.0.3");
    info.setColor(Qt::green);
    info.setMaxJobs(5);
    info.setName("Host3");
    info.setOffline(false);
    info.setPlatform("Windows 3.1");
    info.setServerLoad(1.0);
    info.setServerSpeed(10);
    hostInfoManager()->checkNode(info);
    }
}

void FakeMonitor::setCurrentView( StatusView *view, bool rememberJobs )
{
    Q_UNUSED(rememberJobs);

    m_view = view;
    m_view->updateSchedulerState(true);
}

void FakeMonitor::update()
{
    if (!m_view)
        return;

    // create job
    const int clientId = HOST_IDS[JOB_ID % HOST_IDS.size()];
    const QString fileName = JOB_FILENAMES[JOB_ID % JOB_FILENAMES.length()];
    Job job(JOB_ID++, clientId, fileName);
    time_t rawtime;
    time(&rawtime);
    job.setStartTime(rawtime);
    job.setState(Job::Compiling);
    const int serverId = HOST_IDS[(JOB_ID+1) % HOST_IDS.size()];
    job.setServer(serverId);
    m_view->update(job);
    m_activeJobs << job;

    // clean up old jobs
    if (m_activeJobs.size() > MAX_JOB_COUNT) {
        Job job = m_activeJobs.first();
        m_activeJobs.removeFirst();
        job.setState(Job::Finished);
        m_view->update(job);
    }

    Q_FOREACH(const HostInfo* info, hostInfoManager()->hostMap().values()) {
        if ( info->isOffline() ) {
            m_view->removeNode( info->id() );
        } else {
            m_view->checkNode( info->id() );
        }
    }
}

#include "fakemonitor.moc"

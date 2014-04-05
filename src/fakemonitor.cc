#include "fakemonitor.h"

#include "statusview.h"
#include "job.h"
#include "hostinfo.h"

#include <QDebug>
#include <QStringList>
#include <QTime>
#include <QTimer>

namespace {

// counter variable
int JOB_ID = 0;

const int MAX_JOB_COUNT = 10;
const int MAX_HOST_COUNT = 40;

const QStringList JOB_FILENAMES(QStringList()
    << QLatin1String("/tmp/filename.cc")
    << QLatin1String("/some/very/long/path/containing/filename.cc")
    << QLatin1String("/some/very/long/path/containing/averyverylongfilename.cc")
);

QColor randomColor()
{
    const int r = qrand() % 255;
    const int g = qrand() % 255;
    const int b = qrand() % 255;
    return QColor(r, g, b);
};

}

FakeMonitor::FakeMonitor(HostInfoManager* manager, QObject* parent)
    : Monitor(manager, parent)
    , m_updateTimer(new QTimer(this))
{
    m_updateTimer->setInterval(200);
    m_updateTimer->start();
    connect(m_updateTimer, SIGNAL(timeout()), SLOT(update()));

    setSchedulerState(true);

    for (HostId i = 0; i < MAX_HOST_COUNT; ++i) {
        createHostInfo(i+1);
    }

    qsrand(QTime::currentTime().msec());
}

void FakeMonitor::createHostInfo(HostId id)
{
    HostInfo info(id);
    info.setIp(QString("1.0.0.%1").arg(id));
    info.setColor(randomColor());
    info.setMaxJobs(5);
    info.setName(QString("Host%1").arg(id));
    info.setOffline(false);
    info.setPlatform("Linux 3.6");
    info.setServerLoad(1.0);
    info.setServerSpeed(10);
    hostInfoManager()->checkNode(info);
}

void FakeMonitor::update()
{
    // create job
    const int clientId = (JOB_ID % MAX_HOST_COUNT)+1;
    const QString fileName = JOB_FILENAMES[JOB_ID % JOB_FILENAMES.length()];
    Job job(JOB_ID++, clientId, fileName);
    time_t rawtime;
    time(&rawtime);
    job.setStartTime(rawtime);
    job.setState(Job::Compiling);
    const int serverId = ((JOB_ID+1) % MAX_HOST_COUNT)+1;
    job.setServer(serverId);
    emit jobUpdated(job);
    m_activeJobs << job;

    // clean up old jobs
    if (m_activeJobs.size() > MAX_JOB_COUNT) {
        Job job = m_activeJobs.first();
        m_activeJobs.removeFirst();
        job.setState(Job::Finished);
        emit jobUpdated(job);
    }

    Q_FOREACH(const HostInfo* info, hostInfoManager()->hostMap().values()) {
        if ( info->isOffline() ) {
            emit nodeRemoved(info->id());
        } else {
            emit nodeUpdated(info->id());
        }
    }
}

#include "fakemonitor.moc"

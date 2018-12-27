/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "starview.h"

#include "hostinfo.h"
#include "utils.h"
#include <monitor.h>

#include <qdebug.h>

#include <qlayout.h>
#include <qtooltip.h>
#include <qslider.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qregexp.h>
#include <qcheckbox.h>
#include <qdir.h>
#include <QSettings>
#include <QResizeEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDialogButtonBox>

#include <math.h>

// TODO: This shouldn't be global
static bool suppressDomain = false;

StarViewConfigDialog::StarViewConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Star View Settings"));

    QBoxLayout *topLayout = new QVBoxLayout(this);

    QLabel *label = new QLabel(tr("Number of nodes per ring:"));
    topLayout->addWidget(label);

    QBoxLayout *nodesLayout = new QHBoxLayout();
    topLayout->addLayout(nodesLayout);

    mNodesPerRingSlider = new QSlider(Qt::Horizontal);
    mNodesPerRingSlider->setMinimum(1);
    mNodesPerRingSlider->setMaximum(50);
    mNodesPerRingSlider->setSingleStep(1);
    nodesLayout->addWidget(mNodesPerRingSlider);
    connect(mNodesPerRingSlider, SIGNAL(valueChanged(int)),
            SIGNAL(configChanged()));
    connect(mNodesPerRingSlider, SIGNAL(valueChanged(int)),
            SLOT(slotNodesPerRingChanged(int)));

    mNodesPerRingLabel = new QLabel;
    nodesLayout->addWidget(mNodesPerRingLabel);

    label = new QLabel(tr("Architecture filter:"));
    topLayout->addWidget(label);
    mArchFilterEdit = new QLineEdit;
    topLayout->addWidget(mArchFilterEdit);
    connect(mArchFilterEdit, SIGNAL(textChanged(const QString &)),
            SIGNAL(configChanged()));

    mSuppressDomainName = new QCheckBox(tr("Suppress domain name"));
    topLayout->addWidget(mSuppressDomainName);
    auto hline = new QFrame;
    hline->setFrameShape(QFrame::HLine);
    hline->setFrameShadow(QFrame::Sunken);
    topLayout->addWidget(hline);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    topLayout->addWidget(buttonBox);

    connect(mSuppressDomainName, SIGNAL(toggled(bool)),
            SLOT(slotSuppressDomainName(bool)));

    connect(buttonBox, SIGNAL(rejected()), SLOT(accept()));
}

void StarViewConfigDialog::slotNodesPerRingChanged(int nodes)
{
    mNodesPerRingLabel->setText(QString::number(nodes));
}

void StarViewConfigDialog::setMaxNodes(int maxNodes)
{
    mNodesPerRingSlider->setMaximum(maxNodes + 1);
}

int StarViewConfigDialog::nodesPerRing()
{
    return mNodesPerRingSlider->value();
}

void StarViewConfigDialog::setNodesPerRing(int nodes)
{
    mNodesPerRingSlider->setValue(nodes);
    slotNodesPerRingChanged(nodes);
    configChanged();
}

bool StarViewConfigDialog::suppressDomainName() const
{
    return ::suppressDomain;
}

void StarViewConfigDialog::setSuppressDomainName(bool suppress)
{
    mSuppressDomainName->setChecked(suppress);
    configChanged();
}

QString StarViewConfigDialog::archFilter()
{
    return mArchFilterEdit->text();
}

void StarViewConfigDialog::slotSuppressDomainName(bool b)
{
    suppressDomain = b;
    configChanged();
}

HostItem::HostItem(const QString &text)
    : QGraphicsItemGroup(nullptr)
    , mHostInfo(nullptr)
    , mHostInfoManager(nullptr)
    , m_stateItem(nullptr)
{
    init();

    m_textItem->setPlainText(text);

    updateName();
}

HostItem::HostItem(HostInfo *hostInfo, HostInfoManager *m)
    : QGraphicsItemGroup(nullptr)
    , mHostInfo(hostInfo)
    , mHostInfoManager(m)
    , m_stateItem(nullptr)
{
    init();
}

HostItem::~HostItem()
{
}

void HostItem::init()
{
    mBaseWidth = 0;
    mBaseHeight = 0;

    m_boxItem = new QGraphicsEllipseItem(this);
    m_boxItem->setZValue(80);
    m_boxItem->setPen(QPen(Qt::NoPen));

    m_textItem = new QGraphicsTextItem(this);
    m_textItem->setZValue(100);

    setHostColor(QColor(200, 200, 200));

    mIsActiveClient = false;
    mIsCompiling = false;

    m_client = 0;
}

void HostItem::setHostColor(const QColor &color)
{
    m_boxItem->setBrush(color);
    m_boxItem->setPen(color.darker(PenDarkerFactor));
    m_textItem->setDefaultTextColor(Utils::textColor(color));
}

QString HostItem::hostName() const
{
    QString name = mHostInfo->name();
    if (suppressDomain) {
        return name.section(QLatin1Char('.'), 0, 0);
    }
    return mHostInfo->name();
}

void HostItem::updateName()
{
    // Autosize the textItem's width to determine the desired size...
    m_textItem->setTextWidth(-1);

    if (mHostInfo) {
        QPen pen = m_boxItem->pen();
        QString text;

        if (mHostInfo->noRemote() || mHostInfo->maxJobs() == 0) {
            text = QStringLiteral("<i>%1</i>").arg(hostName());
            pen.setStyle(Qt::DotLine);
        } else
        {
            text = QStringLiteral("%1<br/>[%2/%3]").arg(hostName()).arg(m_jobs.size()).arg(mHostInfo->maxJobs());
            pen.setStyle(Qt::SolidLine);
        }

        m_boxItem->setPen(pen);
        m_textItem->setHtml(text);
    } else
    {
        m_textItem->setHtml(m_fixedText);
    }

    QTextBlockFormat format;
    format.setAlignment(Qt::AlignCenter);
    QTextCursor cursor = m_textItem->textCursor();
    cursor.select(QTextCursor::Document);
    cursor.mergeBlockFormat(format);
    cursor.clearSelection();

    QRectF rect = m_textItem->boundingRect();

    // Set the textItem to fixed width based on previous autosize to apply the alignment.
    m_textItem->setTextWidth(rect.width());

    mBaseWidth = rect.width() * M_SQRT2;
    mBaseHeight = rect.height() * M_SQRT2;

    m_boxItem->setRect(-baseXMargin(), -baseYMargin(), mBaseWidth, mBaseHeight);

    updateHalos();
}

void HostItem::setFixedText(const QString &text)
{
    if (m_fixedText == text) {
        return;
    }

    m_fixedText = text;
    updateName();
}

void HostItem::setCenterPos(double x, double y)
{
    // move all items (also the sub items)
    setPos(x - m_textItem->boundingRect().width() / 2, y - m_textItem->boundingRect().height() / 2);
    //  setPos( x, y );
}

void HostItem::update(const Job &job)
{
    setIsCompiling(job.state == Job::Compiling);
    setClient(job.client);

    if (job.state == Job::WaitingForCS) {
        return;
    }

    bool finished = job.state == Job::Finished ||
                    job.state == Job::Failed;

    JobList::Iterator it = m_jobs.find(job.id);
    bool newJob = (it == m_jobs.end());

    if (newJob && finished) {
        return;
    }
    if (!newJob && !finished) {
        return;
    }

    if (newJob) {
        m_jobs.insert(job.id, job);
        createJobHalo(job);
        updateName();
    } else if (finished) {
        deleteJobHalo(job);
        m_jobs.erase(it);
        updateName();
    }
}

void HostItem::createJobHalo(const Job &job)
{
    auto halo = new QGraphicsEllipseItem(
        centerPosX(), centerPosY(), mBaseWidth, mBaseHeight,
        this);

    halo->setPen(QPen(Qt::NoPen));
    halo->show();

    m_jobHalos.insert(job, halo);

    updateHalos();
}

void HostItem::deleteJobHalo(const Job &job)
{
    QMap<Job, QGraphicsEllipseItem *>::Iterator it = m_jobHalos.find(job);
    if (it == m_jobHalos.end()) {
        return;
    }

    QGraphicsEllipseItem *halo = *it;
    delete halo;
    m_jobHalos.erase(it);

    updateHalos();
}

void HostItem::updateHalos()
{
    Q_ASSERT(m_jobHalos.isEmpty() || mHostInfoManager);

    int count = 1;

    QMap<Job, QGraphicsEllipseItem *>::Iterator it;
    for (it = m_jobHalos.begin(); it != m_jobHalos.end(); ++it) {
        QGraphicsEllipseItem *halo = it.value();
        halo->setZValue(70 - count);
        halo->setRect(halo->x() - baseXMargin() - count * HaloMargin, halo->y() - baseYMargin() - count * HaloMargin, mBaseWidth + count * HaloMargin * 2, mBaseHeight + count * HaloMargin * 2);
        halo->setBrush(mHostInfoManager->hostColor(it.key().client));
        halo->setPen(mHostInfoManager->hostColor(it.key().client).darker(PenDarkerFactor));
        ++count;
    }
}

StarView::StarView(QObject *parent)
    : StatusView(parent)
    , m_canvas(new QGraphicsScene)
    , m_widget(new StarViewGraphicsView(m_canvas, this))
{
    mConfigDialog = new StarViewConfigDialog(m_widget.data());
    connect(mConfigDialog, SIGNAL(configChanged()),
            SLOT(slotConfigChanged()));

    readSettings();

    m_widget->setScene(m_canvas);
    m_widget->arrangeItems();
}

StarView::~StarView()
{
    writeSettings();
}

void StarView::readSettings()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("view_%1").arg(id()));
    mConfigDialog->setNodesPerRing(settings.value(QStringLiteral("nodesPerRing"), 25).toInt());
    mConfigDialog->setSuppressDomainName(settings.value(QStringLiteral("suppressDomainName"), true).toBool());
    settings.endGroup();
}

void StarView::writeSettings()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("view_%1").arg(id()));
    settings.setValue(QStringLiteral("nodesPerRing"), mConfigDialog->nodesPerRing());
    settings.setValue(QStringLiteral("suppressDomainName"), mConfigDialog->suppressDomainName());
    settings.endGroup();
    settings.sync();
}

void StarView::update(const Job &job)
{
    if (job.state == Job::WaitingForCS) {
        m_widget->drawNodeStatus();
        return;
    }

    unsigned int hostid = processor(job);
    if (!hostid) {
        return;
    }

    HostItem *hostItem = findHostItem(hostid);
    if (!hostItem) {
        return;
    }

    hostItem->update(job);

    bool finished = job.state == Job::Finished || job.state == Job::Failed;

    QMap<unsigned int, HostItem *>::Iterator it;
    it = mJobMap.find(job.id);
    if (it != mJobMap.end()) {
        (*it)->update(job);
        if (finished) {
            mJobMap.erase(it);
            unsigned int clientid = job.client;
            HostItem *clientItem = findHostItem(clientid);
            if (clientItem) {
                clientItem->setIsActiveClient(false);
            }
        }
        m_widget->drawNodeStatus();
        return;
    }

    if (!finished) {
        mJobMap.insert(job.id, hostItem);
    }

    if (job.state == Job::Compiling) {
        unsigned int clientid = job.client;
        HostItem *clientItem = findHostItem(clientid);
        if (clientItem) {
            clientItem->setClient(clientid);
            clientItem->setIsActiveClient(true);
        }
    }

    m_widget->drawNodeStatus();
}

QList<HostItem *> StarView::hostItems() const
{
    return m_hostItems.values();
}

HostItem *StarView::findHostItem(unsigned int hostid) const
{
    return m_hostItems.value(hostid);
}

void StarView::setMonitor(Monitor *monitor)
{
    StatusView::setMonitor(monitor);

    if (monitor) {
        createKnownHosts();
        m_widget->arrangeItems();
    }
}

void StarView::checkNode(unsigned int hostid)
{
//  qDebug() << "StarView::checkNode() " << hostid << endl;

    if (!hostid) {
        return;
    }

    if (!filterArch(hostid)) {
        return;
    }

    HostItem *hostItem = findHostItem(hostid);
    if (!hostItem) {
        createHostItem(hostid);
        m_widget->arrangeItems();
    }
}

void StarView::removeNode(unsigned int hostid)
{
//  qDebug() << "StarView::removeNode() " << hostid << endl;

    HostItem *hostItem = findHostItem(hostid);

    if (hostItem && hostItem->hostInfo()->isOffline()) {
        removeItem(hostItem);
    }
}

void StarView::forceRemoveNode(unsigned int hostid)
{
    HostItem *hostItem = findHostItem(hostid);

    if (hostItem) {
        removeItem(hostItem);
    }
}

void StarView::removeItem(HostItem *hostItem)
{
    m_hostItems.remove(hostItem->hostInfo()->id());

    QList<unsigned int> obsoleteJobs;

    QMap<unsigned int, HostItem *>::Iterator it;
    for (it = mJobMap.begin(); it != mJobMap.end(); ++it) {
        if (it.value() == hostItem) {
            obsoleteJobs.append(it.key());
        }
    }

    QList<unsigned int>::ConstIterator it2;
    for (it2 = obsoleteJobs.constBegin(); it2 != obsoleteJobs.constEnd(); ++it2) {
        mJobMap.remove(*it2);
    }

    delete hostItem->stateItem();
    delete hostItem;

    m_widget->arrangeItems();
}

void StarView::updateSchedulerState(Monitor::SchedulerState state)
{
    if (state == Monitor::Offline) {
        QMap<unsigned int, HostItem *>::ConstIterator it;
        for (it = m_hostItems.constBegin(); it != m_hostItems.constEnd(); ++it) {
            delete *it;
        }

        m_hostItems.clear();
        mJobMap.clear();
    }

    m_widget->arrangeItems();
}

QWidget *StarView::widget() const
{
    return m_widget.data();
}

StarViewGraphicsView::StarViewGraphicsView(QGraphicsScene *scene, StarView *starView, QWidget *parent)
    : QGraphicsView(scene, parent)
    , m_starView(starView)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    scene->setSceneRect(0, 0, width(), height());

    // add scheduler item
    m_schedulerItem = new HostItem(QString());
    scene->addItem(m_schedulerItem);
    m_schedulerItem->setZValue(150);
    m_schedulerItem->show();
    arrangeSchedulerItem();
}

void StarViewGraphicsView::resizeEvent(QResizeEvent *)
{
    scene()->setSceneRect(0, 0, width(), height());

    arrangeSchedulerItem();
    arrangeHostItems();
    drawNodeStatus();
}

bool StarViewGraphicsView::event(QEvent *e)
{
    if (e->type() != QEvent::ToolTip) {
        return QGraphicsView::event(e);
    }

    QPoint p(static_cast<QHelpEvent *>(e)->pos());

    HostItem *item = nullptr;
    QGraphicsItem *graphicsItem = itemAt(p);
    if (graphicsItem) {
        item = dynamic_cast<HostItem *>(graphicsItem->parentItem());
    }
    if (item) {
        HostInfo *hostInfo = item->hostInfo();
        const QPoint gp(static_cast<QHelpEvent *>(e)->globalPos());
        const QRect itemRect = mapFromScene(graphicsItem->sceneBoundingRect()).boundingRect();
        if (hostInfo) {
            QToolTip::showText(gp + QPoint(10, 10), hostInfo->toolTip(), this, itemRect);
        } else {
            QToolTip::showText(gp + QPoint(10, 10), QStringLiteral(
                "<h3><b>%1</b></h3>"
                "<table>"
                "<tr><td>%2</td></tr>"
                "<tr><td>%3</td></tr>"
                "</table>"
                "<p><img align=\"right\" src=\":/images/icemonnode.png\"/></p>")
                     .arg(tr("Scheduler"),
                          tr("Host: %1").arg(m_starView->hostInfoManager()->schedulerName()),
                          tr("Network name: %1").arg(m_starView->hostInfoManager()->networkName())
                         ),
                this, itemRect);
        }
    } else {
        QToolTip::hideText();
    }
    return QGraphicsView::event(e);
}

void StarViewGraphicsView::arrangeSchedulerItem()
{
    const Monitor *monitor = m_starView->monitor();
    const bool isOnline = (monitor ? monitor->schedulerState() == Monitor::Online : false);
    m_schedulerItem->setFixedText(isOnline ? tr("Scheduler") : QStringLiteral("<b>No scheduler available</b>"));
    m_schedulerItem->setCenterPos(width() / 2, height() / 2);
}

void StarView::slotConfigChanged()
{
    if (!hostInfoManager()) {
        return;
    }

    HostInfoManager::HostMap hostMap = hostInfoManager()->hostMap();
    HostInfoManager::HostMap::ConstIterator it;
    for (it = hostMap.constBegin(); it != hostMap.constEnd(); ++it) {
        if (filterArch(*it)) {
            checkNode(it.key());
        } else {
            forceRemoveNode(it.key());
        }
    }

    m_widget->arrangeItems();
}

void StarViewGraphicsView::arrangeItems()
{
    arrangeHostItems();
    arrangeSchedulerItem();
}

void StarViewGraphicsView::arrangeHostItems()
{
//  qDebug() << "StarView::arrangeHostItems()" << endl;

    const QList<HostItem *> hostItems = m_starView->hostItems();
    int count = hostItems.count();

//  qDebug() << "  Count: " << count << endl;

    int nodesPerRing = m_starView->configDialog()->nodesPerRing();

    int ringCount = int( count / nodesPerRing ) + 1;

//  qDebug() << "  Rings: " << ringCount << endl;
    const double radiusFactor = 3.0;
    const int xRadius = qRound(scene()->width() / radiusFactor);
    const int yRadius = qRound(scene()->height() / radiusFactor);

    const double step = 2 * M_PI / count;

    double angle = 0.0;
    int i = 0;
    foreach(HostItem * item, hostItems) {
        double factor = 1 - (1.0 / (ringCount + 1)) * (i % ringCount);

        double xr = xRadius * factor;
        double yr = yRadius * factor;

        item->updateName();
        item->setCenterPos(width() / 2 + cos(angle) * xr,
                           height() / 2 + sin(angle) * yr);

        angle += step;
        ++i;
    }
}

HostItem *StarView::createHostItem(unsigned int hostid)
{
    HostInfo *i = hostInfoManager()->find(hostid);

    if (!i || i->isOffline() || i->name().isEmpty()) {
        return nullptr;
    }

//  qDebug() << "New node for " << hostid << " (" << i->name() << ")" << endl;

    //assert( !i->name().isEmpty() );

    auto hostItem = new HostItem(i, hostInfoManager());
    m_canvas->addItem(hostItem);
    hostItem->setHostColor(hostColor(hostid));
    m_hostItems.insert(hostid, hostItem);
    hostItem->show();

    if (m_hostItems.count() > 25) {
        mConfigDialog->setMaxNodes(m_hostItems.count());
    }

    return hostItem;
}

void StarViewGraphicsView::drawNodeStatus()
{
    const QList<HostItem *> hostItems = m_starView->hostItems();
    foreach(HostItem * item, hostItems) {
        drawState(item);
    }
}

void StarViewGraphicsView::drawState(HostItem *node)
{
    delete node->stateItem();
    QGraphicsLineItem *newItem = nullptr;

    unsigned int client = node->client();
    QColor color = client ? m_starView->hostColor(client) : Qt::green;

    if (node->isCompiling() || node->isActiveClient()) {
        newItem = new QGraphicsLineItem(qRound(node->centerPosX()),
                                        qRound(node->centerPosY()),
                                        qRound(m_schedulerItem->centerPosX()),
                                        qRound(m_schedulerItem->centerPosY()));
        if (node->isCompiling()) {
            newItem->setPen(QPen(color, 0));
            newItem->setZValue(-301);
        } else if (node->isActiveClient()) {
            newItem->setPen(QPen(color, 1, Qt::DashLine));
            newItem->setZValue(-300);
        }
        scene()->addItem(newItem);
    }

    node->setStateItem(newItem);
}

void StarView::createKnownHosts()
{
    HostInfoManager::HostMap hosts = hostInfoManager()->hostMap();

    HostInfoManager::HostMap::ConstIterator it;
    for (it = hosts.constBegin(); it != hosts.constEnd(); ++it) {
        unsigned int id = (*it)->id();
        if (!findHostItem(id)) {
            createHostItem(id);
        }
    }
}

void StarView::configureView()
{
    mConfigDialog->show();
    mConfigDialog->raise();
}

bool StarView::filterArch(unsigned int hostid)
{
    HostInfo *i = hostInfoManager()->find(hostid);
    if (!i) {
        return false;
    }

    return filterArch(i);
}

bool StarView::filterArch(HostInfo *i)
{
    if (mConfigDialog->archFilter().isEmpty()) {
        return true;
    }

    QRegExp regExp(mConfigDialog->archFilter());

    if (regExp.indexIn(i->platform()) >= 0) {
        return true;
    }

    return false;
}

StarViewConfigDialog *StarView::configDialog() const
{
    return mConfigDialog;
}

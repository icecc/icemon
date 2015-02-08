/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "ganttstatusview.h"

#include "job.h"
#include "hostinfo.h"
#include "utils.h"

#include <QDebug>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <QBoxLayout>
#include <QList>
#include <QGridLayout>
#include <QFrame>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QScrollBar>

GanttConfigDialog::GanttConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    QBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setMargin(10);
    topLayout->setSpacing(10);

    mTimeScaleVisibleCheck = new QCheckBox(tr("Show time scale"), this);
    topLayout->addWidget(mTimeScaleVisibleCheck);
    connect(mTimeScaleVisibleCheck, SIGNAL(clicked()),
            SIGNAL(configChanged()));

    QFrame *hline = new QFrame(this);
    hline->setFrameShape(QFrame::HLine);
    topLayout->addWidget(hline);

    QBoxLayout *buttonLayout = new QHBoxLayout();
    topLayout->addLayout(buttonLayout);

    buttonLayout->addStretch(1);

    QPushButton *button = new QPushButton(tr("&Close"), this);
    buttonLayout->addWidget(button);
    connect(button, SIGNAL(clicked()), SLOT(hide()));

    setWindowTitle(tr("Configure Gantt View"));
    // apply some minimum size
    resize(300, 100);
}

bool GanttConfigDialog::isTimeScaleVisible()
{
    return mTimeScaleVisibleCheck->isChecked();
}

GanttTimeScaleWidget::GanttTimeScaleWidget(QWidget *parent)
    : QWidget(parent)
    , mPixelsPerSecond(40)
{
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::white);
    setPalette(pal);
}

void GanttTimeScaleWidget::setPixelsPerSecond(int v)
{
    mPixelsPerSecond = v;
}

void GanttTimeScaleWidget::paintEvent(QPaintEvent *pe)
{
    const QRect r = pe->rect();

    QPainter p(this);
    const QFontMetrics fm = p.fontMetrics();

    // Maybe the rectangle overlaps the right half of a number, check whether
    // that is the case and repaint that number if necessary.
    if (r.x() % 100 != 0) {
        const int lastNumberXPos = r.x() - (r.x() % 100);
        QString lastNumber = QString::number(lastNumberXPos / 100 * 5);
        if (r.x() % 100 < p.fontMetrics().width(lastNumber)) {
            p.drawText(lastNumberXPos - r.x() + 2, fm.ascent(), lastNumber);
        }
    }

    // Now draw all the bars and numbers, very straightforward.
    for (int x = 0; x < r.width(); ++x) {
        const int absX = x + r.x();
        if (absX % (mPixelsPerSecond * 10) == 0) {
            p.drawLine(x, -r.y(), x, height() / 2 - r.y());
            p.drawText(x + 2, fm.ascent() - r.y(),
                       QString::number(absX /
                                       mPixelsPerSecond));
        } else if (absX % (mPixelsPerSecond * 5) == 0) {
            p.drawLine(x, -r.y(), x, height() / 4 - r.y());
            p.drawText(x + 2, fm.ascent() - r.y(),
                       QString::number(absX /
                                       mPixelsPerSecond));
        } else if (absX % (mPixelsPerSecond) == 0) {
            p.drawLine(x, -r.y(), x, height() / 8 - r.y());
        }
    }
}

GanttProgress::GanttProgress(StatusView *statusView, QWidget *parent)
    : QWidget(parent)
    , mStatusView(statusView)
    , mClock(0)
    , mIsFree(true)
{
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::white);
    setPalette(pal);
}

void GanttProgress::progress()
{
    mClock += 1;
    adjustGraph();
    QWidget::update();
}

void GanttProgress::adjustGraph()
{
    // Remove non-visible jobs
    if (m_jobs.count() >= 2 &&
        mClock - m_jobs[m_jobs.count() - 2].clock > width()) {
        m_jobs.removeAt(m_jobs.count() - 1);
    }
}

void GanttProgress::update(const Job &job)
{
    if (!m_jobs.isEmpty() && m_jobs.first().job == job) {
        if (job.state() == Job::Finished || job.state() == Job::Failed) {
            Job j = IdleJob();
            m_jobs.prepend(JobData(j, mClock));
            mIsFree = true;
        }
    } else {
        m_jobs.prepend(JobData(job, mClock));
        mIsFree = (job.state() == Job::Idle);
    }
}

void GanttProgress::drawGraph(QPainter &p)
{
    if (height() == 0) {
        return;
    }

    bool lastBox = false;
    int xStart = 0;
    QList<JobData>::ConstIterator it = m_jobs.constBegin();
    for (; (it != m_jobs.constEnd()) && !lastBox; ++it) {
        int xEnd = mClock - (*it).clock;

        if (xEnd > width()) {
            xEnd = width();
            lastBox = true;
        }

        int xWidth = xEnd - xStart;
        if (xWidth == 0) {
            continue;
        }

        // Draw the rectangle for the current job
        QColor color = colorForStatus((*it).job);
        p.fillRect(xStart, 0, xWidth, height(), color);
        p.setPen(color.dark());
        p.drawRect(xStart, 0, xWidth, height());

        if (xWidth > 4 && height() > 4) {
            int width = xWidth - 4;
            QString s = (*it).job.fileName();
            if (!s.isEmpty()) {
                s = s.mid(s.lastIndexOf('/') + 1, s.length());

                // Optimization - cache the drawn text in a pixmap, and update the cache
                // only if the pixmap height doesn't match, if the pixmap width is too large,
                // or if the shortened text with another character added (next_text_width) would fit
                if (width >= (*it).next_text_width || width < (*it).text_cache.width()
                    || height() - 4 != (*it).text_cache.height()) {
                    // If we print the filename, check whether we need to truncate it and
                    // append "..." at the end.
                    int text_width = p.fontMetrics().width(s);
                    if (text_width > width) {
                        int threeDotsWidth = p.fontMetrics().width("...");
                        int next_width = 0;
                        int newLength = 0;
                        for (;
                             next_width <= width;
                             ++newLength) {
                            text_width = next_width;
                            next_width = p.fontMetrics().width(s.left(newLength)) +
                                         threeDotsWidth;
                        }

                        (*it).next_text_width = next_width;
                        s  = s.left(newLength > 2 ? newLength - 2 : 0) + "...";
                    } else {
                        (*it).next_text_width = 1000000; // large number (no next width)
                    }
                    // Finally draw the text.
                    if (text_width > 0) {
                        (*it).text_cache = QPixmap(text_width, height() - 4);
                        (*it).text_cache.fill(color);
                        QPainter painter(&(*it).text_cache);
                        painter.setPen(Utils::textColor(color));
                        painter.drawText(0, 0, text_width, height() - 4,
                                         Qt::AlignVCenter | Qt::AlignLeft, s);
                    }
                }
                if (!(*it).text_cache.isNull()) {
                    p.drawPixmap(xStart + 2, 2, (*it).text_cache);
                }
            }
        }
        xStart = xEnd;
    }
}

QColor GanttProgress::colorForStatus(const Job &job) const
{
    if (job.state() == Job::Idle) {
        return Qt::gray;
    } else {
        QColor c = mStatusView->hostColor(job.client());
        if (job.state() == Job::LocalOnly) {
            return c.light();
        } else {
            return c;
        }
    }
}

void GanttProgress::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setBackgroundMode(Qt::OpaqueMode);
    p.setBackground(palette().background());
    drawGraph(p);
}

void GanttProgress::resizeEvent(QResizeEvent *)
{
    adjustGraph();
}

GanttStatusView::GanttStatusView(QObject *parent)
    : StatusView(parent)
    , m_widget(new QScrollArea)
    , mTopWidget(new QWidget)
{
    mConfigDialog = new GanttConfigDialog(m_widget.data());
    connect(mConfigDialog, SIGNAL(configChanged()),
            SLOT(slotConfigChanged()));

    m_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_widget->setWidget(mTopWidget);
    m_widget->setWidgetResizable(true);

    QPalette palette = mTopWidget->palette();
    palette.setColor(mTopWidget->backgroundRole(), Qt::white);
    mTopWidget->setPalette(palette);

    m_topLayout = new QGridLayout(mTopWidget);
    m_topLayout->setSpacing(5);
    m_topLayout->setMargin(4);
    m_topLayout->setColumnStretch(1, 10);

    mTimeScale = new GanttTimeScaleWidget(mTopWidget);
    mTimeScale->setFixedHeight(50);
    m_topLayout->addWidget(mTimeScale, 0, 1);

    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, SIGNAL(timeout()), SLOT(updateGraphs()));
    m_ageTimer = new QTimer(this);
    connect(m_ageTimer, SIGNAL(timeout()), SLOT(checkAge()));

    mUpdateInterval = 25;
    mTimeScale->setPixelsPerSecond(1000 / mUpdateInterval);

    mMinimumProgressHeight = QFontMetrics(m_widget->font()).height() + 6;

    slotConfigChanged();

    start();
}

void GanttStatusView::update(const Job &job)
{
    if (!mRunning) {
        return;
    }

    if (job.state() == Job::WaitingForCS) {
        return;
    }

    QMap<unsigned int, GanttProgress *>::Iterator it;

    it = mJobMap.find(job.jobId());

    if (it != mJobMap.end()) {
        it.value()->update(job);
        if (job.state() == Job::Finished || job.state() == Job::Failed) {
            mJobMap.erase(it);
        }
        return;
    }

    if (job.state() == Job::Finished || job.state() == Job::Failed) {
        return;
    }

    GanttProgress *slot = 0;

    unsigned int processor;
    if (job.state() == Job::LocalOnly) {
        processor = job.client();
    } else {
        processor = job.server();
    }

    if (!processor) {
        return;
    }

    NodeMap::ConstIterator it2 = mNodeMap.constFind(processor);
    if (it2 == mNodeMap.constEnd()) {
        slot = registerNode(processor);
    } else {
        SlotList slotList = it2.value();
        SlotList::ConstIterator it3;
        for (it3 = slotList.constBegin(); it3 != slotList.constEnd(); ++it3) {
            if ((*it3)->isFree()) {
                slot = *it3;
                break;
            }
        }

        if (it3 == slotList.constEnd()) {
            slot = registerNode(processor);
        }
    }

    Q_ASSERT(slot);
    mJobMap.insert(job.jobId(), slot);
    slot->update(job);
    mAgeMap[processor] = 0;
}

QWidget *GanttStatusView::widget() const
{
    return m_widget.data();
}

void GanttStatusView::checkNode(unsigned int hostid)
{
    if (!mRunning) {
        return;
    }

    if (mNodeMap.find(hostid) == mNodeMap.end()) {
        registerNode(hostid)->update(IdleJob());
    }
    unsigned int max_kids = hostInfoManager()->maxJobs(hostid);
    for (unsigned int i = mNodeMap[hostid].count();
         i < max_kids;
         ++i) {
        registerNode(hostid)->update(IdleJob());
    }

    mAgeMap[hostid] = 0;

    SlotList slotList = mNodeMap[hostid];   // make a copy
    int to_remove = slotList.count() - max_kids;
    if (to_remove <= 0) {
        return;
    }

    QListIterator<GanttProgress *> it2(slotList);
    it2.toBack();
    while (it2.hasPrevious()) {
        GanttProgress *progress = it2.previous();
        if (progress->isFree() && progress->fullyIdle()) {
            removeSlot(hostid, progress);
            if (--to_remove == 0) {
                return;
            }
        }
    }
}

GanttProgress *GanttStatusView::registerNode(unsigned int hostid)
{
//    qDebug() << "GanttStatusView::registerNode(): " << ip << endl;

    static int lastRow = 0;

    QColor color = hostColor(hostid);

    QVBoxLayout *nodeLayout;

    NodeLayoutMap::ConstIterator it = mNodeLayouts.constFind(hostid);
    if (it == mNodeLayouts.constEnd()) {
        ++lastRow;

        nodeLayout = new QVBoxLayout();
        nodeLayout->setObjectName((QString::number(hostid) + "_layout").toLatin1());
        m_topLayout->addLayout(nodeLayout, lastRow, 1);
        mNodeLayouts.insert(hostid, nodeLayout);
        mNodeRows.insert(hostid, lastRow);
    } else {
        nodeLayout = it.value();
    }

    NodeRowMap::ConstIterator rowIt = mNodeRows.constFind(hostid);
    if (rowIt == mNodeRows.constEnd()) {
    } else {
        int row = *rowIt;
        NodeLabelMap::ConstIterator labelIt = mNodeLabels.constFind(hostid);
        if (labelIt == mNodeLabels.constEnd()) {
            QString name = nameForHost(hostid);
            QLabel *l = new QLabel(name, mTopWidget);
            QPalette palette = l->palette();
            palette.setColor(l->foregroundRole(), color);
            palette.setColor(l->backgroundRole(), Qt::white);
            l->setPalette(palette);
            m_topLayout->addWidget(l, row, 0);

            QFont f = l->font();
            f.setBold(true);
            l->setFont(f);

            l->show();
            mNodeLabels.insert(hostid, l);
        }
    }

    GanttProgress *w = new GanttProgress(this, mTopWidget);
    w->setMinimumHeight(mMinimumProgressHeight);
    nodeLayout->addWidget(w);

    mNodeMap[hostid].append(w);
    mAgeMap[hostid] = 0;

    m_topLayout->setRowStretch(mNodeRows[hostid], mNodeMap[hostid].size());

    w->show();

    return w;
}

void GanttStatusView::removeSlot(unsigned int hostid, GanttProgress *slot)
{
    NodeLayoutMap::ConstIterator it = mNodeLayouts.constFind(hostid);
    if (it == mNodeLayouts.constEnd()) {
        return;
    }

    mNodeMap[hostid].removeAll(slot);
    JobMap newJobMap;
    for (QMap<unsigned int, GanttProgress *>::Iterator it = mJobMap.begin();
         it != mJobMap.end();  // QMap::remove doesn't return an iterator like
         ++it) {               // e.g. in QValueList, and I'm not sure if 'it'
        if ((*it) != slot) {   // or '++it' would be still valid, so let's copy
            newJobMap[it.key()] = *it;   // still valid items to a new map
        }
    }

    mJobMap = newJobMap;

    m_topLayout->setRowStretch(mNodeRows[hostid], mNodeMap[hostid].size());
    delete slot;
}

void GanttStatusView::unregisterNode(unsigned int hostid)
{
    NodeLayoutMap::ConstIterator it = mNodeLayouts.constFind(hostid);
    if (it == mNodeLayouts.constEnd()) {
        return;
    }
    while (!mNodeMap[hostid].isEmpty())
        removeSlot(hostid, mNodeMap[hostid].first());
    NodeLabelMap::Iterator labelIt = mNodeLabels.find(hostid);
    if (labelIt != mNodeLabels.end()) {
        delete *labelIt;
        mNodeLabels.erase(labelIt);
    }
    mAgeMap[hostid] = -1;
}

void GanttStatusView::updateGraphs()
{
    NodeMap::ConstIterator it;
    for (it = mNodeMap.constBegin(); it != mNodeMap.constEnd(); ++it) {
        SlotList::ConstIterator it2;
        for (it2 = (*it).constBegin(); it2 != (*it).constEnd(); ++it2) {
            (*it2)->progress();
        }
    }
}

void GanttStatusView::stop()
{
    mRunning = false;
    m_progressTimer->stop();
    m_ageTimer->stop();
}

void GanttStatusView::start()
{
    mRunning = true;
    m_progressTimer->start(mUpdateInterval);
    m_ageTimer->start(10000);
}

void GanttStatusView::checkAge()
{
    QList<unsigned int> to_unregister;
    for (AgeMap::Iterator it = mAgeMap.begin();
         it != mAgeMap.end();
         ++it) {
        if (*it > 1) {
            to_unregister.append(it.key());
        } else if (*it < 0) {
            ; // unregistered ones
        } else {
            ++(*it);
        }
    }

    for (QList<unsigned int>::ConstIterator it = to_unregister.constBegin();
         it != to_unregister.constEnd();
         ++it) {
        unregisterNode(*it);
    }
}

void GanttStatusView::configureView()
{
    mConfigDialog->show();
    mConfigDialog->raise();
}

void GanttStatusView::slotConfigChanged()
{
    if (mConfigDialog->isTimeScaleVisible()) {
        mTimeScale->show();
    } else {
        mTimeScale->hide();
    }
}

/*
    This file is part of Icecream.

    Copyright (c) 2011 Daniel Molkentin <molkentin@kde.org>

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

#include "flowtableview.h"

#include <QHeaderView>
#include <QIcon>
#include <QDebug>
#include <QPainter>
#include <QTimer>

ProgressWidget::ProgressWidget(HostInfo *info, StatusView *statusView, QWidget *parent) :
    QWidget(parent), m_hostInfo(info), m_statusView(statusView), m_isVirgin(true)
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    m_backingStore = QImage(size(), QImage::Format_RGB32);
    m_backingStore.fill(palette().base().color().rgb());
}

void ProgressWidget::paintEvent(QPaintEvent *)
{
    QImage temp(size(), QImage::Format_RGB32);
    QPainter p(&temp);

    p.drawImage(-1,0, m_backingStore);

    if (m_isVirgin) {
        p.fillRect(rect(), palette().base());
        m_isVirgin = false;
    }

    if (m_currentJob.state() == Job::Compiling ||
            m_currentJob.state() == Job::LocalOnly) {
        QLinearGradient gradient;
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        gradient.setColorAt(0, palette().base().color());
        gradient.setColorAt(1, m_statusView->hostColor(m_currentJob.client()));
        p.fillRect(width()-1,0, 1, height(), gradient);
    } else {
        p.fillRect(width()-1,0, 1, height(), palette().base().color());
    }

    QPainter screenp(this);
    screenp.drawImage(0, 0, temp);
    m_backingStore = temp;
}

void ProgressWidget::resizeEvent(QResizeEvent *) {
    m_isVirgin = true;
}

////////////////////////////////////////////////////////////////////////////////

FlowTableView::FlowTableView(QObject* parent)
    : StatusView(parent)
    , m_updateTimer(new QTimer)
    , m_widget(new QTableWidget)
{
    m_widget->setColumnCount(4);
    QStringList labels;
    labels << tr("Host") << tr("File") << tr("History") << tr("State");
    m_widget->setHorizontalHeaderLabels(labels);
    m_widget->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
    m_widget->verticalHeader()->hide();
    m_widget->setSelectionMode(QAbstractItemView::NoSelection);

    m_updateTimer->setInterval(50);
    m_updateTimer->start();
}

void FlowTableView::update(const Job &job)
{
    int serverId = job.server();
    if (serverId == 0)
        return;

    // checkNode hasn't been run for this server yet.
    if (!m_idToRowMap.contains(serverId))
        return;

    int serverRow = m_idToRowMap.value(serverId);
    QTableWidgetItem *fileNameItem = m_widget->item(serverRow, 1);
    QTableWidgetItem *jobStateItem = m_widget->item(serverRow, 3);

    if (job.state() == Job::Finished) {
        fileNameItem->setText("");
        jobStateItem->setText("");
    } else {
        QString filePath = job.fileName();
        QString fileName = filePath.mid(filePath.lastIndexOf('/')+1);
        fileNameItem->setText(fileName);
        fileNameItem->setToolTip(job.fileName());
        fileNameItem->setFlags(Qt::ItemIsEnabled);
        jobStateItem->setText(job.stateAsString());
        jobStateItem->setToolTip(job.stateAsString());
        jobStateItem->setFlags(Qt::ItemIsEnabled);
    }

    if (ProgressWidget *progressWidget = static_cast<ProgressWidget*>(m_widget->cellWidget(serverRow, 2))) {
        progressWidget->setCurrentJob(job);
    }

    // update the host column for the server requesting the job
    QTableWidgetItem *hostNameItem = m_widget->item(serverRow, 0);
    int usageCount = hostNameItem->data(Qt::UserRole).toInt();
    if (job.state() == Job::LocalOnly || job.state() == Job::Compiling)
        ++usageCount;
    else if (job.state() == Job::Finished || job.state() == Job::Failed)
        --usageCount;

    hostNameItem->setData(Qt::UserRole, usageCount);

    QFont f = m_widget->font();
    f.setBold(usageCount > 0);
    hostNameItem->setFont(f);
    hostNameItem->setText(hostInfoText(hostInfoManager()->find(serverId), usageCount));
}

QWidget* FlowTableView::widget() const
{
    return m_widget.data();
}

QString FlowTableView::hostInfoText(HostInfo *hostInfo, int runningProcesses) {
    if (hostInfo->serverSpeed() == 0) // host disabled
        return tr("%1 (Disabled)").arg(hostInfo->name());
    else
        return tr("%1 (%2/%3)").arg(hostInfo->name()).arg(runningProcesses).arg(hostInfo->maxJobs());
}

void FlowTableView::checkNode(unsigned int hostId)
{
    if (m_idToRowMap.contains(hostId))
        return;

    HostInfo *hostInfo = hostInfoManager()->hostMap().value(hostId);
    QTableWidgetItem *widgetItem = new QTableWidgetItem(hostInfoText(hostInfo));
    widgetItem->setIcon(QIcon(":/images/icemonnode.png"));
    widgetItem->setToolTip(hostInfo->toolTip());
    widgetItem->setBackgroundColor(hostInfo->color());
    // usage count
    widgetItem->setData(Qt::UserRole, 0);
    widgetItem->setFlags(Qt::ItemIsEnabled);
    int insertRow = m_widget->rowCount();
    m_widget->setRowCount(insertRow + 1);
    m_idToRowMap.insert(hostId, insertRow);
    m_widget->setItem(insertRow, 0, widgetItem);
    // adjust column width
    int width = QFontMetrics(widgetItem->font()).width(widgetItem->text())+32;
    m_widget->horizontalHeader()->resizeSection(0, qMax(m_widget->horizontalHeader()->sectionSize(0), width));

    widgetItem = new QTableWidgetItem;
    widgetItem->setFlags(Qt::ItemIsEnabled);
    m_widget->setItem(insertRow, 1, widgetItem);

    widgetItem = new QTableWidgetItem;
    widgetItem->setFlags(Qt::ItemIsEnabled);
    m_widget->setItem(insertRow, 3, widgetItem);

    ProgressWidget *pw = new ProgressWidget(hostInfo, this);
    connect(m_updateTimer, SIGNAL(timeout()), pw, SLOT(update()));
    m_widget->setCellWidget(insertRow, 2, pw);
}

void FlowTableView::removeNode(unsigned int hostId)
{
    m_widget->removeRow(m_idToRowMap.value(hostId));
    m_idToRowMap.remove(hostId);
}

#include "flowtableview.moc"

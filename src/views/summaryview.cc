/*
    This file is part of Icecream.

    Copyright (c) 2004 Scott Wheeler <wheeler@kde.org>

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

#include "summaryview.h"

#include "hostinfo.h"
#include "job.h"

#include <qdebug.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <QScrollBar>
#include <QApplication>

class NodeInfoFrame : public QFrame
{
public:
    NodeInfoFrame(QWidget *parent, const QColor &frameColor) :
        QFrame(parent), m_frameColor(frameColor)
    {
        setObjectName("nodeInfoFrame");
        setStyleSheet(QString("QFrame#nodeInfoFrame { border: 2px solid %1 }").arg(frameColor.name()));
    }
private:
    QColor m_frameColor;
};

class SummaryViewScrollArea : public QScrollArea
{
public:
    explicit SummaryViewScrollArea(QWidget* parent = 0);

    virtual void resizeEvent(QResizeEvent*);
};

////////////////////////////////////////////////////////////////////////////////
// SummaryViewItem implementation
////////////////////////////////////////////////////////////////////////////////

SummaryViewItem::SummaryViewItem(unsigned int hostid, QWidget *parent, SummaryView *view, QGridLayout *layout) :
    m_jobCount(0),
    m_view(view)
{
    const int row = layout->rowCount();
    const QColor nodeColor = view->hostInfoManager()->hostColor(hostid);

    NodeInfoFrame *labelBox = new NodeInfoFrame(parent, nodeColor);
    layout->setMargin(5);
    layout->addWidget(labelBox, row, 0);
    labelBox->show();
    labelBox->setMinimumWidth(75);
    m_widgets.append(labelBox);

    QVBoxLayout *labelLayout = new QVBoxLayout(labelBox);
    labelLayout->setMargin(10);
    labelLayout->setSpacing(5);

    QLabel *l;

    l = new QLabel(view->nameForHost(hostid), labelBox);
    l->setAlignment(Qt::AlignCenter);
    l->show();
    labelLayout->addWidget( l );

    const int maxJobs = view->hostInfoManager()->maxJobs(hostid);

    m_jobHandlers.resize(maxJobs);

    for(int i = 0; i < maxJobs; i++) {
        m_jobHandlers[i].stateWidget = new QFrame(labelBox);
        m_jobHandlers[i].stateWidget->setFrameStyle(QFrame::Box | QFrame::Plain);
        m_jobHandlers[i].stateWidget->setLineWidth(2);
        m_jobHandlers[i].stateWidget->setFixedHeight(15);
        QPalette palette = m_jobHandlers[i].stateWidget->palette();
        palette.setColor( m_jobHandlers[i].stateWidget->backgroundRole(), Qt::black );
        m_jobHandlers[i].stateWidget->setPalette( palette );
        m_jobHandlers[i].stateWidget->show();
        labelLayout->addWidget( m_jobHandlers[i].stateWidget );
    }

    NodeInfoFrame *detailsBox = new NodeInfoFrame(parent, nodeColor);
    layout->setMargin(5);
    layout->addWidget(detailsBox, row, 1);
    detailsBox->show();
    m_widgets.append(detailsBox);

    QGridLayout *grid = new QGridLayout(detailsBox);
    grid->setMargin(10);
    grid->setSpacing(5);

    m_jobsLabel = addLine(QApplication::tr("Jobs:"), detailsBox, grid, Qt::AlignBottom, "0");

    for(int i = 0; i < maxJobs; i++) {
        if(maxJobs > 1) {
            QSpacerItem *spacer  = new QSpacerItem(1, 8, QSizePolicy::Expanding);
            const int row = grid->rowCount();
            grid->addItem(spacer, row, 0, 1, grid->columnCount() - 1);
        }
        m_jobHandlers[i].sourceLabel = addLine(QApplication::tr("Source:"), detailsBox, grid);
        m_jobHandlers[i].stateLabel = addLine(QApplication::tr("State:"), detailsBox, grid);
    }

    grid->setColumnStretch(grid->columnCount() - 1, 1);
    grid->setRowStretch(0, 1);
    grid->setRowStretch(grid->rowCount(), 1);
}

SummaryViewItem::~SummaryViewItem()
{
  qDeleteAll( m_widgets );
  m_widgets.clear();
}

void SummaryViewItem::update(const Job &job)
{
    switch(job.state()) {
    case Job::Compiling:
    {
        m_jobCount++;
        m_jobsLabel->setText(QString::number(m_jobCount));

        QVector<JobHandler>::Iterator it = m_jobHandlers.begin();
        while(it != m_jobHandlers.end() && !(*it).currentFile.isNull())
            ++it;

        if(it != m_jobHandlers.end()) {
            const QColor nodeColor = m_view->hostInfoManager()->hostColor(job.client());
            QPalette palette = (*it).stateWidget->palette();
            palette.setColor( (*it).stateWidget->backgroundRole(), nodeColor );
            (*it).stateWidget->setPalette( palette );
            const QString fileName = job.fileName().section('/', -1);
            const QString hostName = m_view->nameForHost(job.client());
            (*it).sourceLabel->setText(QString("%1 (%2)").arg(fileName).arg(hostName));
            (*it).stateLabel->setText(job.stateAsString());
            (*it).currentFile = job.fileName();
        }
        break;
    }
    case Job::Finished:
    case Job::Failed:
    {
        QVector<JobHandler>::Iterator it = m_jobHandlers.begin();
        while(it != m_jobHandlers.end() && (*it).currentFile != job.fileName())
            ++it;

        if(it != m_jobHandlers.end()) {
            QPalette palette = (*it).stateWidget->palette();
            palette.setColor( (*it).stateWidget->backgroundRole(), Qt::black );
            (*it).stateWidget->setPalette( palette );
            (*it).sourceLabel->clear();
            (*it).stateLabel->setText(job.stateAsString());
            (*it).currentFile = QString();
        }
        break;
    }
    default:
        break;
    }
}

QLabel *SummaryViewItem::addLine(const QString &caption, QWidget *parent,
                                             QGridLayout *grid, Qt::Alignment flags,
                                             const QString &status)
{
    QLabel *label = new QLabel(caption, parent);
    label->setAlignment(Qt::AlignRight | flags);
    const int row = grid->rowCount();
    grid->addWidget(label, row, 0);
    QLabel *statusLabel = new QLabel(status, parent);
    //statusLabel->setAlignment(Qt::AlignLeft | flags);
    grid->addWidget(statusLabel, row, 1);
    label->show();
    statusLabel->show();

    return statusLabel;
}

SummaryViewScrollArea::SummaryViewScrollArea(QWidget* parent)
    : QScrollArea(parent)
{
}

void SummaryViewScrollArea::resizeEvent(QResizeEvent *e)
{
    Q_ASSERT(widget());

    QSize s = e->size();

    setMinimumWidth(widget()->minimumSizeHint().width() + verticalScrollBar()->width());
    widget()->setMinimumWidth(s.width());

    if(widget()->height() <= s.height())
        widget()->setMinimumHeight(s.height());
    else
        widget()->setMinimumHeight(widget()->sizeHint().height());

    QScrollArea::resizeEvent(e);
}

////////////////////////////////////////////////////////////////////////////////
// SummaryView implementation
////////////////////////////////////////////////////////////////////////////////

SummaryView::SummaryView(QObject* parent)
    : StatusView(parent)
    , m_widget(new SummaryViewScrollArea)
{
    m_base = new QWidget;
    m_base->setStyleSheet("QWidget { background-color: 'white'; }");
    m_widget->setWidget(m_base);

    m_layout = new QGridLayout(m_base);
    m_layout->setColumnStretch(1, 1);
    m_layout->setSpacing(5);
    m_layout->setMargin(5);

    m_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_widget->setMinimumHeight(150);
}

SummaryView::~SummaryView()
{

}

QWidget *SummaryView::widget() const
{
    return m_widget.data();
}

void SummaryView::update(const Job &job)
{
    if(!job.server())
        return;

    SummaryViewItem *i = m_items[job.server()];
    if(!i) {
        i = new SummaryViewItem(job.server(), m_base, this, m_layout);
        m_items.insert(job.server(), i);
    }
    i->update(job);
}

void SummaryView::checkNode(unsigned int hostid)
{
    HostInfo *hostInfo = hostInfoManager()->find(hostid);

    if(hostInfo && nameForHost(hostid).isNull()) {
        delete m_items[hostid];
        m_items.remove(hostid);
    }
    else if(!m_items[hostid]) {
        SummaryViewItem *i = new SummaryViewItem(hostid, m_base, this, m_layout);
        m_items.insert(hostid, i);
    }
}

/*
 * (C) 2004 Scott Wheeler <wheeler@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "summaryview.h"
#include "hostinfo.h"

#include <kiconloader.h>
#include <ksqueezedtextlabel.h>
#include <klocale.h>
#include <kdebug.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>

////////////////////////////////////////////////////////////////////////////////
// SummaryViewItem implementation
////////////////////////////////////////////////////////////////////////////////

SummaryViewItem::SummaryViewItem(unsigned int hostid, QWidget *parent, SummaryView *view, QGridLayout *layout) :
    m_jobCount(0),
    m_view(view)
{
    int row = layout->numRows();

    QVBox *labelBox = new QVBox(parent);
    labelBox->setFrameStyle(QFrame::Box | QFrame::Plain);
    labelBox->setLineWidth(3);
    labelBox->setMargin(5);
    labelBox->layout()->setSpacing(3);
    layout->addWidget(labelBox, row, 0);
    labelBox->show();
    labelBox->setMinimumWidth(75);

    QLabel *l;

    l = new QLabel(labelBox);
    l->setPixmap(UserIcon("icemonnode"));
    l->setAlignment(Qt::AlignCenter);
    l->show();

    l = new QLabel(view->nameForHost(hostid), labelBox);
    l->setAlignment(Qt::AlignCenter);
    l->show();

    const int maxJobs = view->hostInfoManager()->maxJobs(hostid);

    m_jobHandlers.resize(maxJobs);

    for(int i = 0; i < maxJobs; i++) {
        m_jobHandlers[i].stateWidget = new QFrame(labelBox);
        m_jobHandlers[i].stateWidget->setFrameStyle(QFrame::Box | QFrame::Plain);
        m_jobHandlers[i].stateWidget->setLineWidth(2);
        m_jobHandlers[i].stateWidget->setFixedHeight(15);
        m_jobHandlers[i].stateWidget->setPaletteBackgroundColor(QColor("black"));
        m_jobHandlers[i].stateWidget->show();
    }

    QFrame *detailsBox = new QFrame(parent);
    detailsBox->setFrameStyle(QFrame::Box | QFrame::Plain);
    detailsBox->setLineWidth(3);
    detailsBox->setMargin(5);
    layout->addWidget(detailsBox, row, 1);
    detailsBox->show();

    QGridLayout *grid = new QGridLayout(detailsBox);
    grid->setMargin(10);
    grid->setSpacing(5);

    m_jobsLabel   = addLine(i18n("Jobs:"), detailsBox, grid, Qt::AlignBottom, "0");

    for(int i = 0; i < maxJobs; i++) {
        m_jobHandlers[i].sourceLabel = addLine(i18n("Source:"), detailsBox, grid);
        m_jobHandlers[i].stateLabel = addLine(i18n("State:"), detailsBox, grid);
    }

    grid->setColStretch(grid->numCols() - 1, 1);
    grid->setRowStretch(0, 1);
    grid->setRowStretch(grid->numRows(), 1);
}

void SummaryViewItem::update(const Job &job)
{
    switch(job.state()) {
    case Job::Compiling:
    {
        m_jobCount++;
        m_jobsLabel->setText(QString::number(m_jobCount));

        QValueVector<JobHandler>::Iterator it = m_jobHandlers.begin();
        while(it != m_jobHandlers.end() && !(*it).currentFile.isNull())
            ++it;

        if(it != m_jobHandlers.end()) {
            (*it).stateWidget->setPaletteBackgroundColor(QColor("green"));
            const QString fileName = job.fileName().section('/', -1);
            const QString hostName = m_view->nameForHost(job.client());
            (*it).sourceLabel->setText(QString("%1 (%2)").arg(fileName).arg(hostName));
            (*it).stateLabel->setText(job.stateAsString());
            (*it).currentFile = job.fileName();
        }
        break;
    }
    case Job::Finished:
    {
        QValueVector<JobHandler>::Iterator it = m_jobHandlers.begin();
        while(it != m_jobHandlers.end() && (*it).currentFile != job.fileName())
            ++it;

        if(it != m_jobHandlers.end()) {
            (*it).stateWidget->setPaletteBackgroundColor(QColor("black"));
            (*it).sourceLabel->clear();
            (*it).stateLabel->setText(job.stateAsString());
            (*it).currentFile = QString::null;
        }
        break;
    }
    default:
        break;
    }
}

KSqueezedTextLabel *SummaryViewItem::addLine(const QString &caption, QWidget *parent,
                                             QGridLayout *grid, int flags,
                                             const QString &status)
{
    QLabel *label = new QLabel(caption, parent);
    label->setAlignment(Qt::AlignRight | flags);
    const int row = grid->numRows();
    grid->addWidget(label, row, 0);
    KSqueezedTextLabel *statusLabel = new KSqueezedTextLabel(status, parent);
    statusLabel->setAlignment(Qt::AlignAuto | flags);
    grid->addWidget(statusLabel, row, 1);
    label->show();
    statusLabel->show();

    return statusLabel;
}

////////////////////////////////////////////////////////////////////////////////
// SummaryView implementation
////////////////////////////////////////////////////////////////////////////////

SummaryView::SummaryView(HostInfoManager *h, QWidget *parent, const char *name) :
    QScrollView(parent, name), StatusView(h)
{
    enableClipper(true);
    m_base = new QWidget(viewport());
    addChild(m_base);

    m_layout = new QGridLayout(m_base);
    m_layout->setColStretch(1, 1);
    m_layout->setSpacing(5);
    m_layout->setMargin(5);

    setHScrollBarMode(AlwaysOff);
    setMinimumHeight(150);
}

SummaryView::~SummaryView()
{

}

QWidget *SummaryView::widget()
{
    return this;
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
    if(!m_items[hostid]) {
        SummaryViewItem *i = new SummaryViewItem(hostid, m_base, this, m_layout);
        m_items.insert(hostid, i);
    }
}

void SummaryView::viewportResizeEvent(QResizeEvent *e)
{
    QSize s = e->size();

    setMinimumWidth(m_base->sizeHint().width() + verticalScrollBar()->width());
    m_base->setMinimumWidth(s.width());

    if(m_base->height() <= s.height())
        m_base->setMinimumHeight(s.height());
    else
        m_base->setMinimumHeight(m_base->sizeHint().height());
        
    QScrollView::viewportResizeEvent(e);
}

#include "summaryview.moc"

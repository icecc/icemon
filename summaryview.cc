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
#include <qpainter.h>

class NodeInfoFrame : public QFrame
{
public:
    NodeInfoFrame(QWidget *parent, const QColor &frameColor) :
        QFrame(parent), m_frameColor(frameColor) {}
protected:
    virtual void drawFrame(QPainter *p)
    {
        static const int border = 2;

        QPen oldPen = p->pen();
        QPen newPen = oldPen;

        newPen.setWidth(5);

        p->setPen(newPen);
        p->drawRect(border, border, width() - border * 2, height() - border * 2);

        newPen.setWidth(1);
        newPen.setColor(m_frameColor);

        p->setPen(newPen);
        p->drawRect(border, border, width() - border * 2, height() - border * 2);

        p->setPen(oldPen);       
    }
private:
    QColor m_frameColor;
};

////////////////////////////////////////////////////////////////////////////////
// SummaryViewItem implementation
////////////////////////////////////////////////////////////////////////////////

SummaryViewItem::SummaryViewItem(unsigned int hostid, QWidget *parent, SummaryView *view, QGridLayout *layout) :
    m_jobCount(0),
    m_view(view)
{
    const int row = layout->numRows();
    const QColor nodeColor = view->hostInfoManager()->hostColor(hostid);

    NodeInfoFrame *labelBox = new NodeInfoFrame(parent, nodeColor);
    labelBox->setMargin(5);
    layout->addWidget(labelBox, row, 0);
    labelBox->show();
    labelBox->setMinimumWidth(75);
    m_widgets.append(labelBox);

    QVBoxLayout *labelLayout = new QVBoxLayout(labelBox);
    labelLayout->setAutoAdd(true);
    labelLayout->setMargin(10);
    labelLayout->setSpacing(5);

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

    NodeInfoFrame *detailsBox = new NodeInfoFrame(parent, nodeColor);
    detailsBox->setMargin(5);
    layout->addWidget(detailsBox, row, 1);
    detailsBox->show();
    m_widgets.append(detailsBox);

    QGridLayout *grid = new QGridLayout(detailsBox);
    grid->setMargin(10);
    grid->setSpacing(5);

    m_jobsLabel = addLine(i18n("Jobs:"), detailsBox, grid, Qt::AlignBottom, "0");

    for(int i = 0; i < maxJobs; i++) {
        if(maxJobs > 1) {
            QSpacerItem *spacer  = new QSpacerItem(1, 8, QSizePolicy::Expanding);
            const int row = grid->numRows();
            grid->addMultiCell(spacer, row, row, 0, grid->numCols() - 1);
        }
        m_jobHandlers[i].sourceLabel = addLine(i18n("Source:"), detailsBox, grid);
        m_jobHandlers[i].stateLabel = addLine(i18n("State:"), detailsBox, grid);
    }

    grid->setColStretch(grid->numCols() - 1, 1);
    grid->setRowStretch(0, 1);
    grid->setRowStretch(grid->numRows(), 1);
}

SummaryViewItem::~SummaryViewItem()
{
    for(QValueList<QWidget *>::ConstIterator it = m_widgets.begin();
        it != m_widgets.end();
        ++it)
    {
        delete *it;
    }
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
            const QColor nodeColor = m_view->hostInfoManager()->hostColor(job.client());
            (*it).stateWidget->setPaletteBackgroundColor(nodeColor);
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

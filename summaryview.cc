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

SummaryViewItem::SummaryViewItem(const QString &name, QWidget *parent, SummaryView *view, QGridLayout *layout) :
    m_jobCount(0),
    m_view(view)
{
    int row = layout->numRows();

    QVBox *labelBox = new QVBox(parent);
    labelBox->setFrameStyle(QFrame::Box | QFrame::Plain);
    labelBox->setLineWidth(3);
    labelBox->setMargin(5);
    layout->addWidget(labelBox, row, 0);
    labelBox->show();
    // view->addChild(labelBox);

    QLabel *l;

    l = new QLabel(labelBox);
    l->setPixmap(UserIcon("icemonnode"));
    l->show();

    l = new QLabel(name, labelBox);
    l->setAlignment(Qt::AlignCenter);
    l->show();

    m_stateWidget = new QFrame(labelBox);
    m_stateWidget->setFrameStyle(QFrame::Box | QFrame::Plain);
    m_stateWidget->setLineWidth(2);
    m_stateWidget->setFixedHeight(20);
    m_stateWidget->setPaletteBackgroundColor(QColor("black"));
    m_stateWidget->show();

    QFrame *detailsBox = new QFrame(parent);
    detailsBox->setFrameStyle(QFrame::Box | QFrame::Plain);
    detailsBox->setLineWidth(3);
    detailsBox->setMargin(5);
    layout->addWidget(detailsBox, row, 1);
    detailsBox->show();
    // view->addChild(detailsBox);

    QGridLayout *grid = new QGridLayout(detailsBox);
    grid->setMargin(10);
    grid->setSpacing(5);

    m_jobsLabel   = addLine(i18n("Jobs:"), detailsBox, grid, Qt::AlignBottom, "0");
    m_stateLabel  = addLine(i18n("State:"), detailsBox, grid);
    m_fileLabel   = addLine(i18n("File:"), detailsBox, grid);
    m_sourceLabel = addLine(i18n("Source:"), detailsBox, grid);

    grid->setColStretch(grid->numCols() - 1, 1);
    grid->setRowStretch(0, 1);
    grid->setRowStretch(grid->numRows() - 1, 1);
}

void SummaryViewItem::update(const Job &job)
{
    switch(job.state()) {
    case Job::Compiling:
        m_jobCount++;
        m_stateWidget->setPaletteBackgroundColor(QColor("green"));
        m_jobsLabel->setText(QString::number(m_jobCount));
        m_fileLabel->setText(job.fileName().section('/', -1));
        m_sourceLabel->setText(m_view->nameForHost(job.client()));
        break;
    case Job::Failed:
        m_stateWidget->setPaletteBackgroundColor(QColor("red"));
        m_fileLabel->clear();
        m_sourceLabel->clear();
	break;
    default:
        m_stateWidget->setPaletteBackgroundColor(QColor("black"));
        m_fileLabel->clear();
        m_sourceLabel->clear();
        break;
    }

    m_stateLabel->setText(job.stateAsString());
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

SummaryView::SummaryView(HostInfoManager *m, QWidget *parent, const char *name)
  : QScrollView(parent, name), StatusView(m)
{
    enableClipper(true);
    m_base = new QWidget(viewport());
    addChild(m_base);

    m_layout = new QGridLayout(m_base);
    m_layout->setColStretch(1, 1);
    m_layout->setSpacing(5);
    m_layout->setMargin(5);

    setHScrollBarMode(AlwaysOff);
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
        i = new SummaryViewItem(nameForHost(job.server()), m_base, this, m_layout);
        m_items.insert(job.server(), i);
    }
    i->update(job);
}

void SummaryView::checkNode(unsigned int hostid)
{
    if(!m_items[hostid]) {
        SummaryViewItem *i = new SummaryViewItem(nameForHost(hostid), m_base, this, m_layout);
        m_items.insert(hostid, i);
    }
}

void SummaryView::viewportResizeEvent(QResizeEvent *e)
{
    QSize s = e->size();

    setMinimumWidth(m_base->sizeHint().width() + verticalScrollBar()->width());
    m_base->setMinimumWidth(s.width());

    if(m_base->height() < s.height())
        m_base->setMinimumHeight(s.height());
    else
        m_base->setMinimumHeight(m_base->sizeHint().height());
        
    QScrollView::viewportResizeEvent(e);
}

#include "summaryview.moc"

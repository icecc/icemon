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

SummaryViewItem::SummaryViewItem(const QString &name, SummaryView *parent, QGridLayout *layout) :
    m_jobCount(0),
    m_parent(parent)
{
    int row = layout->numRows();

    QVBox *labelBox = new QVBox(parent);
    labelBox->setFrameStyle(QFrame::Box | QFrame::Plain);
    labelBox->setLineWidth(3);
    labelBox->setMargin(5);
    layout->addWidget(labelBox, row, 0);
    labelBox->show();

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
        m_sourceLabel->setText(m_parent->nameForHost(job.client()));
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
  : QWidget(parent, name), StatusView(m)
{
    m_layout = new QGridLayout(this);
    m_layout->setColStretch(1, 1);
    m_layout->setSpacing(5);
    m_layout->setMargin(5);
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
    if( !job.server() )
        return;

    SummaryViewItem *i = m_items[job.server()];
    if(!i) {
        i = new SummaryViewItem(nameForHost(job.server()), this, m_layout);
        m_items.insert(job.server(), i);
    }
    i->update(job);
}

void SummaryView::checkNode(unsigned int hostid)
{
    if(!m_items[hostid]) {
        SummaryViewItem *i = new SummaryViewItem(nameForHost(hostid), this, m_layout);
        m_items.insert(hostid, i);
    }
}

#include "summaryview.moc"

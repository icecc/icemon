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

SummaryViewItem::SummaryViewItem(const QString &name, QWidget *parent) :
    QHBox(parent, name.latin1()),
    m_jobCount(0)
{
    setSpacing(10);

    QVBox *labelBox = new QVBox(this);
    labelBox->setFrameStyle(Box | Plain);
    labelBox->setLineWidth(3);
    labelBox->setMargin(5);

    QLabel *l;

    l = new QLabel(labelBox);
    l->setPixmap(UserIcon("icemonnode"));

    l = new QLabel(name, labelBox);
    l->setAlignment(AlignCenter);

    m_stateWidget = new QFrame(labelBox);
    m_stateWidget->setFrameStyle(Box | Plain);
    m_stateWidget->setLineWidth(2);
    m_stateWidget->setFixedHeight(20);
    m_stateWidget->setPaletteBackgroundColor(QColor("black"));

    QFrame *detailsBox = new QFrame(this);
    detailsBox->setFrameStyle(Box | Plain);
    detailsBox->setLineWidth(3);
    detailsBox->setMargin(5);

    setStretchFactor(detailsBox, 1);

    QGridLayout *grid = new QGridLayout(detailsBox);
    grid->setMargin(10);
    grid->setSpacing(5);

    l = new QLabel(i18n("Jobs:"), detailsBox);
    l->setAlignment(AlignRight | AlignBottom);
    grid->addWidget(l, 0, 0);
    m_jobsLabel = new QLabel("0", detailsBox);
    m_jobsLabel->setAlignment(AlignAuto | AlignBottom);
    grid->addWidget(m_jobsLabel, 0, 1);

    l = new QLabel(i18n("State:"), detailsBox);
    l->setAlignment(AlignRight | AlignTop);
    grid->addWidget(l, 1, 0);
    m_stateLabel = new QLabel(detailsBox);
    m_stateLabel->setAlignment(AlignAuto | AlignTop);
    grid->addWidget(m_stateLabel, 1, 1);

    l = new QLabel(i18n("File:"), detailsBox);
    l->setAlignment(AlignRight | AlignTop);
    grid->addWidget(l, 2, 0);
    m_fileLabel = new KSqueezedTextLabel(detailsBox);
    m_fileLabel->setAlignment(AlignAuto | AlignTop);
    grid->addWidget(m_fileLabel, 2, 1);

    l = new QLabel(i18n("Source:"), detailsBox);
    l->setAlignment(AlignRight | AlignTop);
    grid->addWidget(l, 3, 0);
    m_sourceLabel = new QLabel(detailsBox);
    m_sourceLabel->setAlignment(AlignAuto | AlignTop);
    grid->addWidget(m_sourceLabel, 3, 1);

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
        m_fileLabel->setText(job.fileName());
        m_sourceLabel->setText(job.client());
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

////////////////////////////////////////////////////////////////////////////////
// SummaryView implementation
////////////////////////////////////////////////////////////////////////////////

SummaryView::SummaryView(QWidget *parent, const char *name) : QWidget(parent, name)
{
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setAutoAdd(true);
    l->setMargin(10);
    l->setSpacing(10);
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
    if(job.server().isNull())
        return;

    SummaryViewItem *i = m_items[job.server()];
    if(!i) {
        i = new SummaryViewItem(job.server(), this);
        i->show();
        m_items.insert(job.server(), i);
    }
    i->update(job);
}

void SummaryView::checkNode(unsigned int hostid, const QString &statmsg )
{
    StatusView::checkNode( hostid, statmsg );

    if(!m_items[host]) {
        SummaryViewItem *i = new SummaryViewItem(host, this);
        i->show();
        m_items.insert(host, i);
    }
}

#include "summaryview.moc"

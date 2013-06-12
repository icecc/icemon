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

#ifndef SUMMARYVIEW_H
#define SUMMARYVIEW_H

#include "statusview.h"

#include <QScrollArea>
#include <QResizeEvent>

class QLabel;
class QGridLayout;

class SummaryView;

class SummaryViewItem
{
public:
    SummaryViewItem(unsigned int hostid, QWidget *parent, SummaryView *view, QGridLayout *layout);
    ~SummaryViewItem();
    void update(const Job &job);

private:
    QLabel *addLine(const QString &caption, QWidget *parent, QGridLayout *grid,
                                Qt::Alignment flags = Qt::AlignTop,
                                const QString &status = QString());

    struct JobHandler
    {
        JobHandler() : stateWidget(0), sourceLabel(0), stateLabel(0) {}

        QFrame *stateWidget;
        QLabel *sourceLabel;
        QLabel *stateLabel;
        QString currentFile;
    };

    QFrame *m_stateWidget;
    QLabel *m_jobsLabel;
    QLabel *m_sourceLabel;

    int m_jobCount;
    SummaryView *m_view;

    QVector<JobHandler> m_jobHandlers;
    QList<QWidget *> m_widgets;
};

class SummaryView : public QScrollArea, public StatusView
{
    Q_OBJECT

public:
    SummaryView(HostInfoManager *h, QWidget *parent);
    ~SummaryView();

    virtual QWidget *widget();
    virtual void update(const Job &job);
    virtual void checkNode(unsigned int hostid);
    virtual QString id() const { return "summary"; }

protected:
    virtual void resizeEvent(QResizeEvent *e);

private:
    QMap<unsigned int, SummaryViewItem *> m_items;
    QGridLayout *m_layout;
    QWidget *m_base;
};

#endif

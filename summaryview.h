/*
 * (C) 2004 Scott Wheeler <wheeler@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef SUMMARYVIEW_H
#define SUMMARYVIEW_H

#include "mon-kde.h"

#include <qscrollview.h>
#include <qvaluevector.h>

class KSqueezedTextLabel;
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
    KSqueezedTextLabel *addLine(const QString &caption, QWidget *parent, QGridLayout *grid,
                                int flags = Qt::AlignTop,
                                const QString &status = QString::null);

    struct JobHandler
    {
        JobHandler() : stateWidget(0), sourceLabel(0), stateLabel(0) {}

        QFrame *stateWidget;
        KSqueezedTextLabel *sourceLabel;
        KSqueezedTextLabel *stateLabel;
        QString currentFile;
    };

    QFrame *m_stateWidget;
    KSqueezedTextLabel *m_jobsLabel;
    KSqueezedTextLabel *m_sourceLabel;

    int m_jobCount;
    SummaryView *m_view;

    QValueVector<JobHandler> m_jobHandlers;
    QValueList<QWidget *> m_widgets;
};

class SummaryView : public QScrollView, public StatusView
{
    Q_OBJECT

public:
    SummaryView(HostInfoManager *h, QWidget *parent, const char *name = 0);
    ~SummaryView();

    virtual QWidget *widget();
    virtual void update(const Job &job);
    virtual void checkNode(unsigned int hostid);
    virtual QString id() const { return "summary"; }

protected:
    virtual void viewportResizeEvent(QResizeEvent *e);

private:
    QMap<unsigned int, SummaryViewItem *> m_items;
    QGridLayout *m_layout;
    QWidget *m_base;
};

#endif

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

#include <qhbox.h>

class KSqueezedTextLabel;
class QLabel;
class QGridLayout;

class SummaryView;

class SummaryViewItem
{
public:
    SummaryViewItem(const QString &name, SummaryView *parent, QGridLayout *layout);
    void update(const Job &job);

private:
    KSqueezedTextLabel *addLine(const QString &caption, QWidget *parent, QGridLayout *grid,
                                int flags = Qt::AlignBottom,
                                const QString &status = QString::null);

    QFrame *m_stateWidget;
    KSqueezedTextLabel *m_stateLabel;
    KSqueezedTextLabel *m_jobsLabel;
    KSqueezedTextLabel *m_fileLabel;
    KSqueezedTextLabel *m_sourceLabel;

    int m_jobCount;
    SummaryView *m_parent;
};

class SummaryView : public QWidget, public StatusView
{
    Q_OBJECT

public:
    SummaryView(HostInfoManager *, QWidget *parent, const char *name = 0);
    ~SummaryView();

    virtual QWidget *widget();
    virtual void update(const Job &job);
    virtual void checkNode(unsigned int hostid);

    QString id() const { return "summary"; }

private:
    QMap<unsigned int, SummaryViewItem *> m_items;
    QGridLayout *m_layout;
};

#endif

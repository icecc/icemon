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

class SummaryViewItem : public QHBox
{
    Q_OBJECT

public:
    SummaryViewItem(const QString &name, QWidget *parent);
    void update(const Job &job);

private:
    QFrame *m_stateWidget;
    QLabel *m_stateLabel;
    QLabel *m_jobsLabel;
    KSqueezedTextLabel *m_fileLabel;
    QLabel *m_sourceLabel;

    int m_jobCount;
};

class SummaryView : public QWidget, public StatusView
{
    Q_OBJECT

public:
    SummaryView(QWidget *parent, const char *name = 0);
    ~SummaryView();

    virtual QWidget *widget();
    virtual void update(const Job &job);
    virtual void checkNode(const QString &host, unsigned int max_kids);

private:
    QDict<SummaryViewItem> m_items;
};

#endif

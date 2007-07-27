/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Stephan Kulow <coolo@kde.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef MON_KDE_H
#define MON_KDE_H

#include <kxmlguiwindow.h>

class HostInfoManager;
class Monitor;
class StatusView;
class KSelectAction;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
  public:
    MainWindow( QWidget *parent, const char *name = 0 );
    ~MainWindow();

    void setCurrentNet( const QByteArray & );

  private slots:
    void setupListView();
    void setupStarView();
    void setupPoolView();
    void setupSummaryView();
    void setupGanttView();
    void setupDetailedHostView();

    void stopView();
    void startView();
    void checkNodes();
    void configureView();

  private:
    void readSettings();
    void writeSettings();

    void setupView( StatusView *view, bool rememberJobs );

    HostInfoManager *m_hostInfoManager;
    Monitor *m_monitor;
    StatusView *m_view;

    enum views {
      ListViewType,
      StarViewType,
      PoolViewType,
      GanttViewType,
      SummaryViewType,
      DetailedHostViewType
    };

    KSelectAction* m_viewMode;
};

#endif // MON_KDE_H
// vim:ts=4:sw=4:et

/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef ICEMON_STARVIEW_H
#define ICEMON_STARVIEW_H

#include "mon-kde.h"

#include <time.h>
#include <qdict.h>

class QCanvas;
class QCanvasText;
class QCanvasView;
class HostItem;

class StarView : public QWidget, public StatusView
{
    Q_OBJECT
  public:
    StarView( QWidget *parent, const char *name = 0 );

    void update( const Job &job );
    QWidget *widget();

    QString id() const { return "star"; }

    unsigned int processor( const Job & );

    HostItem *StarView::findHostItem( unsigned int hostid );

    void checkNode( unsigned int hostid, const StatsMap & );

  protected:
    virtual void resizeEvent( QResizeEvent *e );

  private:
    void centerLocalhostItem();
    void arrangeHostItems();
    HostItem *createHostItem( unsigned int hostid );
    void drawNodeStatus();
    void drawState( HostItem *node );

    QCanvas *m_canvas;
    QCanvasView *m_canvasView;
    QCanvasText *m_localhostItem;
    QMap<unsigned int,HostItem*> m_hostItems;
    QMap<unsigned int,HostItem *> mJobMap;
};

#endif
// vim:ts=4:sw=4:noet

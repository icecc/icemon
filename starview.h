/*
    This file is part of Icecream.

    Copyright (c) 2003 Frerich Raabe <raabe@kde.org>

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

class StarStatusView : public StatusView
{
    Q_OBJECT
  public:
    StarStatusView( QWidget *parent, const char *name = 0 );

    QString id() const { return "star"; }

  public slots:
    virtual void update( const JobList &jobs );

  protected:
    virtual void resizeEvent( QResizeEvent *e );

  private:
    void centerLocalhostItem();
    void arrangeNodeItems();
    void checkForNewNodes( const JobList &jobs );
    void updateNodeStatus( const JobList &jobs );
    void drawNodeStatus();
    void drawState( NodeItem *node );

    QCanvas *m_canvas;
    QCanvasView *m_canvasView;
    QCanvasText *m_localhostItem;
    QDict<NodeItem> m_nodeItems;
};

#endif
// vim:ts=4:sw=4:noet

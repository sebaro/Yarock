/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2015 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
*                                                                                       *
*  This program is free software; you can redistribute it and/or modify it under        *
*  the terms of the GNU General Public License as published by the Free Software        *
*  Foundation; either version 2 of the License, or (at your option) any later           *
*  version.                                                                             *
*                                                                                       *
*  This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
*  PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
*                                                                                       *
*  You should have received a copy of the GNU General Public License along with         *
*  this program.  If not, see <http://www.gnu.org/licenses/>.                           *
*****************************************************************************************/
#ifndef _GRAPHICS_ITEM_MENU_H_
#define _GRAPHICS_ITEM_MENU_H_

#include "local_item.h"
#include "views.h"

#include <QMenu>
#include <QWidget>
#include <QMap>
#include <QAction>


enum ENUM_ACTION_ITEM_MENU {
      ALBUM_PLAY          = 1,
      ALBUM_QUEUE_END,

      GENRE_PLAY,
      GENRE_QUEUE_END,

      UPDATE_FAVORITE, 
      EDIT,
      
      ARTIST_PLAY,
      ARTIST_QUEUE_END,

      PLAYLIST_PLAY,
      PLAYLIST_QUEUE_END,
      PLAYLIST_REMOVE,

      TRACK_PLAY,
      TRACK_QUEUE_END,

      STREAM_PLAY,
      STREAM_EDIT,
      STREAM_QUEUE_END,
      STREAM_FAVORITE,

      SELECTION_PLAY,
      SELECTION_QUEUE_END, 
      SELECTION_TRACK_EDIT
};

/*
********************************************************************************
*                                                                              *
*    Class GraphicsItemMenu                                                    *
*                                                                              *
********************************************************************************
*/
class GraphicsItemMenu : public QMenu
{
  Q_OBJECT
public:
    GraphicsItemMenu(QWidget* parent = 0);
    void setBrowserView(QWidget* w) {m_browserview = w;}
    
    void appendItem(QGraphicsItem* item);
    void appendItems(QList<QGraphicsItem*> items);

    void updateMenu(bool isSelection);
    void setMode(VIEW::Id m) {m_view_mode = m;}

    bool eventFilter(QObject *obj, QEvent *ev);
    
private slots:
    void slot_actionsTriggered();
    void slot_updateScene();

private:
    QWidget                 *m_browserview;
    QGraphicsView           *m_view;
    QGraphicsScene          *m_scene;
    
    QList<QGraphicsItem*>    m_items;
    bool                     is_selection;
    VIEW::Id                 m_view_mode;
    
    QMap<ENUM_ACTION_ITEM_MENU, QAction*>  m_map_actions;

signals:
    void menu_action_triggered(ENUM_ACTION_ITEM_MENU);
};

#endif // _GRAPHICS_ITEM_MENU_H_

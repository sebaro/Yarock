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

#ifndef _STREAM_SCENE_H_
#define _STREAM_SCENE_H_

#include "scene_base.h"

#include "models/stream/service_base.h"
#include "views/item_menu.h"
#include "views.h"


#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QtCore>


class VirtualPlayqueue;
class StreamModel;
class EngineBase;
class ExLineEdit;

/*
********************************************************************************
*                                                                              *
*    Class StreamScene                                                         *
*                                                                              *
********************************************************************************
*/
class StreamScene : public SceneBase
{
Q_OBJECT
  public:
    StreamScene(QWidget *parent);
    int   elementCount() const {return m_infoSize;}

    //! playing item scene method (for Mainwindow access)
    void setSearch(const QVariant&);
    void setData(const QVariant&);
    void playSceneContents(const QVariant&) {};
    
    void playSelected();

    QList<QAction *> actions();
    
  /* Basic Scene virtual */      
  public slots:
    void populateScene();
    void resizeScene();
    void initScene();
        
  protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );    
    void keyPressEvent ( QKeyEvent * keyEvent );

  private slots :
    void slot_contextmenu_triggered(ENUM_ACTION_ITEM_MENU);
    void slot_item_mouseMove();

    void slot_streams_fetched(MEDIA::TrackPtr);
    void slot_reload_stream_list();
    void slot_on_add_stream_clicked();
    void slot_on_import_stream_clicked();
    void slot_on_search_activated();
    
    void slot_on_service_state_changed();
    
  private:
    /* favorite */
    void updateStreamFavorite();
    void editStream();
    
    /* selection */
    QList<QGraphicsItem*> sortedSelectedItem();

    /* population scene method */
    void populateExtendedStreamScene();
    void populateWaitingScene(QString text);

    /* playing item scene method */
    void playStream();

    /* playqueue method */
    void enqueueStream(bool is_new_playqueue);
    void enqueueSelected(bool is_new_playqueue);
    void startStreamsDrag(QGraphicsItem* i=0);
    
  signals:
    void linked_changed(QVariant);

  private :
    int                      item_count;
    
    /*  stream services */
    QMap<VIEW::Id,Service*>   m_services;
      
    StreamModel              *m_model;

    EngineBase               *_player;
    int                       m_infoSize;

    QGraphicsItem            *m_mouseGrabbedItem;
    
    /* seazrch widget */
    QGraphicsProxyWidget     *ui_proxy;
    ExLineEdit               *ui_ex_line_edit;

    QMap<QString,QAction*>    m_actions;

    GraphicsItemMenu         *m_graphic_item_menu;
    
    /* messages Id for StatusWidget management */
    QMap<QString, uint>       m_messageIds;
};

#endif // _STREAM_SCENE_H_

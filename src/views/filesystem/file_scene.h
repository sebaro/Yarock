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

#ifndef _FILE_SCENE_H_
#define _FILE_SCENE_H_

#include "views.h"
#include "scene_base.h"
#include "mediaitem.h"

#include <QGraphicsItem>
#include <QWidget>


class FileModel;
/*
********************************************************************************
*                                                                              *
*    Class FileScene                                                           *
*                                                                              *
********************************************************************************
*/
class FileScene : public SceneBase
{
Q_OBJECT
  public:
    FileScene(QWidget *parent);

    void setPath(const QString& );
    void setSearch(const QVariant&);
    void setData(const QVariant& data);
    void playSceneContents(const QVariant&) {};

  /* Basic Scene virtual */      
  public slots:
    void populateScene();
    void resizeScene();
    void initScene();

  signals:
    void load_directory(const QString&);
    
  private slots:
    void slot_on_directory_loaded();
    void slot_item_mouseMove();  
    void slot_on_go_up();
    
  protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );
    
private:
    void async_load_item();
    void startDirectoriesDrag(QGraphicsItem*);
    void startTracksDrag(QGraphicsItem*);
    void startPlaylistsDrag(QGraphicsItem*);
      
  private :
    QMap<QString, QGraphicsItem*> map_graphic_items;
    QList<MEDIA::TrackPtr>   m_tracks;

    QString                  m_filter;

    int                      m_infosize;
    
    int                      item_count;
      
    FileModel                *m_model;

    QGraphicsItem            *m_mouseGrabbedItem;

    QPixmap                   m_folder_pixmap;
    QString                   m_current_path;
    
    bool                      loading_directory;
};

#endif // _FILE_SCENE_H_

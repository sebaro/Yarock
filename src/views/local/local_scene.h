/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2014 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

#ifndef _LOCAL_SCENE_H_
#define _LOCAL_SCENE_H_

//! local
#include "views/scene_base.h"
#include "views/local/local_item.h"
#include "views/item_menu.h"
#include "views.h"

// Qt
#include <QStringList>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QMap>


class LocalTrackModel;
class LocalPlaylistModel;
class HistoModel;

/*
********************************************************************************
*                                                                              *
*    Class LocalScene                                                          *
*                                                                              *
********************************************************************************
*/
class LocalScene : public SceneBase
{
Q_OBJECT
  public:
    LocalScene(QWidget *parent);

    int   elementCount() const {return m_infosize;}

    QPoint get_item_position(MEDIA::MediaPtr media);

    void playSelected();
    void setSearch(const QVariant&);
    void setData(const QVariant&) {};

    QList<QAction *> actions();
    
  /* Basic Scene virtual */      
  public slots:
    void populateScene();
    void resizeScene();
    void initScene();
    
  private :
    int                      item_count;
  
    LocalTrackModel          *m_localTrackModel;
    LocalPlaylistModel       *m_localPlaylistModel;
    HistoModel               *m_histoModel;

    int                       m_infosize;

    QGraphicsItem            *m_mouseGrabbedItem;
    GraphicsItemMenu         *m_graphic_item_menu;

    QMap<QString,QAction*>    m_actions;
        
  protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );
    void keyPressEvent ( QKeyEvent * keyEvent );
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );

  private slots :
    void slot_item_mouseMove();
    void slot_item_ratingclick();
    void slot_contextmenu_triggered(ENUM_ACTION_ITEM_MENU);

    void slot_change_view_settings();

    void slot_clear_history();

  private :
    //! selection
    QList<QGraphicsItem*> sortedSelectedItem();

    //! population scene method
    void populateLocalSceneBuilding();
    void populateAlbumScene();
    void populateAlbumGridScene();
    void populateAlbumExtendedScene();
    void populateArtistScene();
    void populateTrackScene();
    void populateGenreScene();
    void populateYearScene();
    void populateFavoriteScene();
    void populatePlaylistScene();
    void populatePlaylistOverviewScene();
    void populatePlaylistByTrackScene();
    void populatePlaylistSmartScene();
    void populateHistoScene();
    void populateDashBoardScene();
    void populateMostPlayedAlbum(int YPos);
    void populateTopRatedAlbum(int YPos);
    void populateMostPlayedArtist(int YPos);
    void populateTopRatedArtist(int YPos);
    
    //! playing item scene method
    void playAlbum();
    void playAlbumGenre();
    void playArtist();
    void playTrack();
    void playPlaylist();

    //! playqueue method
    void enqueueAlbum(bool is_new_playqueue);
    void enqueueAlbumGenre(bool is_new_playqueue);
    void enqueueArtist(bool is_new_playqueue);
    void enqueuePlaylist(bool is_new_playqueue);
    void enqueueTrack(bool is_new_playqueue);
    void enqueueSelected(bool is_new_playqueue);

    //! drag multiple items
    void startAlbumsDrag(QGraphicsItem* i=0);
    void startAlbumsGenreDrag(QGraphicsItem* i=0);
    void startArtistsDrag(QGraphicsItem* i=0);
    void startTracksDrag(QGraphicsItem* i=0);
    void startPlaylistsDrag(QGraphicsItem* i=0);

    //! edition dialog
    void edit_media_dialog();
    void edit_media_dialog_selection();
    void updateMediaFavorite();
    void removePlaylistFromDisk();
};

#endif // _LOCAL_SCENE_H_

/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2016 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

#include "local_scene.h"

#include "views/item_button.h"
#include "views/item_common.h"
#include "views/local/local_item.h"

#include "models/local/local_track_model.h"
#include "models/local/local_playlist_model.h"
#include "models/local/histo_model.h"
#include "playqueue/playqueue_model.h"
#include "playqueue/virtual_playqueue.h"

#include "core/mediaitem/mediaitem.h"
#include "covers/covercache.h"

#include "core/database/database.h"
#include "core/database/database_cmd.h"

#include "core/history/histomanager.h"

#include "smartplaylist/smartplaylist.h"
#include "widgets/main/main_right.h"
#include "widgets/editors/editor_playlist.h"
#include "widgets/editors/editor_album.h"
#include "widgets/editors/editor_artist.h"
#include "widgets/editors/editor_track.h"
#include "widgets/editors/editor_smart.h"

#include "threadmanager.h"
#include "settings.h"
#include "utilities.h"
#include "global_actions.h"
#include "debug.h"
#include "filedialog.h"

#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#endif

#include <QtCore>
#include <QSqlQuery>
#include <QGraphicsView>
#include <QGraphicsSceneEvent>
/*
********************************************************************************
*                                                                              *
*    Class LocalScene                                                          *
*                                                                              *
********************************************************************************
*/
LocalScene::LocalScene(QWidget *parent) : SceneBase(parent)
{
    //! set model
    m_localTrackModel     = LocalTrackModel::instance();
    m_localPlaylistModel  = LocalPlaylistModel::instance();
    m_histoModel          = HistoModel::instance();

    connect(VirtualPlayqueue::instance(), SIGNAL(signal_playing_status_change()),  this, SLOT(update()));
    connect(m_localTrackModel, SIGNAL(dataChanged()), this, SLOT(slot_on_model_data_changed()));
}

/*******************************************************************************
    initScene
*******************************************************************************/
void LocalScene::initScene()
{
    m_infosize          = 0;
    m_mouseGrabbedItem  = 0;
    item_count          = 0;
    
    /*  graphic item context menu */
    m_graphic_item_menu = new GraphicsItemMenu(0);
    m_graphic_item_menu->setBrowserView(this->parentView());
    QObject::connect(m_graphic_item_menu, SIGNAL(menu_action_triggered(ENUM_ACTION_ITEM_MENU)), this, SLOT(slot_contextmenu_triggered(ENUM_ACTION_ITEM_MENU)), Qt::DirectConnection);

    //! gestion du double click par action globale et data
    ACTIONS()->insert(BROWSER_ITEM_RATING_CLICK, new QAction(this));
    ACTIONS()->insert(BROWSER_LOCAL_ITEM_MOUSE_MOVE, new QAction(this));
    
    connect(ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE), SIGNAL(triggered()), this, SLOT(slot_item_mouseMove()), Qt::DirectConnection);
    connect(ACTIONS()->value(BROWSER_ITEM_RATING_CLICK), SIGNAL(triggered()), this, SLOT(slot_item_ratingclick()), Qt::DirectConnection);
    
    setInit(true);
}


/*******************************************************************************
    actions
*******************************************************************************/
QList<QAction *> LocalScene::actions() 
{
    if(m_actions.isEmpty())
    {
        /*  scene actions */
        m_actions.insert("album_grid",      new QAction(QIcon(),QString(tr("view grid")),this));
        m_actions.insert("playlist_tracks", new QAction(QIcon(),QString(tr("view by tracks")),this));
        m_actions.insert("new_playlist",    ACTIONS()->value(NEW_PLAYLIST));
        m_actions.insert("new_smart",       ACTIONS()->value(NEW_SMART_PLAYLIST));
        m_actions.insert("reload_histo",    new QAction(QIcon(":/images/rebuild.png"),QString(tr("reload history")),this));
        m_actions.insert("clear_histo",     new QAction(QIcon(),QString(tr("clear history")),this));
        
        m_actions.value("album_grid")->setCheckable(true);
        m_actions.value("album_grid")->setChecked( SETTINGS()->_album_view_type == 0);
        
        m_actions.value("playlist_tracks")->setCheckable(true);
        m_actions.value("playlist_tracks")->setChecked(SETTINGS()->_playlist_view_type ==1);

        connect(m_actions.value("album_grid"), SIGNAL(triggered()), this, SLOT(slot_change_view_settings()));
        connect(m_actions.value("playlist_tracks"), SIGNAL(triggered()), this, SLOT(slot_change_view_settings()));

        connect(m_actions.value("reload_histo"), SIGNAL(triggered()), this, SLOT(populateScene()));
        connect(m_actions.value("clear_histo"), SIGNAL(triggered()), this, SLOT(slot_clear_history()));     
    }

    QList<QAction*> list;
   
    switch(this->mode())
    {
      case VIEW::ViewAlbum:
        list << m_actions.value("album_grid");
        break;
      case VIEW::ViewHistory:
        list << m_actions.value("reload_histo");
        list << m_actions.value("clear_histo");
        break;
      case VIEW::ViewPlaylist:
        list << m_actions.value("playlist_tracks");
        list << m_actions.value("new_playlist");
        break;
      case VIEW::ViewSmartPlaylist:
        list << m_actions.value("new_smart");
        break;
      default:break;
    }
  
    return list;
}

    

/*******************************************************************************
     setSearch
*******************************************************************************/
void LocalScene::setSearch(const QVariant& variant)
{
    Debug::debug() << "   [LocalScene] setSearch";
    
    if(variant.canConvert<MediaSearch>())
    {
       m_localTrackModel->setSearch( variant );
       m_localPlaylistModel->setSearch( variant );
       m_histoModel->setSearch( variant );
    }
    else
    {
      QString pattern = variant.toString();
      
      MediaSearch search;
      search.search_type_   =  MediaSearch::Type_Or;
      search.query_list_    =  SearchQueryList() 
             << SearchQuery(SearchQuery::field_artist_name, SearchQuery::op_Contains, pattern)
             << SearchQuery(SearchQuery::field_album_name, SearchQuery::op_Contains, pattern)
             << SearchQuery(SearchQuery::field_track_trackname, SearchQuery::op_Contains, pattern)
             << SearchQuery(SearchQuery::field_genre_name, SearchQuery::op_Contains, pattern);      
      
       m_localTrackModel->setSearch( QVariant::fromValue<MediaSearch>(search) );
       m_localPlaylistModel->setSearch( QVariant::fromValue<MediaSearch>(search) );
       m_histoModel->setSearch( QVariant::fromValue<MediaSearch>(search) );
    }
}

/*******************************************************************************
     setData
*******************************************************************************/
void LocalScene::setData(const QVariant& data)
{
    MEDIA::LinkPtr active_link = qvariant_cast<MEDIA::LinkPtr>(data);
    
    if( active_link )
    {
      Debug::debug() << "LocalScene::setData active link" << active_link->name;
        
      m_localTrackModel->setActiveLink(active_link);
    }
    else if (mode() == VIEW::ViewGenre)
    {
      m_localTrackModel->setActiveLink(m_localTrackModel->rootLink());
    }
}

/*******************************************************************************
     resizeScene
*******************************************************************************/
void LocalScene::resizeScene()
{
    //Debug::debug() << "   [LocalScene] resizeScene";   
    int new_item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;

    if(item_count != new_item_count)  
    {
      populateScene();
    }
    else
    {
      update();
    }
}

/*******************************************************************************
     slot_change_view_settings
*******************************************************************************/
void LocalScene::slot_change_view_settings()
{
    SETTINGS()->_album_view_type    = m_actions.value("album_grid")->isChecked() ? 0 : 1;
    SETTINGS()->_playlist_view_type = m_actions.value("playlist_tracks")->isChecked() ? 1 : 0;
    
    populateScene();
}

/*******************************************************************************
    slot_on_model_data_changed
*******************************************************************************/
void LocalScene::slot_on_model_data_changed()
{
    Debug::debug() << "   [LocalScene] slot_on_model_data_changed";

    MEDIA::LinkPtr link = m_localTrackModel->activeLink();
    
    QVariant v;
    v.setValue( static_cast<MEDIA::LinkPtr>(link) );
       
    emit linked_changed( v );
}


/*******************************************************************************
    populateScene
*******************************************************************************/
void LocalScene::populateScene()
{
    Debug::debug() << "   [LocalScene] PopulateScene";

    //! clear scene and delete all items
    clear();
    
    /* si model est vide et database est en cours de construction */
    if(m_localTrackModel->isEmpty() && ThreadManager::instance()->isDbRunning())
    {
      populateLocalSceneBuilding();
    }    
    else 
    {
      switch(mode())
      {
        case VIEW::ViewAlbum          : populateAlbumScene();    break;
        case VIEW::ViewArtist         : populateArtistScene();   break;
        case VIEW::ViewTrack          : populateTrackScene();    break;
        case VIEW::ViewGenre          : populateGenreScene();    break;
        case VIEW::ViewYear           : populateYearScene();     break;
        case VIEW::ViewFavorite       : populateFavoriteScene(); break;
        case VIEW::ViewPlaylist       : populatePlaylistScene(); break;
        case VIEW::ViewSmartPlaylist  : populatePlaylistSmartScene(); break;
        case VIEW::ViewHistory        : populateHistoScene();     break;
        case VIEW::ViewDashBoard      : populateDashBoardScene(); break;
        default: break;
      }
    }

    //! we need to ajust SceneRect
    setSceneRect ( itemsBoundingRect().adjusted(0, -10, 0, 40) );
}


void LocalScene::populateLocalSceneBuilding()
{
    LoadingGraphicItem *info = new LoadingGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    info->_text = tr("Updating music database");
    info->setPos( 0 , 70);
    addItem(info);
}



void LocalScene::populateAlbumScene()
{
    if( VIEW::ViewAlbum_Type(SETTINGS()->_album_view_type) == VIEW::album_grid)
      populateAlbumGridScene();
    else
      populateAlbumExtendedScene();
}

void LocalScene::populatePlaylistScene()
{
    if( VIEW::ViewPlaylist_Type(SETTINGS()->_playlist_view_type) == VIEW::playlist_overview)
      populatePlaylistOverviewScene();
    else
      populatePlaylistByTrackScene();
}

void LocalScene::populateAlbumExtendedScene()
{
    int albumRow  = 0;
    int artistRow = 0;
    int Column    = 0;
    m_infosize    = 0;

    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;


    /* artist loop */
    for ( int i = 0; i < m_localTrackModel->rootItem()->childCount(); i++ )
    {
      if(!m_localTrackModel->isMediaMatch(m_localTrackModel->rootItem()->child(i)) ) continue;

      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast( m_localTrackModel->rootItem()->child(i) );

      CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      category->m_name = artist->name;
      category->setPos( 0 ,10 + artistRow*50 + albumRow*170);

      addItem(category);

      Column = 0;
      artistRow++;

      /* album loop */
      for (int j = 0; j < artist->childCount(); j++)
      {
        if(!m_localTrackModel->isMediaMatch(artist->child(j)) ) continue;

        MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast( artist->child(j) );

        m_infosize++;

        AlbumGraphicItem_v2 *album_item = new AlbumGraphicItem_v2();
        album_item->media = album;
        album_item->setPos(4+160*Column, artistRow*50 + albumRow*170);
        addItem(album_item);


        if(Column < (item_count-1)) {
          Column++;
        }
        else {
          Column = 0;
          albumRow++;
        }
      }
      if(Column>0) albumRow++;
    }

    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + artistRow*50 + albumRow*20);
      addItem(info);
    }
}

void LocalScene::populateAlbumGridScene()
{
    int albumRow      = 0;
    int Column        = 0;
    m_infosize        = 0;
    int categorieRow  = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;


    CategorieGraphicItem *cat = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    cat->m_name = tr("All Albums");
    cat->setPos( 0 ,10 + categorieRow*50);
    categorieRow++;
    addItem(cat);


    /* artist loop */
    for ( int i = 0; i < m_localTrackModel->rootItem()->childCount(); i++ )
    {
      if(!m_localTrackModel->isMediaMatch(m_localTrackModel->rootItem()->child(i)) ) continue;

      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(m_localTrackModel->rootItem()->child(i));

      /* album loop */
      for (int j = 0; j < artist->childCount(); j++)
      {
        if(!m_localTrackModel->isMediaMatch(artist->child(j)) ) continue;

        MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast( artist->child(j) );

        m_infosize++;

        AlbumGraphicItem *album_item = new AlbumGraphicItem();
        album_item->media = album;
        album_item->setPos(4+160*Column, 10 + albumRow*170 + categorieRow*50);
        addItem(album_item);

        if(Column < (item_count-1)) {
          Column++;
        }
        else {
          Column = 0;
          albumRow++;
        }
      }
    }

    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 ,10 + categorieRow*50);
      addItem(info);
    }
}



void LocalScene::populateArtistScene()
{
    int artistRow    = 0;
    int categorieRow = 0;
    int Column       = 0;
    m_infosize       = 0;
    int idx          = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;
   
    
    /* artist loop */
    QChar start_char;
    for (int i=0 ; i < m_localTrackModel->rootItem()->childCount(); i++ )
    {
        if(! m_localTrackModel->isMediaMatch(m_localTrackModel->rootItem()->child(i)) ) continue;

        MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(m_localTrackModel->rootItem()->child(i));

        QChar current_char = artist->name.at(0).toLower();

        if(start_char !=  current_char) {
          /*  new category */
          start_char = current_char;
          if(idx>0) artistRow++;

          CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
          category->m_name = QString(start_char);
          category->setPos( 0 , 10 + categorieRow*50 + artistRow*165);
          addItem(category);

          Column = 0;
          categorieRow++;
        }

        /* add new artist item */
        m_infosize++;
        idx++;

        /* new Artist Item */
        ArtistGraphicItem *artist_item = new ArtistGraphicItem();
        artist_item->media = artist;
        artist_item->setPos( 4 + 160*Column , categorieRow*50 + artistRow*165);
        addItem(artist_item);

        /* ALBUM COVER LOOP */
        artist_item->albums_covers.clear();
        for(int j = artist->childCount()-1 ; j >= 0; j--) {
          if(!m_localTrackModel->isMediaMatch(artist->child(j)) ) continue;
          MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(artist->child(j));

          artist_item->albums_covers.prepend(album);

          /* WARNING limite de l'affichage à 6 cover max */
          if(artist_item->albums_covers.size() >=6) break;
        }
        
        
        if(Column < (item_count-1)) {
          Column++;
        }
        else {
          Column = 0;
          artistRow++;
          idx    = 0;
        }

    } /* end for artist loop */

    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50);
      addItem(info);
    }
}



void LocalScene::populateTrackScene()
{
    int artistRow     = 0;
    int albumRow      = 0;
    int trackRow      = 0;
    int trackPerAlbum = 0;
    int offset        = 0;
    m_infosize        = 0;


    //! artist loop
    for ( int i = 0; i < m_localTrackModel->rootItem()->childCount(); i++ )
    {
      if(!m_localTrackModel->isMediaMatch(m_localTrackModel->rootItem()->child(i)) ) continue;

      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(m_localTrackModel->rootItem()->child(i));
      
      CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      category->m_name = artist->name;
      category->setPos( 0 ,artistRow*50 + trackRow*20 + albumRow*30 + offset );
      addItem(category);

      artistRow++;

      //! album loop
      for (int j = 0; j < artist->childCount(); j++)
      {
        if(!m_localTrackModel->isMediaMatch(artist->child(j))) continue;

        MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(artist->child(j));

        AlbumGraphicItem_v2 *album_item = new AlbumGraphicItem_v2();
        album_item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        album_item->media = album;
        album_item->setPos(4, artistRow*50 + trackRow*20 + albumRow*30 + offset );
        addItem(album_item);

        trackPerAlbum = 0;

        //! track loop
        int disc_number = 0;
        bool m_isGrouping = Database::instance()->param()._option_group_albums;
        for (int k = 0; k < album->childCount(); k++)
        {
          if(!m_localTrackModel->isMediaMatch(album->child(k))) continue;

          MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(album->child(k));

          if(m_isGrouping && album->isMultiset() && (disc_number != track->disc_number)) 
          {
            disc_number = track->disc_number;
            
            QGraphicsTextItem* text_item = new QGraphicsTextItem(QString(tr("disc %1")).arg(disc_number));
            text_item->setDefaultTextColor(QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
            text_item->setFont( QFont("Arial", 10, QFont::Bold) );

            text_item->setPos(160, artistRow*50 + trackRow*20 + albumRow*30 + offset);
            addItem(text_item);

            trackRow++;
          }

          TrackGraphicItem_v2 *track_item = new TrackGraphicItem_v2();
          track_item->media = track;
          track_item->setPos(155, artistRow*50 + trackRow*20 + albumRow*30 + offset);

          //PATCH (-20 => fix alignement of the scene)
          track_item->_width = parentView()->width()-155-20;

          addItem(track_item);

          trackRow++;
          trackPerAlbum++;
          m_infosize++;
        }
        albumRow++;

        if( trackPerAlbum < 8 ) offset = offset + (8 - trackPerAlbum)*20;
      }
    }

    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + artistRow*50);
      addItem(info);
    }
}



void LocalScene::populateGenreScene()
{
    int categorieRow = 0;
    int albumRow     = 0;
    int Column       = 0;
    m_infosize       = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;

    
    int Xpos = 20,Ypos = 10;
    
    /* -------------- add ROOT button ------------------ */
    ButtonStateItem* button = new ButtonStateItem();
    button->setText(m_localTrackModel->rootLink()->name);
    button->setChecked(m_localTrackModel->rootLink() == m_localTrackModel->activeLink());
    QVariant v;
    v.setValue(static_cast<MEDIA::LinkPtr>(m_localTrackModel->rootLink()));
    button->setData(v);
  
    connect(button, SIGNAL(clicked()), m_localTrackModel, SLOT(slot_activate_link()));
      
    button->setPos(Xpos ,Ypos );
    Xpos = Xpos + button->width() + 20;

    addItem(button);    
    
    /* -------------- add CHILD GENRE button ----------- */
    for ( int i = 0; i < m_localTrackModel->rootLink()->childCount(); i++ )
    {
      MEDIA::LinkPtr link = MEDIA::LinkPtr::staticCast( m_localTrackModel->rootLink()->child(i) );
        
      ButtonStateItem* button = new ButtonStateItem();
      button->setText(link->name);
      button->setChecked(link == m_localTrackModel->activeLink());
      QVariant v;
      v.setValue(static_cast<MEDIA::LinkPtr>(link));
      button->setData(v);
  
      connect(button, SIGNAL(clicked()), m_localTrackModel, SLOT(slot_activate_link()));
      
      if(Xpos + button->width() > parentView()->width()-20) {
        Xpos = 20;
        Ypos = Ypos + 30;
      }

      button->setPos(Xpos ,Ypos );
      Xpos = Xpos + button->width() + 20;

      addItem(button);
    }      
    
    /* -------------- add ALBUMS by GENRE ------------- */
    categorieRow++;
    for ( int i = 0; i < m_localTrackModel->rootLink()->childCount(); i++ )
    {
      MEDIA::LinkPtr link = MEDIA::LinkPtr::staticCast( m_localTrackModel->rootLink()->child(i) );

      if(m_localTrackModel->activeLink() != m_localTrackModel->rootLink())
          if( m_localTrackModel->activeLink() != link ) continue;
      
      if(!m_localTrackModel->isMediaMatch(link)) continue;

      /* ------- New Genre ------- */
      if(Column>0) albumRow++;

      CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      category->m_name = link->name;
      category->setPos( 0 , Ypos + 10 + categorieRow*50 + albumRow*170);
      addItem(category);

      m_infosize++; // on compte les categorie = genre
      categorieRow++;
      Column     = 0;
      
      for ( int j = 0; j < link->childCount(); j++ )
      {
          if(!m_localTrackModel->isMediaMatch(link->child(j))) continue;
      
          AlbumGenreGraphicItem *album_item = new AlbumGenreGraphicItem();
          album_item->media    = MEDIA::AlbumPtr::staticCast( link->child(j) );
          album_item->_genre   = link->name;
          album_item->setPos(4+160*Column, Ypos + categorieRow*50 + albumRow*170);
          addItem(album_item);

          if(Column < (item_count-1)) {
              Column++;
          }
          else {
            Column = 0;
            albumRow++;
          }
      }
    }

    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , Ypos + 10 + categorieRow*50);
      addItem(info);
    }
}


void LocalScene::populateYearScene()
{
    int categorieRow = 0;
    int albumRow     = 0;
    int Column       = 0;
    m_infosize       = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;


    int year         = -1;
    int idx          = 0;

    QList<MEDIA::AlbumPtr> list_album_by_year = m_localTrackModel->albumItemList;

    //! Sort Media Album Item list By Year
    qSort(list_album_by_year.begin(), list_album_by_year.end(),MEDIA::compareAlbumItemYear);

    foreach (MEDIA::AlbumPtr album, list_album_by_year)
    {
      //! add filtre
      if(!m_localTrackModel->isMediaMatch(album)) continue;

      if(year != album->year) {
        //! new year
        year = album->year;

        if(idx>0) albumRow++;

        CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
        category->m_name = QString::number(year);
        category->setPos( 0 , 10 + categorieRow*50 + albumRow*170);
        addItem(category);

        m_infosize++; // on compte les categorie = genre
        categorieRow++;
        Column     = 0;
        idx        = 0;
      }

      AlbumGraphicItem *album_item = new AlbumGraphicItem();
      album_item->media  = album;
      album_item->setPos(4+160*Column, categorieRow*50 + albumRow*170);
      addItem(album_item);

      if(Column < (item_count-1)) {
        Column++;
        idx++;
      }
      else {
        Column = 0;
        albumRow++;
        idx=0;
      }
    } // fin foreach track


    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50);
      addItem(info);
    }
}



void LocalScene::populateFavoriteScene()
{
    //Debug::debug() << "   [LocalScene] populateFavoriteScene";
    
    int categorieRow = 0;
    int artistRow    = 0;
    int albumRow     = 0;
    int Column       = 0;
    m_infosize        = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;


    
    CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    category->m_name = tr("Artists");
    category->setPos( 0 , 10 + categorieRow*50 + artistRow*165);
    addItem(category);

    categorieRow++;


    /*-----------------------------------------------*/
    /* Favorite Artists                              */
    /* ----------------------------------------------*/
    for ( int i = 0; i < m_localTrackModel->rootItem()->childCount(); i++ ) {

      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(m_localTrackModel->rootItem()->child(i));

      if(!artist->isFavorite) continue;
      if(!m_localTrackModel->isMediaMatch(artist)) continue;

      m_infosize++; // we count artist favorite item

      //! New Artist Item
      ArtistGraphicItem *artist_item = new ArtistGraphicItem();
      artist_item->media = artist;
      artist_item->setPos( 4 + 160*Column , categorieRow*50 + artistRow*165);
      addItem(artist_item);

      if(Column < (item_count-1)) {
        Column++;
      }
      else {
        Column = 0;
        artistRow++;
      }
    } // end artist loop

    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50 + artistRow*165);
      addItem(info);
      categorieRow++;
    }


    /*-----------------------------------------------*/
    /* Favorite Albums                               */
    /* ----------------------------------------------*/
    if (Column > 0) artistRow++;

    CategorieGraphicItem *category2 = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    category2->m_name = tr("Albums");
    category2->setPos( 0 , 10 + categorieRow*50 + artistRow*165);
    addItem(category2);

    Column = 0;
    categorieRow++;
    int oldInfoSize = m_infosize;
    foreach (MEDIA::AlbumPtr album, m_localTrackModel->albumItemList)
    {
        if(!album->isFavorite)  continue;
        if(!m_localTrackModel->isMediaMatch(album)) continue;

        m_infosize++; // we count album favorite item

        AlbumGraphicItem *album_item = new AlbumGraphicItem();
        album_item->media = album;
        album_item->setPos(4+160*Column, categorieRow*50 + artistRow*165 + albumRow*170);
        addItem(album_item);

        if(Column < (item_count-1)) {
          Column++;
        }
        else {
          Column = 0;
          albumRow++;
        }
    } // end album loop

    if(oldInfoSize == m_infosize) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text   = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50 + artistRow*165);
      addItem(info);
    }
}



void LocalScene::populatePlaylistOverviewScene()
{
    int categorieRow = 0;
    int playlistRow  = 0;
    int Column       = 0;
    m_infosize       = 0;
    int idx    = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;


    /*-----------------------------------------------*/
    /* Playlist T_DATABASE                           */
    /* ----------------------------------------------*/
    CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    category->m_name = tr("All playlists");
    category->setPos( 0 ,10 + categorieRow*50 + playlistRow*150);
    addItem(category);
    categorieRow++;

    //! playlist loop for T_DATABASE playlist
    int i = 0;
    for ( ; i < m_localPlaylistModel->rootItem()->childCount(); i++ )
    {
      MEDIA::PlaylistPtr playlist = MEDIA::PlaylistPtr::staticCast(m_localPlaylistModel->rootItem()->child(i));

      // keep only database or file playlist
      if(playlist->p_type != T_DATABASE && playlist->p_type != T_FILE) break;

      if(!m_localPlaylistModel->isMediaMatch(playlist) ) continue;

      m_infosize++;
      idx++; // nouvelle ligne

      PlaylistGraphicItem *playlist_item = new PlaylistGraphicItem();
      playlist_item->media = playlist;
      playlist_item->setPos(4+160*Column, categorieRow*50 + playlistRow*150);
      addItem(playlist_item);

      if(Column < (item_count-1)) {
        Column++;
      }
      else {
        Column = 0;
        playlistRow++;
        idx = 0;
      }
    }

    if(m_infosize == 0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50 + playlistRow*150);
      addItem(info);
      categorieRow++;
    }
}


void LocalScene::populatePlaylistByTrackScene()
{
    int categorieRow  = 0;
    int playlistRow   = 0;
    int trackRow      = 0;
    int trackPerAlbum = 0;
    int offset        = 0;
    m_infosize         = 0;

   

    //! playlist loop
    for ( int i = 0; i < m_localPlaylistModel->rootItem()->childCount(); i++ )
    {
      MEDIA::PlaylistPtr playlist = MEDIA::PlaylistPtr::staticCast(m_localPlaylistModel->rootItem()->child(i));

      //WARNING drop smart playlist for this view
      if(playlist->p_type == T_SMART) break;

      if(!m_localPlaylistModel->isMediaMatch(playlist) ) continue;

      CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      category->m_name = playlist->name;
      category->setPos( 0 , 10 + categorieRow*50 + playlistRow*30 + trackRow*20 + offset );
      addItem(category);

      categorieRow++;

      PlaylistGraphicItem *playlist_item = new PlaylistGraphicItem();
      playlist_item->setFlag(QGraphicsItem::ItemIsSelectable, false);
      playlist_item->media = playlist;
      playlist_item->setPos(10, categorieRow*50 + playlistRow*30 + trackRow*20 + offset );
      addItem(playlist_item);

      //! track loop
      trackPerAlbum = 0;
      for (int k = 0; k < playlist->childCount(); k++)
      {
        if(!m_localPlaylistModel->isMediaMatch(playlist->child(k))) continue;

        MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(playlist->child(k));

        TrackGraphicItem *track_item = new TrackGraphicItem();
        track_item->media = track;
        track_item->setPos(155, categorieRow*50 + playlistRow*30 + trackRow*20 + offset);
        track_item->_width = parentView()->width()-155 -20;

        addItem(track_item);

        trackRow++;
        trackPerAlbum++;
        m_infosize++;
      }
      playlistRow++;
      if( trackPerAlbum < 6 ) offset = offset + (6 - trackPerAlbum)*25;
    }


    if(m_infosize==0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 0 , 10 + categorieRow*50 + playlistRow*150);
      addItem(info);
    }
}



void LocalScene::populatePlaylistSmartScene()
{
    int categorieRow = 0;
    int playlistRow  = 0;
    int Column       = 0;
    m_infosize        = 0;
    int idx = 0;
    item_count = (parentView()->width()/160 > 2) ? parentView()->width()/160 : 2;


    CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    category->m_name = tr("Smart playlists");
    category->setPos( 0 ,10 + categorieRow*50 + playlistRow*150);
    addItem(category);
    categorieRow++;


    //! playlist loop
    for ( int i = 0; i < m_localPlaylistModel->rootItem()->childCount(); i++ )
    {
      MEDIA::PlaylistPtr playlist = MEDIA::PlaylistPtr::staticCast(m_localPlaylistModel->rootItem()->child(i));
      if(playlist->p_type != T_SMART)
        continue;

      m_infosize++;
      idx++; // nouvelle ligne

      PlaylistGraphicItem *playlist_item = new PlaylistGraphicItem();
      playlist_item->media = playlist;
      playlist_item->setPos(4+160*Column, categorieRow*50 + playlistRow*150);
      addItem(playlist_item);

      if(Column < (item_count-1)) {
        Column++;
      }
      else {
        Column = 0;
        playlistRow++;
        idx = 0;
      }
    }
}



void LocalScene::populateHistoScene()
{
    //Debug::debug() << "   [LocalScene] populateHistoScene";

    int categorieRow  = 0;
    int trackRow      = 0;
    m_infosize        = 0;

    m_histoModel->updateModel();


    //! track loop from history model
    QDate oldDate, newDate;
    for ( int i = 0; i < m_histoModel->itemCount(); i++ )
    {
      MEDIA::TrackPtr track = m_histoModel->trackAt(i);
      
      if(!m_histoModel->isMediaMatch( track )) continue;
      
      newDate = QDateTime::fromTime_t(track->lastPlayed).date();

      if(oldDate != newDate)
      {
        oldDate = newDate;

        CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
        category->m_name = track->lastplayed_ago();
        category->setPos(0 , 10 + categorieRow*50 + trackRow*20);
        addItem(category);

        categorieRow++;
      }

      TrackGraphicItem_v3 *track_item = new TrackGraphicItem_v3();
      track_item->media = track;
      track_item->setPos(30, categorieRow*50 + trackRow*20);
      track_item->_width = parentView()->width()-50 -20;

      if(track->type() == TYPE_STREAM)
        track_item->setToolTip(track->url);

      addItem(track_item);
      m_infosize++;

      trackRow++;
    }

    if (m_infosize == 0) {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = tr("No entry found");
      info->setPos( 10 ,10 + categorieRow*50);
      addItem(info);
    }
}

/*******************************************************************************
 Mouse and menu event
*******************************************************************************/
void LocalScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Debug::debug() << "   [LocalScene] contextMenuEvent";
    m_mouseGrabbedItem = this->itemAt(event->scenePos(), QTransform());
    if(!m_mouseGrabbedItem) {
      m_graphic_item_menu->hide();
      event->accept();
      
      return;
    }

    if(this->selectedItems().isEmpty() || !selectedItems().contains(m_mouseGrabbedItem))
    {
      m_graphic_item_menu->appendItem(m_mouseGrabbedItem);
      m_graphic_item_menu->updateMenu(false);
    }
    else if (selectedItems().count() == 1)
    {
      m_mouseGrabbedItem = selectedItems().first();
      clearSelection();
      m_graphic_item_menu->appendItem(m_mouseGrabbedItem);
      m_graphic_item_menu->updateMenu(false);
    }
    else 
    {
      m_graphic_item_menu->appendItems(selectedItems());
      m_graphic_item_menu->updateMenu(true);
    }

    m_graphic_item_menu->exec(event->screenPos());

    // no need to propagate to item
    event->accept();
}

void LocalScene::slot_contextmenu_triggered(ENUM_ACTION_ITEM_MENU id)
{
    switch(id)
    {
      case ALBUM_PLAY          : playAlbum();              break;
      case ALBUM_QUEUE_END     : enqueueAlbum(false);      break;

      case GENRE_PLAY          : playAlbumGenre();         break;
      case GENRE_QUEUE_END     : enqueueAlbumGenre(false); break;

      case ARTIST_PLAY         : playArtist();             break;
      case ARTIST_QUEUE_END    : enqueueArtist(false);     break;

      case PLAYLIST_PLAY       : playPlaylist();           break;
      case PLAYLIST_QUEUE_END  : enqueuePlaylist(false);   break;
      case PLAYLIST_REMOVE     : removePlaylistFromDisk(); break;

      case TRACK_PLAY          : playTrack();              break;
      case TRACK_QUEUE_END     : enqueueTrack(false);      break;

      case UPDATE_FAVORITE     : updateMediaFavorite();    break;
      case EDIT                : edit_media_dialog();      break;

      case SELECTION_PLAY      : playSelected();break;
      case SELECTION_QUEUE_END : enqueueSelected(false);break;
      case SELECTION_TRACK_EDIT: edit_media_dialog_selection();break;

      default:break;
    }
}


void LocalScene::slot_item_mouseMove()
{
    Debug::debug() << "   [LocalScene] slot_item_mouseMove";

    // single drag and drop
    QGraphicsItem *gItem = qvariant_cast<QGraphicsItem *>( (ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE))->data() );

    if(this->selectedItems().isEmpty() || !selectedItems().contains(gItem))
    {
      switch(gItem->type()) {
        case GraphicsItem::AlbumType      : startAlbumsDrag(gItem);break;
        case GraphicsItem::AlbumGenreType : startAlbumsGenreDrag(gItem);break;
        case GraphicsItem::ArtistType     : startArtistsDrag(gItem);break;
        case GraphicsItem::TrackType      : startTracksDrag(gItem);break;
        case GraphicsItem::PlaylistType   : startPlaylistsDrag(gItem);break;
      }
    }
    // multiple drag and drop
    else
    {
      switch(gItem->type()) {
        case GraphicsItem::AlbumType      : startAlbumsDrag();break;
        case GraphicsItem::AlbumGenreType : startAlbumsGenreDrag();break;
        case GraphicsItem::ArtistType     : startArtistsDrag();break;
        case GraphicsItem::TrackType      : startTracksDrag();break;
        case GraphicsItem::PlaylistType   : startPlaylistsDrag();break;
      }
    }
}

void LocalScene::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    //Debug::debug() << "   [LocalScene] mousePressEvent";  
    m_mouseGrabbedItem = this->itemAt(event->scenePos(), QTransform());

    if(m_mouseGrabbedItem && (Qt::ShiftModifier == QApplication::keyboardModifiers()) )
    {
      QPoint currentpoint = m_mouseGrabbedItem->scenePos().toPoint();
      QPoint startPoint;

      int mindistance;
      QList<QGraphicsItem*> oldSelected = sortedSelectedItem();
      foreach(QGraphicsItem* item,  oldSelected )
      {
         QPoint pointdiff = currentpoint - item->scenePos().toPoint();
         mindistance =  currentpoint.manhattanLength();
         if (pointdiff.manhattanLength() < mindistance)
           startPoint =  item->scenePos().toPoint();
      }
      
      if(!startPoint.isNull())
      {
        if(startPoint.y() < currentpoint.y())
          startPoint += QPoint(0,10);
        else
          currentpoint += QPoint(0,10);

       QPainterPath pp;
       pp.addRect( QRect(startPoint, currentpoint) );
       this->setSelectionArea(pp);

       foreach(QGraphicsItem* item, oldSelected )
         item->setSelected(true);
       event->accept();
       return;
      }      
    }

    QGraphicsScene::mousePressEvent(event);
}

    
void LocalScene::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
{
    //Debug::debug() << "   [LocalScene] mouseDoubleClickEvent";
    // A FAIRE EGALEMENT DANS LES AUTRES SCENE SI CELA FONCTIONNE
    m_mouseGrabbedItem = this->itemAt(event->scenePos(), QTransform());

    if(!m_mouseGrabbedItem) {
      event->ignore();
      return;
    }

    // ENQUEUE
    if(Qt::ControlModifier == QApplication::keyboardModifiers())
    {
      switch(m_mouseGrabbedItem->type()) {
        case GraphicsItem::AlbumType      : enqueueAlbum(false);break;
        case GraphicsItem::AlbumGenreType : enqueueAlbumGenre(false);break;
        case GraphicsItem::ArtistType     : enqueueArtist(false);break;
        case GraphicsItem::TrackType      : enqueueTrack(false);break;
        case GraphicsItem::PlaylistType   : enqueuePlaylist(false);break;
        default: event->ignore(); break;
      }
    }
    // PLAY ITEM
    else
    {
        switch(m_mouseGrabbedItem->type()) {
          case GraphicsItem::AlbumType      : playAlbum();break;
          case GraphicsItem::AlbumGenreType : playAlbumGenre();break;
          case GraphicsItem::ArtistType     : playArtist();break;
          case GraphicsItem::TrackType      : playTrack();break;
          case GraphicsItem::PlaylistType   : playPlaylist();break;
          default: event->ignore(); break;
        }
    }

    // no need to propagate to item
    event->accept();
}



/*******************************************************************************
    keyPressEvent
*******************************************************************************/
void LocalScene::keyPressEvent ( QKeyEvent * keyEvent )
{
    //Debug::debug() << "  [LocalScene] keyPressEvent " << keyEvent->key();

    if( keyEvent->key() == Qt::Key_Escape ) 
    {
       clearSelection();
       keyEvent->accept();
       return;      
    }
    else if( keyEvent->key() == Qt::Key_Return ) 
    {
       if( selectedItems().isEmpty() )
           playSelected();
//        else
//            playItem();
       
       keyEvent->accept();
       return;       
    }
    
    QGraphicsScene::keyPressEvent (keyEvent);
}

/*******************************************************************************
    slot_item_ratingclick
*******************************************************************************/
void LocalScene::slot_item_ratingclick()
{
    QList<QGraphicsView *> view = this->views();
    if( view.isEmpty() )
      return;

    //Debug::debug() << "   [LocalScene] slot_item_ratingclick";
    QGraphicsItem *gItem = qvariant_cast<QGraphicsItem *>( (ACTIONS()->value(BROWSER_ITEM_RATING_CLICK))->data() );

    QList<QGraphicsItem*> items;
    if(selectedItems().contains(gItem)) 
      items << selectedItems();
    else
      items << gItem;

    QList<MEDIA::MediaPtr> medias;
    foreach(QGraphicsItem* gi, items) 
    {
        switch(gi->type()) {
            case GraphicsItem::AlbumType      :
            case GraphicsItem::AlbumGenreType : medias << static_cast<AlbumGraphicItem*>(gi)->media;break;
            case GraphicsItem::ArtistType     : medias << static_cast<ArtistGraphicItem*>(gi)->media;break;
            case GraphicsItem::TrackType      : medias << static_cast<TrackGraphicItem*>(gi)->media;break;
            default: return;break;
        }
    }
      
    /* rate in database */  
    QtConcurrent::run(DatabaseCmd::rateMediaItems, medias);
}

/*******************************************************************************
  LocalScene::sortedSelectedItem
*******************************************************************************/
QList<QGraphicsItem*> LocalScene::sortedSelectedItem()
{
    QList<QGraphicsItem*> selected = selectedItems();
    int count = selected.count();

    bool sort_ok = false;

    do
    {
      sort_ok = false;
      for(int i =0; i<=count-2; i++)
      {
        QGraphicsItem* gi1 = selected.at(i);
        QGraphicsItem* gi2 = selected.at(i+1);

        QPoint p1 = gi1->scenePos().toPoint();
        QPoint p2 = gi2->scenePos().toPoint();

        bool lessthan;
        if(p1.y() < p2.y())
          lessthan = true;
        else if (p1.y() > p2.y())
          lessthan = false;
        else if (p1.x() < p2.x())
          lessthan = true;
        else
          lessthan = false;

        if(!lessthan) {
          selected.swap(i,i+1);
          sort_ok = true;
        }
      } // end for
    } while(sort_ok);

    return selected;
}


/*******************************************************************************
  LocalScene::get_item_position
*******************************************************************************/
QPoint LocalScene::get_item_position(MEDIA::MediaPtr media)
{
    Debug::debug() << "   [LocalScene] get_item_position";
    QPoint point = QPoint(0,0);
    if(media->type() == TYPE_ARTIST && mode() == VIEW::ViewArtist)
    {
      foreach(QGraphicsItem* gItem, this->items()) {
        if(gItem->type() != GraphicsItem::ArtistType) continue;

        ArtistGraphicItem* artist_item = static_cast<ArtistGraphicItem*>(gItem);

        if(artist_item->media->id == MEDIA::ArtistPtr::staticCast(media)->id) {
          point = artist_item->scenePos().toPoint();
          break;
        }
      }
    }
    else if(media->type() == TYPE_ALBUM && mode() == VIEW::ViewAlbum)
    {
      foreach(QGraphicsItem* gItem, this->items()) {
        if(gItem->type() != GraphicsItem::AlbumType) continue;

        AlbumGraphicItem* album_item = static_cast<AlbumGraphicItem*>(gItem);

        if(album_item->media->id == MEDIA::AlbumPtr::staticCast(media)->id) {
          point = album_item->scenePos().toPoint();
          break;
        }
      }
    }
    else if(media->type() == TYPE_TRACK && mode() == VIEW::ViewTrack)
    {
      foreach(QGraphicsItem* gItem, this->items()) {
        if(gItem->type() != GraphicsItem::TrackType) continue;

        TrackGraphicItem* track_item = static_cast<TrackGraphicItem*>(gItem);

        if(track_item->media->id == MEDIA::TrackPtr::staticCast(media)->id) {
          point = track_item->scenePos().toPoint();
          break;
        }
      }
    }

    return point;
}


/*******************************************************************************
    Edit media dialog
*******************************************************************************/
void LocalScene::edit_media_dialog()
{
    //Debug::debug() << "   [LocalScene] edit_media_dialog ";
    if(!m_mouseGrabbedItem) return;
    
    QWidget* editor = 0;
    
    if ( ArtistGraphicItem *item = dynamic_cast<ArtistGraphicItem*>(m_mouseGrabbedItem) )
    {
       editor = new EditorArtist( item->media, this );
    }
    else if( AlbumGraphicItem *item = dynamic_cast<AlbumGraphicItem*>(m_mouseGrabbedItem) )
    {
       editor = new EditorAlbum( item->media, this );
    }
    else if( TrackGraphicItem *item = dynamic_cast<TrackGraphicItem*>(m_mouseGrabbedItem) )
    {
       editor = new EditorTrack( item->media, this );
    }
    else if( PlaylistGraphicItem *item = dynamic_cast<PlaylistGraphicItem*>(m_mouseGrabbedItem) )
    {    
      const int type       = int(item->media->p_type);

      if(type == T_DATABASE || type == T_FILE)
      {
        editor = new EditorPlaylist(parentView());
        static_cast<EditorPlaylist*>(editor)->setPlaylist(item->media);
      }
      else //T_SMART
      {
        editor = new EditorSmart(parentView());
        static_cast<EditorSmart*>(editor)->setPlaylist(item->media);
      }
    }
    
    MainRightWidget::instance()->addWidget(editor);
}

void LocalScene::edit_media_dialog_selection()
{
    //Debug::debug() << "   [LocalScene] edit_media_dialog_selection ";
    QList<MEDIA::TrackPtr> tracks;
    QList<QGraphicsItem*> selected_items = sortedSelectedItem();

    foreach(QGraphicsItem* gi, selected_items)
    {
      if (gi->type() == GraphicsItem::TrackType)
      {
        TrackGraphicItem *item = static_cast<TrackGraphicItem*>(gi);
        tracks << item->media;
      }
    } // end foreach
    
    MainRightWidget::instance()->addWidget( new EditorTrack( tracks, this ) );
}

/*******************************************************************************
    Update favorite status
*******************************************************************************/
void LocalScene::updateMediaFavorite()
{
    if(!m_mouseGrabbedItem) return;

    if ( AlbumGraphicItem *item = dynamic_cast<AlbumGraphicItem*>(m_mouseGrabbedItem) )
    {
      item->media->isFavorite = !item->media->isFavorite;
      DatabaseCmd::updateFavorite(item->media, item->media->isFavorite);
      item->update();      
    }
    else if ( ArtistGraphicItem *item = dynamic_cast<ArtistGraphicItem*>(m_mouseGrabbedItem) )
    {
      item->media->isFavorite = !item->media->isFavorite;
      DatabaseCmd::updateFavorite(item->media, item->media->isFavorite);
      item->update();
    }
    else if (PlaylistGraphicItem * item = dynamic_cast<PlaylistGraphicItem*>(m_mouseGrabbedItem) )
    {
      item->media->isFavorite = !item->media->isFavorite;
      DatabaseCmd::updateFavorite(item->media, item->media->isFavorite);
      item->update();
    }
}


/*******************************************************************************
    Playlist stuff
*******************************************************************************/
void LocalScene::removePlaylistFromDisk()
{
    if(!m_mouseGrabbedItem) return;
    PlaylistGraphicItem *item = static_cast<PlaylistGraphicItem*>(m_mouseGrabbedItem);

    //! get data
    const int id         = item->media->id;
    const QString fname  = item->media->url;
    const int type       = int(item->media->p_type);

    Debug::debug() << "   [LocalScene] remove playlist item id" << id;

    //! remove from database
    if (!Database::instance()->open()) return;
    
    if(type == T_SMART)
    {
      QSqlQuery q("",*Database::instance()->db());
      q.prepare("DELETE FROM `smart_playlists` WHERE `id`=?;");
      q.addBindValue( id );
      Debug::debug() << "database -> delete smart playlist :" << q.exec();
    }
    else 
    {      
      /* remove file on disk */
      if(type == T_FILE) 
      {
        QFile playlistFile(fname);
        if(playlistFile.exists())
          playlistFile.remove();
      }      
      
      QSqlQuery q("",*Database::instance()->db());
      q.prepare("DELETE FROM `playlists` WHERE `id`=?;");
      q.addBindValue( id );
      Debug::debug() << "database -> delete playlist :" << q.exec();
    
      q.prepare("DELETE FROM `playlist_items` WHERE `playlist_id` NOT IN (SELECT `id` FROM `playlists`);");
      Debug::debug() << "database -> delete playlist :" << q.exec();
    }

    ThreadManager::instance()->populateLocalPlaylistModel();
}


/*******************************************************************************
    History model clear
*******************************************************************************/
void LocalScene::slot_clear_history()
{
    HistoManager::instance()->clearHistory();
    populateScene();
}


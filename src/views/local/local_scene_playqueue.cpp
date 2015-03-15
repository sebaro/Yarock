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


#include "local_scene.h"

#include "models/local/local_track_model.h"
#include "models/local/local_playlist_model.h"
#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/mediamimedata.h"

#include "smartplaylist/smartplaylist.h"

#include "playqueue/playqueue_model.h"
#include "playqueue/task_manager.h"

#include "debug.h"

#include <QtCore>
#include <QDrag>



/*
********************************************************************************
*        -> queue method                                                       *
********************************************************************************
*/
void LocalScene::enqueueTrack(bool is_new_playqueue)
{
    //Debug::debug() << " ---- enqueueTrack";
    if(!m_mouseGrabbedItem) return;
    TrackGraphicItem *item = static_cast<TrackGraphicItem*>(m_mouseGrabbedItem);

    if(item->media) {
        if(is_new_playqueue)
          Playqueue::instance()->clear();

        Playqueue::instance()->addMediaItem(item->media);
    }
}


void LocalScene::enqueueAlbum(bool is_new_playqueue)
{
    //Debug::debug() << " ---- enqueueAlbum";
    if(!m_mouseGrabbedItem) return;
    AlbumGraphicItem *item = static_cast<AlbumGraphicItem*>(m_mouseGrabbedItem);

    QList<MEDIA::TrackPtr> list = m_localTrackModel->getItemChildrenTracks(item->media);

    if(!list.isEmpty()) {
        if(is_new_playqueue)
          Playqueue::instance()->clear();

        Playqueue::instance()->manager()->playlistAddMediaItems(list);
    }
}

void LocalScene::enqueueAlbumGenre(bool is_new_playqueue)
{
    //Debug::debug() << " ---- enqueueAlbumGenre";
    if(!m_mouseGrabbedItem) return;
    AlbumGenreGraphicItem *item = static_cast<AlbumGenreGraphicItem*>(m_mouseGrabbedItem);

    QList<MEDIA::TrackPtr> list = m_localTrackModel->getAlbumChildrenTracksGenre(item->media,item->_genre);

    if(!list.isEmpty()) {
        if(is_new_playqueue)
          Playqueue::instance()->clear();

        Playqueue::instance()->manager()->playlistAddMediaItems(list);
    }
}

void LocalScene::enqueueArtist(bool is_new_playqueue)
{
    //Debug::debug() << " ---- enqueueArtist";
    if(!m_mouseGrabbedItem) return;
    ArtistGraphicItem *item = static_cast<ArtistGraphicItem*>(m_mouseGrabbedItem);

    QList<MEDIA::TrackPtr> list = m_localTrackModel->getItemChildrenTracks(item->media);

    if(!list.isEmpty()) {
        if(is_new_playqueue)
          Playqueue::instance()->clear();

        Playqueue::instance()->manager()->playlistAddMediaItems(list);
    }
}


void LocalScene::enqueuePlaylist(bool is_new_playqueue)
{
    if(!m_mouseGrabbedItem) return;
    PlaylistGraphicItem *item = static_cast<PlaylistGraphicItem*>(m_mouseGrabbedItem);

    QList<MEDIA::TrackPtr> list;

    // rechercher dynamique pour les smart playlists
    if(item->media->p_type == T_SMART)
    {
      list = SmartPlaylist::mediaItem( item->media->rules );
    }
    // playliste normale -> lecture dans le modele
    else {
      list = m_localPlaylistModel->getItemChildrenTracks(item->media);
    }

    if(!list.isEmpty()) {
        if(is_new_playqueue)
          Playqueue::instance()->clear();

        Playqueue::instance()->manager()->playlistAddMediaItems(list);
    }
}

void LocalScene::enqueueSelected(bool is_new_playqueue)
{
    //Debug::debug() << "LocalScene::enqueueSelected";
    QList<MEDIA::TrackPtr> tracks;

    if(is_new_playqueue)
      Playqueue::instance()->clear();

    QList<QGraphicsItem*> selected_items = sortedSelectedItem();

    foreach(QGraphicsItem* gi, selected_items)
    {
      if (gi->type() == GraphicsItem::AlbumType)
      {
        AlbumGraphicItem *item = static_cast<AlbumGraphicItem*>(gi);
        tracks << m_localTrackModel->getItemChildrenTracks(item->media);
      }
      else if (gi->type() == GraphicsItem::AlbumGenreType)
      {
        AlbumGenreGraphicItem *item = static_cast<AlbumGenreGraphicItem*>(gi);
        tracks << m_localTrackModel->getAlbumChildrenTracksGenre(item->media,item->_genre);
      }
      else if (gi->type() == GraphicsItem::ArtistType)
      {
        ArtistGraphicItem *item = static_cast<ArtistGraphicItem*>(gi);
        tracks << m_localTrackModel->getItemChildrenTracks(item->media);
      }
      else if (gi->type() == GraphicsItem::TrackType)
      {
        TrackGraphicItem *item = static_cast<TrackGraphicItem*>(gi);
        tracks << m_localTrackModel->getItemChildrenTracks(item->media);
      }
      else if (gi->type() == GraphicsItem::PlaylistType)
      {
        PlaylistGraphicItem *item = static_cast<PlaylistGraphicItem*>(gi);

        // rechercher dynamique pour les smart playlists
        if(item->media->p_type == T_SMART)
          tracks << SmartPlaylist::mediaItem( item->media->rules );
        // playliste normale -> lecture dans le modele
        else
          tracks << m_localPlaylistModel->getItemChildrenTracks(item->media);
      }
    } // end foreach

    if(!tracks.isEmpty()) {
        Playqueue::instance()->manager()->playlistAddMediaItems(tracks);
    }
}


/*
********************************************************************************
*        -> drag method                                                        *
********************************************************************************
*/
void LocalScene::startAlbumsDrag(QGraphicsItem* i /* = 0 */)
{
    if(!i) // multiple albums drags
    {
      MediaMimeData* mimedata = new MediaMimeData(SOURCE_COLLECTION);

      foreach(QGraphicsItem* gi, sortedSelectedItem())
      {
        AlbumGraphicItem *item = static_cast<AlbumGraphicItem *>(gi);
        mimedata->addTracks(m_localTrackModel->getItemChildrenTracks(item->media));
      }

      QDrag *drag = new QDrag(parentView());
      drag->setMimeData(mimedata);
      drag->exec();
    }
    else // single drag
    {
        AlbumGraphicItem *item = static_cast<AlbumGraphicItem *>(i);
        item->startDrag(parentView());
    }
}

void LocalScene::startAlbumsGenreDrag(QGraphicsItem* i /* = 0 */)
{
    if(!i) // multiple albums genre drags
    {
      MediaMimeData* mimedata = new MediaMimeData(SOURCE_COLLECTION);

      foreach(QGraphicsItem* gi, sortedSelectedItem())
      {
        AlbumGenreGraphicItem *item = static_cast<AlbumGenreGraphicItem *>(gi);
        mimedata->addTracks(m_localTrackModel->getAlbumChildrenTracksGenre(item->media,item->_genre));
      }

      QDrag *drag = new QDrag(parentView());
      drag->setMimeData(mimedata);
      drag->exec();
    }
    else // single drag
    {
        AlbumGenreGraphicItem *item = static_cast<AlbumGenreGraphicItem *>(i);
        item->startDrag(parentView());
    }
}

void LocalScene::startArtistsDrag(QGraphicsItem* i /* = 0 */)
{
    if(!i) // multiple artist drags
    {
      MediaMimeData* mimedata = new MediaMimeData(SOURCE_COLLECTION);

      foreach(QGraphicsItem* gi, sortedSelectedItem())
      {
        ArtistGraphicItem *item = static_cast<ArtistGraphicItem *>(gi);
        mimedata->addTracks(m_localTrackModel->getItemChildrenTracks(item->media));
      }

      QDrag *drag = new QDrag(parentView());
      drag->setMimeData(mimedata);
      drag->exec();
    }
    else // single drag
    {
        ArtistGraphicItem *item = static_cast<ArtistGraphicItem *>(i);
        item->startDrag(parentView());
    }
}


void LocalScene::startTracksDrag(QGraphicsItem* i)
{
    if(!i) // multiple tracks drags
    {
      MediaMimeData* mimedata = new MediaMimeData(SOURCE_COLLECTION);

      foreach(QGraphicsItem* gi, sortedSelectedItem())
      {
        TrackGraphicItem *item = static_cast<TrackGraphicItem *>(gi);
        mimedata->addTrack(item->media);
      }

      QDrag *drag = new QDrag(parentView());
      drag->setMimeData(mimedata);
      drag->exec();
    }
    else // single drag
    {
        TrackGraphicItem *item = static_cast<TrackGraphicItem *>(i);
        item->startDrag(parentView());
    }
}


void LocalScene::startPlaylistsDrag(QGraphicsItem* i)
{
    if(!i) // multiple tracks drags
    {
      MediaMimeData* mimedata = new MediaMimeData(SOURCE_COLLECTION);

      foreach(QGraphicsItem* gi, sortedSelectedItem())
      {
        PlaylistGraphicItem *item = static_cast<PlaylistGraphicItem *>(gi);

        // rechercher dynamique pour les smart playlists
        if(item->media->p_type == T_SMART)
          mimedata->addTracks( SmartPlaylist::mediaItem( item->media->rules ) );
        // playliste normale -> lecture dans le modele
        else
          mimedata->addTracks( m_localPlaylistModel->getItemChildrenTracks(item->media) );
      }

      QDrag *drag = new QDrag(parentView());
      drag->setMimeData(mimedata);
      drag->exec();
    }
    else // single drag
    {
        PlaylistGraphicItem *item = static_cast<PlaylistGraphicItem *>(i);
        item->startDrag(parentView());
    }
}


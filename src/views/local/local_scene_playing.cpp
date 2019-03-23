/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2018 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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
#include "playqueue/virtual_playqueue.h"
#include "models/local/local_track_model.h"
#include "models/local/local_playlist_model.h"
#include "models/local/histo_model.h"

#include "core/mediaitem/mediaitem.h"
#include "smartplaylist/smartplaylist.h"

#include "global_actions.h"

#include "debug.h"

#include <QtCore>

/*
********************************************************************************
*                                                                              *
*    Class LocalScene                                                          *
*        -> playing method                                                     *
********************************************************************************
*/
void LocalScene::playSceneContents(const QVariant& search)
{
//     Debug::debug() << "#############   [LocalScene] playSceneContents search variant " << search;
//     Debug::debug() << "#############   [LocalScene] playSceneContents search variant " << search.toString();
//     Debug::debug() << "#############   [LocalScene] playSceneContents search variant " << search.isValid();
  
    if( !ACTIONS()->value(APP_PLAY_ON_SEARCH)->isChecked() || !search.isValid() || 
       (!search.canConvert<MediaSearch>() && search.toString().isEmpty()) )
      return;
    
    Debug::debug() << "   [LocalScene] playSceneContents";
    QList<MEDIA::TrackPtr> tracks;

    if( mode() == VIEW::ViewArtist || mode() == VIEW::ViewAlbum || mode() == VIEW::ViewTrack ||
        mode() == VIEW::ViewGenre  || mode() == VIEW::ViewYear  || mode() == VIEW::ViewFavorite )
    {
        tracks = m_localTrackModel->getItemChildrenTracks(m_localTrackModel->rootItem());
    }
    else if(mode() == VIEW::ViewPlaylist)
    {
        tracks = m_localPlaylistModel->getItemChildrenTracks(m_localPlaylistModel->rootItem());
    }
    else if(mode() == VIEW::ViewHistory)
    {
        for ( int i = 0; i < m_histoModel->itemCount(); i++ ) {
          if(!m_histoModel->isMediaMatch( m_histoModel->trackAt(i) ))
            continue;
          tracks << m_histoModel->trackAt(i);
        }
    }

    if( !tracks.isEmpty() )
      VirtualPlayqueue::instance()->addTracksAndPlayAt(tracks, 0);
}



void LocalScene::playAlbum()
{
    Debug::debug() << "   [LocalScene] playAlbum";
    if(!m_mouseGrabbedItem) return;
    AlbumGraphicItem *item = static_cast<AlbumGraphicItem*>(m_mouseGrabbedItem);

    QList<MEDIA::TrackPtr> tracks = m_localTrackModel->getItemChildrenTracks(item->media);

    if(!tracks.isEmpty())
      VirtualPlayqueue::instance()->addTracksAndPlayAt(tracks, 0);
}

void LocalScene::playAlbumGenre()
{
    Debug::debug() << "   [LocalScene] playAlbumGenre";
    if(!m_mouseGrabbedItem) return;
    AlbumGenreGraphicItem *item = static_cast<AlbumGenreGraphicItem*>(m_mouseGrabbedItem);

    QList<MEDIA::TrackPtr> tracks = m_localTrackModel->getAlbumChildrenTracksGenre(item->media,item->_genre);

    if(!tracks.isEmpty())
      VirtualPlayqueue::instance()->addTracksAndPlayAt(tracks, 0);
}

void LocalScene::playArtist()
{
    Debug::debug() << "   [LocalScene] playArtist";
    if(!m_mouseGrabbedItem) return;
    ArtistGraphicItem *item = static_cast<ArtistGraphicItem*>(m_mouseGrabbedItem);

    QList<MEDIA::TrackPtr> tracks = m_localTrackModel->getItemChildrenTracks(item->media);

    if(!tracks.isEmpty())
      VirtualPlayqueue::instance()->addTracksAndPlayAt(tracks, 0);
}


void LocalScene::playTrack()
{
    Debug::debug() << "   [LocalScene] playTrack";
    if(!m_mouseGrabbedItem) return;

    QList<MEDIA::TrackPtr> tracks;
    /* ------ tracks from  TRACK VIEW  ------ */
    if(mode() == VIEW::ViewTrack)
    {
        tracks = m_localTrackModel->getItemChildrenTracks(m_localTrackModel->rootItem());
    }
    /* ------ tracks from  PLAYLIST VIEW  ------ */
    else if(mode() == VIEW::ViewPlaylist)
    {
        tracks = m_localPlaylistModel->getItemChildrenTracks(m_localPlaylistModel->rootItem());
    }
    /* ------ tracks from  HISTORY VIEW  ------ */
    else if(mode() == VIEW::ViewHistory)
    {
        for ( int i = 0; i < m_histoModel->itemCount(); i++ ) {
          if(!m_histoModel->isMediaMatch( m_histoModel->trackAt(i) ))
            continue;
          tracks << m_histoModel->trackAt(i);
        }
    }

    TrackGraphicItem *item = static_cast<TrackGraphicItem*>(m_mouseGrabbedItem);
    int index = tracks.indexOf(item->media);
    
    if( index != -1 && !tracks.isEmpty() )
      VirtualPlayqueue::instance()->addTracksAndPlayAt(tracks, index);
}

void LocalScene::playPlaylist()
{
    if(!m_mouseGrabbedItem) return;
    PlaylistGraphicItem *item = static_cast<PlaylistGraphicItem*>(m_mouseGrabbedItem);

    QList<MEDIA::TrackPtr> tracks;

    // rechercher dynamique pour les smart playlists
    if(item->media->p_type == T_SMART)
    {
      tracks = SmartPlaylist::mediaItem( item->media->rules );
    }
    // playliste normale -> lecture dans le modele
    else 
    {
      tracks = m_localPlaylistModel->getItemChildrenTracks(item->media);
    }

    if(!tracks.isEmpty())
      VirtualPlayqueue::instance()->addTracksAndPlayAt(tracks,0);
}

void LocalScene::playSelected()
{
    //Debug::debug() << "   [LocalScene] playSelected";

    QList<MEDIA::TrackPtr> tracks;

    QList<QGraphicsItem*> selected_items = sortedSelectedItem();

    foreach(QGraphicsItem* gi, selected_items)
    {
      if (gi->type() == GraphicsItem::AlbumType)
      {
        AlbumGraphicItem *item = static_cast<AlbumGraphicItem *>(gi);
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

    if(!tracks.isEmpty())
      VirtualPlayqueue::instance()->addTracksAndPlayAt(tracks,0);
}


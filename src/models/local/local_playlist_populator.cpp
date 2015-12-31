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

#include "local_playlist_populator.h"
#include "local_playlist_model.h"
#include "local_track_model.h"
#include "core/mediaitem/mediaitem.h"
#include "core/mediasearch/media_search.h"
#include "core/database/database.h"
#include "debug.h"

#include <QFileInfo>
#include <QSqlQuery>
#include <QVariant>
#include <QTime>
/*
********************************************************************************
*                                                                              *
*    Class LocalPlaylistPopulator                                              *
*                                                                              *
********************************************************************************
*/

LocalPlaylistPopulator::LocalPlaylistPopulator()
{
    setObjectName("LocalPlaylistPopulator");
    m_model    = LocalPlaylistModel::instance();
    m_exit     = false;
}

void LocalPlaylistPopulator::run()
{
     Debug::debug() << "  [LocalPlaylistPopulator] start " << QTime::currentTime().second() << ":" << QTime::currentTime().msec();
    /*-----------------------------------------------------------*/
    /* Get connection                                            */
    /* ----------------------------------------------------------*/
    if (!Database::instance()->open()) {
      emit populatingFinished();
      return;
    }

    /*-----------------------------------------------------------*/
    /* Get file count from database                              */
    /* ----------------------------------------------------------*/
    QSqlQuery queryCount("SELECT COUNT(*) FROM `playlist_items`",*Database::instance()->db());
    queryCount.next();

    int _progressMax = queryCount.value(0).toInt();
    int _progress    = 0;

    /*-----------------------------------------------------------*/
    /* Parse Database for normal playlist and populate model     */
    /* ----------------------------------------------------------*/
    m_model->clear();

    MEDIA::PlaylistPtr playlistItem = MEDIA::PlaylistPtr(0);

    QVariant playlist_id = -1;
    QVariant item_id     = -1;


    QSqlQuery query("SELECT playlist_id,playlist_filename,playlist_name,playlist_type,playlist_favorite,playlist_mtime, \
                            url,name,track_id \
                     FROM view_playlists \
                     ORDER BY playlist_type ASC",*Database::instance()->db());


    while (query.next() && !m_exit)
    {
        if( query.value(3).toInt() == T_PLAYQUEUE)
          break;
      
        if(  playlist_id != query.value(0) )
        {
          playlist_id = query.value(0);

          playlistItem = MEDIA::PlaylistPtr::staticCast( m_model->rootItem()->addChildren(TYPE_PLAYLIST) );

          playlistItem->id         =  query.value(0).toInt();
          playlistItem->url        =  query.value(1).toString();
          playlistItem->name       =  query.value(2).toString();
          playlistItem->icon       =  QString(":/images/media-playlist-110x110.png");
          playlistItem->p_type     =  (T_PLAYLIST)query.value(3).toInt();
          playlistItem->isFavorite =  query.value(4).toBool();
          playlistItem->date       =  (query.value(5).isNull() ? -1 : query.value(5).toInt());
          playlistItem->setParent(m_model->rootItem());
        }

        item_id = query.value(8);

        /* item is a track in collection  */
        if(!item_id.isNull())
        {
          if(LocalTrackModel::instance()->trackItemHash.contains(item_id.toInt()))
          {
            MEDIA::TrackPtr c_track = LocalTrackModel::instance()->trackItemHash.value(item_id.toInt());
    
            //!WARNING make a deep copy of shared data because we have to do a setParent
            c_track.detach();

            playlistItem->insertChildren(c_track);
            c_track->setParent(playlistItem);
          }
          else 
          {
            Debug::warning() << "failed to find " << query.value(7).toString() << "in collection";
          }
        }
        /* item is not in collection database */
        else
        {
            if(MEDIA::isLocal(query.value(6).toString()))
            {
              // too long
              MEDIA::TrackPtr track = MEDIA::FromLocalFile(query.value(6).toString());

              if(track.isNull()) {
                track = MEDIA::TrackPtr(new MEDIA::Track());
                track->id           = -1;
                track->url          = query.value(6).toString();
                track->title        = query.value(7).toString();

                //! default value
                track->isPlaying    =  false;
                track->isBroken     =  !QFile(track->url).exists();
                track->isPlayed     =  false;
                track->isStopAfter  =  false;
              }
              playlistItem->insertChildren(track);
              track->setParent(playlistItem);
            }
            else
            {
              MEDIA::TrackPtr stream = MEDIA::TrackPtr(new MEDIA::Track());
              stream->setType(TYPE_STREAM);
              stream->id          = -1;
              stream->url         = query.value(6).toString();
              stream->extra["station"] = query.value(7).toString();
              stream->isFavorite  = false;
              stream->isPlaying   = false;
              stream->isBroken    = false;
              stream->isPlayed    = false;
              stream->isStopAfter = false;
              //Debug::debug() << "  [LocalPlaylistPopulator] stream->url  " << stream->url;
              //Debug::debug() << "  [LocalPlaylistPopulator] stream->name  " << stream->name;

              playlistItem->insertChildren(stream);
              stream->setParent(playlistItem);
            }
        }

        _progress++;

        if( _progressMax != 0)  // when only smart playlist exist
          emit populatingProgress(  (int) ( (_progress*100)/_progressMax ) );
        //Debug::debug() << "  [LocalPlaylistPopulator] populatingProgress :" << percent << " %";

    } //! Fin while query


    /*-----------------------------------------------------------*/
    /* Parse Smart playlist and populate model                   */
    /* ----------------------------------------------------------*/
    QSqlQuery query_2("SELECT id,name,icon,rules,type,favorite FROM smart_playlists ORDER BY type ASC",*Database::instance()->db());

    while (query_2.next() && !m_exit) {
      playlistItem = MEDIA::PlaylistPtr::staticCast( m_model->rootItem()->addChildren(TYPE_PLAYLIST) );
      playlistItem->id         =  query_2.value(0).toInt();
      playlistItem->name       =  query_2.value(1).toString();
      playlistItem->icon       =  query_2.value(2).toString();
      playlistItem->rules      =  QVariant::fromValue<MediaSearch>( MediaSearch::fromDatabase (query_2.value(3)) );
      playlistItem->p_type     =  (T_PLAYLIST)query_2.value(4).toInt();
      playlistItem->isFavorite =  query_2.value(5).toBool();
      playlistItem->setParent(m_model->rootItem());
    }

    /*-----------------------------------------------------------*/
    /* End                                                       */
    /* ----------------------------------------------------------*/
    Debug::debug() << "  [LocalPlaylistPopulator] end " << QTime::currentTime().second() << ":" << QTime::currentTime().msec();
    
    if(!m_exit)
      emit populatingFinished();
}


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

#include "local_track_populator.h"
#include "local_track_model.h"
#include "core/mediaitem/mediaitem.h"

#include "core/database/database.h"
#include "settings.h"
#include "utilities.h"
#include "debug.h"

#include <QFileInfo>
#include <QSqlQuery>
#include <QVariant>

#include <QDateTime>
#include <QCryptographicHash>
/*
********************************************************************************
*                                                                              *
*    Class LocalTrackPopulator                                                 *
*                                                                              *
********************************************************************************
*/
LocalTrackPopulator::LocalTrackPopulator()
{
    m_model   = LocalTrackModel::instance();
    m_exit    = false;
}

void LocalTrackPopulator::run()
{
    Debug::debug() << " --- LocalTrackPopulator--> Start " << QTime::currentTime().second() << ":" << QTime::currentTime().msec();

    /*-----------------------------------------------------------*/
    /* Get connection                                            */
    /* ----------------------------------------------------------*/
    if (!Database::instance()->open()) {
        Debug::warning() << " --- LocalTrackPopulator--> db connect failed";
        emit populatingFinished();
        return;
    }

    /*-----------------------------------------------------------*/
    /* Get file count from database                              */
    /* ----------------------------------------------------------*/
    QSqlQuery queryCount("SELECT COUNT(*) FROM `view_tracks`",*Database::instance()->db());
    queryCount.next();
    if (queryCount.value(0).toInt() == 0) {
      emit populatingFinished();
      return;
    }

    //int _progressMax = queryCount.value(0).toInt();
    int _progress    = 0;

    /*-----------------------------------------------------------*/
    /* database option                                           */
    /* ----------------------------------------------------------*/
    m_isGrouping = Database::instance()->param()._option_group_albums;
    
    /*-----------------------------------------------------------*/
    /* Parse Database and populate model                         */
    /* ----------------------------------------------------------*/
    m_model->clear();

    MEDIA::ArtistPtr artistItem = MEDIA::ArtistPtr(0);
    MEDIA::AlbumPtr  albumItem  = MEDIA::AlbumPtr(0);
    MEDIA::TrackPtr  trackItem  = MEDIA::TrackPtr(0);

    QVariant artist_id = -1;
    QVariant album_id  = -1;
    QVariant track_id  = -1;
    QString prev_album_hash = QString();

    QSqlQuery query_1("SELECT artist_id,artist_name,artist_favorite,artist_playcount,artist_rating, \
                              album_id,album_name,album_year,album_favorite,album_playcount,album_rating,album_disc, \
                              id,trackname,filename,number,genre_name,length,albumgain,albumpeakgain,trackgain,trackpeakgain,last_played,playcount,rating \
                       FROM view_tracks \
                       ORDER BY artist_name COLLATE NOCASE ASC,album_year ASC,album_name ASC,album_disc ASC, number ASC",*Database::instance()->db());

    while (query_1.next() && !m_exit)
    {
      if(  artist_id != query_1.value(0) )
      {
        artist_id = query_1.value(0);

        artistItem = MEDIA::ArtistPtr::staticCast( m_model->rootItem()->addChildren(TYPE_ARTIST) );
        artistItem->id            =  query_1.value(0).toInt();
        artistItem->name          =  query_1.value(1).toString();
        artistItem->isFavorite    =  query_1.value(2).toBool();
        artistItem->playcount     =  query_1.value(3).toInt();
        artistItem->rating        =  query_1.value(4).toFloat();
        artistItem->isUserRating  =  (artistItem->rating != -1.0) ? true : false;
        artistItem->setParent(m_model->rootItem());
      }

      if(  album_id != query_1.value(5) ) 
      {
        album_id = query_1.value(5);


        bool is_new_album = true;
        int disc_number = query_1.value(12).toInt();

        if(disc_number != 0 && m_isGrouping) {
          QString album_hash = getAlbumHash(query_1.value(1).toString(),query_1.value(6).toString()); 

          if(album_hash == prev_album_hash) {
            is_new_album = false;  
            albumItem->disc_number++; // now it's disc count
            albumItem->ids << album_id.toInt();
          }

          prev_album_hash = album_hash;
        }

        if(is_new_album) {
          albumItem = MEDIA::AlbumPtr::staticCast( artistItem->addChildren(TYPE_ALBUM) );
          albumItem->id            =  query_1.value(5).toInt();
          albumItem->name          =  query_1.value(6).toString();
          albumItem->year          =  query_1.value(7).toInt();
          albumItem->isFavorite    =  query_1.value(8).toBool();
          albumItem->playcount     =  query_1.value(9).toInt();
          albumItem->rating        =  query_1.value(10).toFloat();
          albumItem->disc_number   =  query_1.value(11).toInt();
          albumItem->isUserRating  =  (albumItem->rating != -1.0) ? true : false;
          albumItem->setParent(artistItem);

          m_model->albumItemList.append(albumItem);

          if(m_isGrouping && albumItem->disc_number != 0) {
            albumItem->disc_number = 1;
            albumItem->ids << album_id.toInt();
          }
        }
      }

      if(  track_id != query_1.value(12) ) 
      {
        track_id = query_1.value(12);

        trackItem = MEDIA::TrackPtr::staticCast( albumItem->addChildren(TYPE_TRACK) );
        trackItem->id         =  query_1.value(12).toInt();
        trackItem->title      =  query_1.value(13).toString();
        trackItem->url        =  query_1.value(14).toString();
        trackItem->num        =  query_1.value(15).toUInt();
        trackItem->artist     =  artistItem->name;
        trackItem->album      =  albumItem->name;
        trackItem->year       =  albumItem->year;
        trackItem->genre      =  query_1.value(16).toString();
        trackItem->duration   =  query_1.value(17).toInt();
        /* can save a little memory if needed */
        if(SETTINGS()->_replaygain != 0) {
         trackItem->albumGain  =  query_1.value(18).toFloat();
         trackItem->albumPeak  =  query_1.value(19).toFloat();
         trackItem->trackGain  =  query_1.value(20).toFloat();
         trackItem->trackPeak  =  query_1.value(21).toFloat();
        }
        trackItem->lastPlayed =  !query_1.value(22).isNull() ? query_1.value(22).toInt() : -1;
        trackItem->playcount  =  query_1.value(23).toInt();
        trackItem->rating     =  query_1.value(24).toFloat();
        trackItem->disc_number = query_1.value(11).toInt();
        trackItem->setParent(albumItem);
        m_model->trackItemHash[trackItem->id] = trackItem;
        m_model->trackByGenre << trackItem;

        _progress++;
      }

      //emit populatingProgress(  (int) ( (_progress*100)/_progressMax ) );
      //Debug::debug() << " --- LocalTrackPopulator -->populatingProgress :" <<  (int) ( (_progress*100)/_progressMax ) << " %";

    } // end while

    
    /*-----------------------------------------------------------*/
    /* Calculate auto rating                                     */
    /* ----------------------------------------------------------*/
    for ( int i = 0; i < m_model->rootItem()->childCount(); i++ )
    {
      float artist_rating = 0.0;
      MEDIA::ArtistPtr artist_media = MEDIA::ArtistPtr::staticCast(m_model->rootItem()->child(i));

      for (int j = 0; j < artist_media->childCount(); j++)
      {
         MEDIA::AlbumPtr album_media = MEDIA::AlbumPtr::staticCast( artist_media->child(j) );

         /* WARNING not ok for multiset album */
         if(!album_media->isUserRating)
           album_media->rating = m_model->getItemAutoRating(album_media);
         //else keep user rating from database

         artist_rating += album_media->rating;
      }

      if(!artist_media->isUserRating) {
        float result =  artist_media->childCount()!=0 ? float(artist_rating/(artist_media->childCount())) : 0 ;
        artist_media->rating = double(int(result * 5 * 2 + 0.5)) / (5 * 2);
      }
    }

    
    //! Sort Media Track Item list By Genre
    qSort(m_model->trackByGenre.begin(), m_model->trackByGenre.end(),MEDIA::compareTrackItemGenre);

           
    /*-----------------------------------------------------------*/
    /* End                                                       */
    /* ----------------------------------------------------------*/
    Debug::debug() << " --- LocalTrackPopulator--> End " << QTime::currentTime().second() << ":" << QTime::currentTime().msec();
    
    if(!m_exit)
      emit populatingFinished();
}


QString LocalTrackPopulator::getAlbumHash(const QString & artist, const QString& album)
{
    if( (!artist.isEmpty()) && (!album.isEmpty()) )
    {
      QCryptographicHash hash(QCryptographicHash::Sha1);
      hash.addData(artist.toUtf8().constData());
      hash.addData(album.toUtf8().constData());

      return QString(hash.result().toHex());
    }

    return QString();
}


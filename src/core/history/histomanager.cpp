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

#include "histomanager.h"
#include "core/database/database.h"
#include "core/player/engine.h"
#include "core/mediaitem/mediaitem.h"
#include "models/local/local_track_model.h"

#include "settings.h"
#include "debug.h"

#include <QDateTime>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QVariant>


HistoManager* HistoManager::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class HistoManager                                                        *
*                                                                              *
********************************************************************************
*/
HistoManager::HistoManager()
{
    INSTANCE  = this;

    //! set player trigger
    m_player = Engine::instance();
    m_timer = new QTimer();

    connect(m_player, SIGNAL(mediaChanged()), this, SLOT(addEntry()));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(addToDatabase()));

    //! check database histo table
    checkHisto();
}

/* ---------------------------------------------------------------------------*/
/* HistoManager::reloadSettings                                               */
/* ---------------------------------------------------------------------------*/
void HistoManager::reloadSettings()
{
    //Debug::debug() << "  [HistoManager] reloadSettings";
}


/* ---------------------------------------------------------------------------*/
/* HistoManager::addEntry                                                     */
/* ---------------------------------------------------------------------------*/
void HistoManager::addEntry()
{
    m_timer->stop();

    if(!m_player->playingTrack()) return;

    m_timer->start(20000);
}

/* ---------------------------------------------------------------------------*/
/* HistoManager::addToDatabase                                                */
/* ---------------------------------------------------------------------------*/
void HistoManager::addToDatabase()
{
    m_timer->stop();

    if(Engine::instance()->state() != ENGINE::PLAYING) {
       m_timer->stop();
       return;
    }

    MEDIA::TrackPtr media   =  m_player->playingTrack();
    int       now_date      =  QDateTime::currentDateTime().toSecsSinceEpoch();

    QString   engine_url    =  media->url;
    if(engine_url.isEmpty())
      return;

    QString   media_name;
    if(media->type() == TYPE_TRACK)
      media_name = media->artist + " - " + media->album + " - " + media->title;
    else
      media_name = media->extra["station"].toString();

    if (!Database::instance()->open()) return;

    QSqlQuery("BEGIN TRANSACTION;",*Database::instance()->db());

    //---------------------------------------
    //    add or update entry in history
    //---------------------------------------
    QSqlQuery q("", *Database::instance()->db());

    if(SETTINGS()->_useHistory)
    {
        q.prepare("SELECT `id`,`url` FROM `histo` WHERE `url`=:val;");
        q.bindValue(":val", engine_url );
        q.exec();

        if ( !q.next() ) {
            Debug::debug() << "      [Histo] add a new entry" << engine_url;

            q.prepare("INSERT INTO `histo`(`url`,`name`,`date`) VALUES (:u,:n,:d);");
            q.bindValue(":u", engine_url);
            q.bindValue(":n", media_name);
            q.bindValue(":d", now_date);
            q.exec();

            if(q.numRowsAffected() < 1)
                Debug::warning() << "[Histo] error adding entry !! ";

            QSqlQuery query("DELETE FROM `histo` WHERE `id` <= (SELECT MAX(`id`) FROM `histo`) - 2000;", *Database::instance()->db());
        }
        else
        {
            Debug::debug() << "      [Histo] update an existing entry" << engine_url;
            int histo_id = q.value(0).toString().toInt();

            q.prepare("UPDATE `histo` SET `date`=:d WHERE `id`=:id;");
            q.bindValue(":d", now_date);
            q.bindValue(":id", histo_id);
            q.exec();
        }
    }

    //---------------------------------------
    //    update playcount
    //---------------------------------------
    q.prepare("SELECT `id`,`artist_id`,`album_id` FROM `view_tracks` WHERE `filename`=:val LIMIT 1;");
    q.bindValue(":val", engine_url );
    q.exec();

    if (q.next())
    {
      //Debug::debug() << "      [Histo] update playcount!";

      const int trackId  = q.value(0).toInt();
      const int artistId = q.value(1).toInt();
      const int albumId  = q.value(2).toInt();

      QSqlQuery query1("UPDATE `tracks` " \
                       "SET `playcount`=`playcount`+1 " \
                       "WHERE `id`="+QString::number(trackId)+";", *Database::instance()->db());

      QSqlQuery query2("UPDATE `albums` " \
                       "SET `playcount`=`playcount`+1 " \
                       "WHERE `id`="+QString::number(albumId)+";", *Database::instance()->db());

      QSqlQuery query3("UPDATE `artists` " \
                       "SET `playcount`=`playcount`+1 " \
                       "WHERE `id`="+QString::number(artistId)+";", *Database::instance()->db());

      /* update collection model item */
      MEDIA::TrackPtr track = MEDIA::TrackPtr(
               LocalTrackModel::instance()->trackItemHash.value(trackId)
               );

      if(!track.isNull()) {
        track->playcount++;
        track->lastPlayed = now_date;

        MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(track->parent());
        album->playcount++;

        MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(album->parent());
        artist->playcount++;
      }
    }


    QSqlQuery("COMMIT TRANSACTION;",*Database::instance()->db());
}


/* ---------------------------------------------------------------------------*/
/* HistoManager::checkHisto                                                   */
/*      -> At startup fix de limit of the size of the histo table             */
/* ---------------------------------------------------------------------------*/
void HistoManager::checkHisto()
{
    if (!Database::instance()->open()) return;

    QSqlQuery query("SELECT MAX(`id`) FROM `histo`;", *Database::instance()->db());
    query.next();

    if (query.isValid()) {
      const int maxId = query.value(0).toInt();
      //Debug::debug() << "      [Histo] check table max id" << maxId;
      if(maxId >= 100000) {
        QSqlQuery query("UPDATE `histo` SET `id` = `id` - (SELECT MIN(`id`) -1 FROM `histo`) ;", *Database::instance()->db());
        query.exec();
      }
    }
}

/* ---------------------------------------------------------------------------*/
/* HistoManager::clearHistory                                                 */
/* ---------------------------------------------------------------------------*/
void HistoManager::clearHistory()
{
    if (!Database::instance()->open()) return;

    QSqlQuery query("DELETE FROM `histo`;", *Database::instance()->db());
    query.exec();
}


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

#include "playlistpopulator.h"
#include "playqueue_model.h"

#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/playlist_parser.h"

#include "core/database/database.h"
#include "models/local/local_track_model.h"

#include "views/stream/stream_loader.h"

#include "networkaccess.h"

#include "settings.h"
#include "utilities.h"
#include "debug.h"

//Qt
#include <QDataStream>
#include <QFile>
#include <QByteArray>
#include <QDirIterator>
#include <QMutexLocker>
#include <QFileInfo>
#include <QSqlQuery>

/*
********************************************************************************
*                                                                              *
*    Class PlaylistPopulator                                                   *
*                                                                              *
********************************************************************************
*/
PlaylistPopulator::PlaylistPopulator()
{
    setObjectName("PlaylistPopulator");

    m_files.clear();
    m_tracks.clear();
    m_isRunning      = false;
    m_playlist_row   = -1;
}

/*******************************************************************************
  PlaylistPopulator::run
*******************************************************************************/
void PlaylistPopulator::run()
{
  Debug::debug() << "  [PlaylistPopulator] Start "  << QTime::currentTime().second() << ":" << QTime::currentTime().msec();

  m_isRunning = true;
    
  /*-----------------------------------------------------------*/
  /* Get connection                                            */
  /* ----------------------------------------------------------*/
  if (!Database::instance()->open()) {
        Debug::warning() << "  [PlaylistPopulator] db connect failed";
        return;
  }
    
  while (!m_files.isEmpty() || !m_tracks.isEmpty())
  {
      /*--------------------------------------------------*/
      /* cas des dossiers                                 */
      /* -------------------------------------------------*/
      if (m_files.size() > 0) {
        if (QFileInfo(m_files.first()).isDir()) {
          load_dir(m_files.takeFirst(),m_playlist_row);
        }
      }

      /*--------------------------------------------------*/
      /* cas des playlists remote                         */
      /* -------------------------------------------------*/
      if (m_files.size() > 0) {
        if( !MEDIA::isLocal(m_files.first()) && !MEDIA::isMediaPlayable(m_files.first()))
        {
            //! remote playlist --> download, make it local
            Debug::debug() << "  [PlaylistPopulator] download remote playlist" << m_files.first();

            QString url = m_files.takeFirst();
            MEDIA::TrackPtr stream = MEDIA::TrackPtr(new MEDIA::Track());
            stream->setType(TYPE_STREAM);
            stream->id          = -1;
            stream->url         = url;
            stream->name        = url;
      
            emit async_load(stream, m_playlist_row);
        }
      }

      /*--------------------------------------------------*/
      /* cas des playlists locale                         */
      /* -------------------------------------------------*/
      if (m_files.size() > 0) {
      //! local playlist
        if( MEDIA::isLocal(m_files.first()) && MEDIA::isPlaylistFile(m_files.first())) {
              //! local playlist
              //Debug::debug() << "  [PlaylistPopulator] PlaylistFromFile local " << m_files.first();
              QList<MEDIA::TrackPtr> list = MEDIA::PlaylistFromFile(m_files.takeFirst());

              foreach (MEDIA::TrackPtr track, list) 
              {
                if(track->type() == TYPE_TRACK) {
                    QString url = track->url;
                    track.reset();
                    track = MEDIA::FromDataBase(url);
                    if(!track)
                      track = MEDIA::FromLocalFile(url);

                    m_model->request_insert_track(track, m_playlist_row);

                    track.reset();
                }
                else {
                    m_model->request_insert_track(track, m_playlist_row);
                    track.reset();
                }
             } // foreach MediaItem
          }
      }

      /*--------------------------------------------------*/
      /* cas des fichiers et urls                         */
      /* -------------------------------------------------*/
      if (m_files.size() > 0) {
          if(  MEDIA::isLocal(m_files.first()) &&
               MEDIA::isAudioFile(m_files.first()) &&
              !MEDIA::isPlaylistFile(m_files.first()))
          {
              QString fileName = QFileInfo(m_files.takeFirst()).canonicalFilePath();

              MEDIA::TrackPtr track = MEDIA::FromDataBase(fileName);
              if(!track)
                track = MEDIA::FromLocalFile(fileName);

              m_model->request_insert_track(track, m_playlist_row);
              track.reset();
          }
          else if(!MEDIA::isLocal(m_files.first())) {
              //! remote file
              Debug::debug() << "remote uRL" << m_files.first();
              QString url = m_files.takeFirst();
              MEDIA::TrackPtr stream = MEDIA::TrackPtr(new MEDIA::Track());
              stream->setType(TYPE_STREAM);
              stream->id          = -1;
              stream->url         = url;
              stream->name        = url;
              stream->title       = QString();
              stream->artist      = QString();
              stream->album       = QString();
              stream->categorie   = QString();
              stream->isFavorite  = false;
              stream->isPlaying   = false;
              stream->isBroken    = false;
              stream->isPlayed    = false;
              stream->isStopAfter = false;

              m_model->request_insert_track(stream, m_playlist_row);

              stream.reset();
          }
          else 
          {
              //!WARNING on doit sortir les elements non traites (si boucle while)
              Debug::warning() << "  [PlaylistPopulator] unsupported media !" << m_files.takeFirst();
          }
      }

      
      /*--------------------------------------------------*/
      /* cas des mediaitems                               */
      /* -------------------------------------------------*/
      if(m_tracks.size() > 0) 
      {
        //Debug::debug() << "  [PlaylistPopulator] m_tracks.size() :" << m_tracks.size();
        
        MEDIA::TrackPtr track = m_tracks.takeFirst();

        if( !MEDIA::isMediaPlayable(track->url) )
        {
            emit async_load(track, m_playlist_row);
        }
        else 
        {
            m_model->request_insert_track(track, m_playlist_row++);
        }
      }

  } //! END !m_files.isEmpty() && !m_tracks.isEmpty()
    
  if(SETTINGS()->_playqueueDuplicate == false)
    m_model->removeDuplicate();

  m_isRunning = false;
  emit playlistPopulated();

  //Debug::debug() << "  [PlaylistPopulator] Start "  << startTime.second() << ":" << startTime.msec();
  Debug::debug() << "  [PlaylistPopulator] End "  << QTime::currentTime().second() << ":" << QTime::currentTime().msec();
}

/*******************************************************************************
  load_dir
*******************************************************************************/
void PlaylistPopulator::load_dir(const QString& path, int row)
{
    Debug::debug() << "PlaylistPopulator::load_dir";
    
    const QStringList dirFilter  = QStringList() << "*.mp3" << "*.ogg" << "*.flac" << "*.wav" << "*.m4a" << "*.aac" << "*.ape";
    QDirIterator dirIterator(path, dirFilter ,QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

    /* --- read directoy files (with subfolders) --- */
    QStringList files;
    while(dirIterator.hasNext()) {
      files << dirIterator.next();
    }
    
    /* --- get media item from files --- */
    QList<MEDIA::TrackPtr> list;
    foreach (const QString & file, files)
    {
        const QString filename = QFileInfo(file).canonicalFilePath();

        MEDIA::TrackPtr track = MEDIA::FromDataBase( filename );
        if(!track)
          track = MEDIA::FromLocalFile( filename );

        list << track;
    }
      
    /* --- sort list --- */   
    qSort(list.begin(), list.end(),MEDIA::compareTrackNatural);
    
    /* --- add to playqueue --- */
    int insert_row = row;
    foreach(MEDIA::MediaPtr track, list) 
    {
        m_model->request_insert_track(track, insert_row);
        if(insert_row != -1 )
          insert_row++;
    }
}

/*******************************************************************************
  User methode
*******************************************************************************/
/*
  addFile  : add one file to proceed
  addFiles : add files
  addUrls  : add Urls
*/

void PlaylistPopulator::addFile(const QString &file)
{
    //Debug::debug() << "  [PlaylistPopulator] append file :" << file;
    m_playlist_row = -1;
    m_files.append(file);
}

void PlaylistPopulator::addFiles(const QStringList &files)
{
    //Debug::debug() << "  [PlaylistPopulator] append files :" << files;
    m_playlist_row = -1;
    m_files.append(files);
}

void PlaylistPopulator::addUrls(QList<QUrl> listUrl, int playlist_row)
{
    //Debug::debug() << "  [PlaylistPopulator] addUrls";
  
    m_playlist_row = playlist_row;

    foreach (const QUrl &url, listUrl)
    {
      if(MEDIA::isLocal(url.toString()))
        m_files.append(QFileInfo(url.toLocalFile()).canonicalFilePath());
      else
        m_files.append(url.toString());
    }
}

void PlaylistPopulator::addMediaItems(QList<MEDIA::TrackPtr> list, int playlist_row)
{
    Debug::debug() << "  [PlaylistPopulator] addMediaItems " << list;
    m_playlist_row = playlist_row;
    m_tracks.append(list);
}


void PlaylistPopulator::restoreSession()
{
    Debug::debug() << "  [PlaylistPopulator] restoreSession ";
    
    if (!Database::instance()->open())
      return;

    QSqlQuery query(*Database::instance()->db());
    query.prepare("SELECT url,name,track_id FROM view_playlists WHERE playlist_type=?");
    query.addBindValue((int) T_PLAYQUEUE);
    query.exec();

    while (query.next())
    {
        QVariant track_id = query.value(2);
        
        /* item is a track in collection  */
        if(!track_id.isNull())
        {
          if(LocalTrackModel::instance()->trackItemHash.contains(track_id.toInt()))
          {
            m_model->request_insert_track(
              LocalTrackModel::instance()->trackItemHash.value(track_id.toInt())
            );
          }
        }
        /* item is not in collection database */
        else
        {
            if(MEDIA::isLocal(query.value(0).toString()))
            {
              // too long
              MEDIA::TrackPtr track = MEDIA::FromLocalFile(query.value(0).toString());

              if(track.isNull()) {
                track = MEDIA::TrackPtr(new MEDIA::Track());
                track->id           = -1;
                track->url          = query.value(0).toString();
                track->name         = query.value(1).toString();
                track->title        = query.value(1).toString();

                //! default value
                track->isPlaying    =  false;
                track->isBroken     =  !QFile(track->url).exists();
                track->isPlayed     =  false;
                track->isStopAfter  =  false;
              }
              
              m_model->request_insert_track( track);
            }
            else
            {
              MEDIA::TrackPtr stream = MEDIA::TrackPtr(new MEDIA::Track());
              stream->setType(TYPE_STREAM);
              stream->id          = -1;
              stream->url         = query.value(0).toString();
              stream->name        = query.value(1).toString();
              stream->isFavorite  = false;
              stream->isPlaying   = false;
              stream->isBroken    = false;
              stream->isPlayed    = false;
              stream->isStopAfter = false;
              m_model->request_insert_track(stream);
            }
        }
    }
}


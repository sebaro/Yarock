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

#include "stream_loader.h"
#include "networkaccess.h"
#include "playlist_parser.h"
#include "debug.h"

const int Loader_Timeout = 7500;

/*
********************************************************************************
*                                                                              *
*    Class StreamLoader                                                        *
*                                                                              *
********************************************************************************
*/
StreamLoader::StreamLoader(MEDIA::TrackPtr parent)
{
    m_parent  = parent;
    m_parent->isBroken     = false;
    
    m_timeout_timer = new QTimer(this);
    m_timeout_timer->setSingleShot(true);
    connect(m_timeout_timer, SIGNAL(timeout()), SLOT(slot_download_error()));
}

void StreamLoader::start_asynchronous_download(const QString& url)
{
    Debug::debug() << "StreamLoader::start_asyncronous_download";
   
    QString _url = url.isEmpty() ? m_parent->url : url;
    
    QObject *reply = HTTP()->get(QUrl(_url));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_download_error()));
    connect(reply, SIGNAL(data(QByteArray)), SLOT(slot_download_done(QByteArray)));
    
    m_timeout_timer->start(Loader_Timeout);
}


void StreamLoader::slot_download_done(QByteArray bytes)
{
    Debug::debug() << "StreamLoader::slot_download_done";
    m_timeout_timer->stop();

    if(bytes.isEmpty())
    {
      Debug::warning() << "StreamLoader::received empty bytes";
      emit download_done(m_parent);
      return;
    }

    /* read playlist from received byte */
    QList<MEDIA::TrackPtr> list = MEDIA::PlaylistFromBytes(bytes);

    foreach (MEDIA::TrackPtr track, list)
    {
      /*-------------------------------*/
      /* TYPE TRACK                    */
      /* ------------------------------*/      
      if(track->type() == TYPE_TRACK) 
      {
        MEDIA::TrackPtr track = MEDIA::FromDataBase(track->url);
        if(!track)
          track = MEDIA::FromLocalFile(track->url);

        m_parent->insertChildren(track);
        track->setParent(m_parent);
      }
      /*-------------------------------*/
      /* TYPE STREAM                   */
      /* ------------------------------*/
      else
      {
        /* get parent informations */
        if( track->extra["station"].toString().isEmpty() )
          track->extra["station"] = m_parent->extra["station"];

        if( track->extra["website"].toString().isEmpty() )
          track->extra["website"] = m_parent->extra["website"];
          
        if( track->genre.isEmpty() )
          track->genre = m_parent->genre;
        
        /* remote playlist found in playlist WARNING (can be recursive) */
        if( !MEDIA::isMediaPlayable(track->url) )
        {
          Debug::debug() << "StreamLoader remote playlist found :" << track->url;
          StreamLoader* loader = new StreamLoader(m_parent);
          connect(loader, SIGNAL(download_done(MEDIA::TrackPtr)), this, SLOT(slot_pending_task_done()));
          m_pending_task << loader;
          loader->start_asynchronous_download(track->url);
        }
        else 
        {
          Debug::debug() << "StreamLoader track found :" << track->url;
          insertTrackToParent(track);
        }
      }
      track.reset();
    
    } // foreach track
    
    if(m_pending_task.isEmpty())      
      emit download_done(m_parent);
}

void StreamLoader::slot_download_error()
{
    Debug::debug() << "StreamLoader::slot_download_error";
    m_timeout_timer->stop();
    m_parent->isBroken = true;
    
    emit download_done(m_parent);
}

void StreamLoader::insertTrackToParent(MEDIA::TrackPtr track)
{
    QSet<QString> urls;
    
    for (int i=0; i < m_parent->childCount(); i++)
    {
        QString url = static_cast<MEDIA::TrackPtr>(m_parent->child(i))->url;
        
        if( !urls.contains( url ) )
           urls.insert( url );
    }    
    
    if( !urls.contains( track->url ) )
    {
      m_parent->insertChildren(track);
      track->setParent(m_parent);
    }
}


void StreamLoader::slot_pending_task_done()
{
    Debug::debug() << "StreamLoader slot_pending_task_done";
    StreamLoader* loader = qobject_cast<StreamLoader*>(sender());
    int idx = m_pending_task.indexOf(loader);
    if(idx != -1) {
      m_pending_task.removeAt(idx);
    }

    /* emit dowload DONE */
    if(m_pending_task.isEmpty())
      emit download_done(m_parent);
}


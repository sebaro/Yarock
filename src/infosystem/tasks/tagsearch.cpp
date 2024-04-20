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
#include "tagsearch.h"

#include "info_system.h"

#include "core/database/database.h"
#include "core/mediaitem/mediaitem.h"

#include "settings.h"
#include "utilities.h"
#include "debug.h"

#include <QSqlQuery>
#include <QVariant>

#include <QFile>
#include <QImage>

/*
********************************************************************************
*                                                                              *
*    Class TagSearch                                                           *
*                                                                              *
********************************************************************************
*/

TagSearch::TagSearch(QObject *parent)
{
Q_UNUSED(parent)

    setObjectName("tagsearch");

    //! init
    m_exit                = false;
    m_isRunning           = false;
    
    m_timeout.setInterval( 10000 );
    m_timeout.setSingleShot( false );
    connect( &m_timeout, SIGNAL( timeout() ), SLOT( slot_request_timeout() ) );    
}

TagSearch::~TagSearch()
{
   m_timeout.stop();
   m_isRunning       = false;
}


/* ---------------------------------------------------------------------------*/
/* TagSearch::run                                                             */
/* ---------------------------------------------------------------------------*/
void TagSearch::run()
{
    Debug::debug() << Q_FUNC_INFO << m_type;
    
    m_isRunning = true;
    m_max = m_requests.size();

    process_search();

    /* Start thread event loop --> makes signals and slots work */
    QEventLoop eventLoop;
    eventLoop.exec();
    
    m_requests.clear();
}

void TagSearch::setSearch(TYPE type, INFO::InfoRequestData request)
{
    m_type = type;
    
    /* prepare request to process */
    m_requests.clear();
    switch (m_type)
    {
       case ALBUM_COVER_SINGLE:
       case ALBUM_GENRE_SINGLE:  
       case ARTIST_IMAGE_SINGLE:
         m_requests.insert(request.requestId, request);
       break;  
             
       case ALBUM_COVER_FULL : set_requests_cover_search(); break;
       case ALBUM_GENRE_FULL : set_requests_genre_search(); break;
       case ARTIST_IMAGE_FULL: set_requests_artist_search(); break;
    
       case ARTIST_ALBUM_FULL:
         set_requests_artist_search();
         set_requests_cover_search(); 
         break;
       default:break;
    }
}

/* ---------------------------------------------------------------------------*/
/* TagSearch::start                                                           */
/* ---------------------------------------------------------------------------*/
// void TagSearch::start(TYPE type, INFO::InfoRequestData request)
// {
//     m_type = type;
//     Debug::debug() << Q_FUNC_INFO << m_type;
//     
//     /* prepare request to process */
//     m_requests.clear();
//     switch (m_type)
//     {
//        case ALBUM_COVER_SINGLE:
//        case ALBUM_GENRE_SINGLE:  
//        case ARTIST_IMAGE_SINGLE:
//          m_requests.insert(request.requestId, request);
//        break;  
//              
//        case ALBUM_COVER_FULL : set_requests_cover_search(); break;
//        case ALBUM_GENRE_FULL : set_requests_genre_search(); break;
//        case ARTIST_IMAGE_FULL: set_requests_artist_search(); break;
//     
//        case ARTIST_ALBUM_FULL:
//          set_requests_artist_search();
//          set_requests_cover_search(); 
//          break;
//        default:break;
//     }
//     
//     m_isRunning = true;
//     m_max = m_requests.size();
// 
//     process_search();
// 
//     /* Start thread event loop --> makes signals and slots work */
//     QEventLoop eventLoop;
//     eventLoop.exec();
//     
//     m_requests.clear();
// }

/* ---------------------------------------------------------------------------*/
/* TagSearch::finish_search                                                   */
/* ---------------------------------------------------------------------------*/
void TagSearch::finish_search()
{
    Debug::debug() << Q_FUNC_INFO;
    m_timeout.stop();
    m_isRunning       = false;
    emit finished();
}

/* ---------------------------------------------------------------------------*/
/* TagSearch::slot_request_timeout                                            */
/* ---------------------------------------------------------------------------*/
void TagSearch::slot_request_timeout()
{
    if(!m_requests.isEmpty())
      m_requests.take(m_requests.keys().first());
    
    process_search();
}

/* ---------------------------------------------------------------------------*/
/* TagSearch::process_search                                                  */
/* ---------------------------------------------------------------------------*/
void TagSearch::process_search()
{
    Debug::debug() << Q_FUNC_INFO << " request size:" << m_requests.size();

    if(!m_requests.isEmpty() && !m_exit) 
    {
      emit progress ( ((m_max - m_requests.values().size())*100)/m_max );
      
      INFO::InfoRequestData request = m_requests.value(m_requests.keys().first());
    
      connect( InfoSystem::instance(),
             SIGNAL( info( INFO::InfoRequestData, QVariant ) ),
             SLOT( slot_system_info( INFO::InfoRequestData, QVariant ) ),  Qt::UniqueConnection );

      InfoSystem::instance()->getInfo( request );
      Debug::debug() << Q_FUNC_INFO << " start search for request.id:" << request.requestId;
      
      /* timer is needed as we dont have info signal from InfoSystem each time */
      m_timeout.start();
    }
    else 
    {
       m_timeout.stop();
       finish_search();
    }
}

/* ---------------------------------------------------------------------------*/
/* TagSearch::slot_system_info                                                */
/* ---------------------------------------------------------------------------*/
void TagSearch::slot_system_info( INFO::InfoRequestData request, QVariant output)
{
    m_timeout.stop();
    Debug::debug() << Q_FUNC_INFO << "get info for request.id " << request.requestId;
 
    if(!m_requests.contains(request.requestId)) {
       m_timeout.start();
      return;
    }

    m_requests.take(request.requestId);

    if(output.isNull()) {
      process_search();
      return;
    }
    
    /* get request info */
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();

    switch (request.type )
    {
      case INFO::InfoAlbumCoverArt : handle_cover_search_result(hash,output);break;
      case INFO::InfoAlbumGenre    : handle_genre_search_result(hash,output);break;
      case INFO::InfoArtistImages  : handle_artist_search_result(hash,output);break;
      default:break; 
    }
    
    process_search();
}

/* ---------------------------------------------------------------------------*/
/* TagSearch::                                                                */
/* ---------------------------------------------------------------------------*/
void TagSearch::set_requests_cover_search()
{
    if (!Database::instance()->open())  return;

    //! albums database loop
    QSqlQuery query("SELECT name,artist_name FROM view_albums",*Database::instance()->db());
    
    while (query.next())
    {
        INFO::InfoStringHash hash;
        hash["artist"]     = query.value(1).toString();
        hash["album"]      = query.value(0).toString();
        hash["covername"]  = MEDIA::coverHash(hash["artist"], hash["album"]);

        const QString path = UTIL::CONFIGDIR + "/albums/" + hash["covername"];
        if(QFile::exists(path))
          continue;

        INFO::InfoRequestData request = INFO::InfoRequestData(INFO::InfoAlbumCoverArt, hash);
          
        m_requests.insert(request.requestId, request); 
    }
}

void TagSearch::set_requests_genre_search()
{
    if (!Database::instance()->open())  return;

    //! albums database loop
    QSqlQuery query("SELECT name,artist_name FROM view_albums",*Database::instance()->db());
    
    while (query.next())
    {
        INFO::InfoStringHash hash;
        hash["artist"] = query.value(1).toString();
        hash["album"]  = query.value(0).toString();

        INFO::InfoRequestData request = INFO::InfoRequestData(INFO::InfoAlbumGenre, hash);
          
        m_requests.insert(request.requestId, request);          
    }
}

void TagSearch::set_requests_artist_search()
{
    if (!Database::instance()->open())  return;

    //! artist database loop
    QSqlQuery query("SELECT name FROM artists",*Database::instance()->db());
    
    while (query.next())
    {
        INFO::InfoStringHash hash;
        hash["artist"]     = query.value(0).toString();

        const QString path = UTIL::CONFIGDIR + "/artists/" + MEDIA::artistHash( hash["artist"] );
        if(QFile::exists(path))
          continue;

        INFO::InfoRequestData request = INFO::InfoRequestData(INFO::InfoArtistImages, hash);
        m_requests.insert(request.requestId, request);
    }
}

/* ---------------------------------------------------------------------------*/
/* TagSearch::handle_cover_search_result                                      */
/* ---------------------------------------------------------------------------*/
void TagSearch::handle_cover_search_result(INFO::InfoStringHash hash, QVariant output)
{
    /* filepath for cover file */
    const QString filePath = UTIL::CONFIGDIR + "/albums/" + hash["covername"];

    QFile file(filePath);
    if(file.exists())
    {
        Debug::debug() << "       [TagSearch] file exists, start next cover search";
    }
    else if (!file.open(QIODevice::WriteOnly))
    {
        Debug::debug() << "       [TagSearch] error writing album image";
    }
    else
    {
        const QByteArray bytes = output.toByteArray();

        QImage image = QImage::fromData(bytes);
        image = image.scaled(QSize(SETTINGS()->_coverSize, SETTINGS()->_coverSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        image.save(filePath, "png", -1);
    }  
}

/* ---------------------------------------------------------------------------*/
/* TagSearch::handle_genre_search_result                                      */
/* ---------------------------------------------------------------------------*/
void TagSearch::handle_genre_search_result(INFO::InfoStringHash hash, QVariant output)
{
    QVariantMap map = output.toMap();
    Debug::debug() << "  artist:" << hash["artist"] 
                   << "  album:" << hash["album"]
                   << "  genre:" << map["genre"].toStringList();  
}
 
/* ---------------------------------------------------------------------------*/
/* TagSearch::handle_artist_search_result                                     */
/* ---------------------------------------------------------------------------*/
void TagSearch::handle_artist_search_result(INFO::InfoStringHash hash, QVariant output)
{
    Debug::debug() << "       [TagSearch] handle_artist_search_result";
  
    const QString filePath = UTIL::CONFIGDIR + "/artists/" + MEDIA::artistHash( hash["artist"] );
     
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        Debug::debug() << "       [TagSearch] error writing artist image";
    }
    else
    {
        /* get data */  
        QImage image = UTIL::artistImageFromByteArray( output.toByteArray(), SETTINGS()->_coverSize);
    
        if( image.isNull() )
            return;

        image.save(filePath, "png", -1);
    }
}


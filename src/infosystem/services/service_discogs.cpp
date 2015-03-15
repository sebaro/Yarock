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
#include "service_discogs.h"
#include "networkaccess.h"
#include "debug.h"
#include "constants.h"
#include "utilities.h"

#include <QtCore>
#if QT_VERSION >= 0x050000
#include <QtCore/QJsonDocument>
#else
#include <qjson/parser.h>
#endif

/*
API
  http://www.discogs.com/help/api
  http://www.discogs.com/developers/
*/
namespace DISCOGS {
static const QString API_KEY = "91734dd989";
}

/*
********************************************************************************
*                                                                              *
*    Class ServiceDiscogs                                                      *
*                                                                              *
********************************************************************************
*/
ServiceDiscogs::ServiceDiscogs() : InfoService()
{
    Debug::debug() << Q_FUNC_INFO;

    setName("discogs");
    
     m_supportedInfoTypes << INFO::InfoAlbumInfo
                          << INFO::InfoAlbumSongs 
                          << INFO::InfoAlbumGenre
                          << INFO::InfoArtistReleases;
}


ServiceDiscogs::~ServiceDiscogs() {}


void ServiceDiscogs::getInfo( INFO::InfoRequestData requestData )
{
    emit checkCache( requestData );
}


void ServiceDiscogs::fetchInfo( INFO::InfoRequestData requestData )
{
    Debug::debug() << Q_FUNC_INFO;

    switch ( requestData.type )
    {
        case INFO::InfoArtistReleases      : fetch_artist_releases( requestData );break;
        case INFO::InfoAlbumInfo           : fetch_album_info( requestData );break;
        case INFO::InfoAlbumGenre          : fetch_album_info( requestData );break;
        case INFO::InfoAlbumSongs          : fetch_album_info( requestData );break;
        default:
        {
            emit finished( requestData );
            return;
        }
    }  
}


/*------------------------------------------------------------------------------*/
/*                                                                              */
/*     ARTIST SEARCH                                                            */
/*                                                                              */
/*------------------------------------------------------------------------------*/
void ServiceDiscogs::fetch_artist_releases( INFO::InfoRequestData requestData )
{
    //Debug::debug() << Q_FUNC_INFO;
    
    INFO::InfoStringHash hash = requestData.data.value< INFO::InfoStringHash >();

    if ( !hash.contains( "artist" ))
    {
        emit info( requestData, QVariant() );
        return;
    }
    
    QUrl url("http://api.discogs.com/database/search");
    // UTIL::urlAddQueryItem( url, "api_key", DISCOGS::API_KEY );
    UTIL::urlAddQueryItem( url, "page", QString::number( 1 ) );
    UTIL::urlAddQueryItem( url, "q", hash["artist"] );
    UTIL::urlAddQueryItem( url, "type", "artist" );
    
    //Debug::debug() << "    [ServiceDiscogs] get_artist " << url;
    QObject* reply = HTTP()->get(url);
    m_requests[reply] = requestData;

    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_parse_artist_search_response(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
}


void ServiceDiscogs::slot_parse_artist_search_response(QByteArray bytes)
{
    Debug::debug() << Q_FUNC_INFO;

    /*-------------------------------------------------*/
    /* Get id from sender reply                        */
    /* ------------------------------------------------*/
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply)) {
      return;
    }
    
    INFO::InfoRequestData request =  m_requests.take(reply);

#if QT_VERSION >= 0x050000
    bool ok = true;
    QVariantMap reply_map = QJsonDocument::fromJson(bytes).toVariant().toMap();
#else
    QJson::Parser parser;
    bool ok;
    QVariantMap reply_map = parser.parse(bytes, &ok).toMap();
#endif
    
    if (!ok || !reply_map.contains("results")) {
      return;
    }    
    
    QVariantList results = reply_map["results"].toList();

    foreach (const QVariant& result, results) {
      //Debug::debug() << Q_FUNC_INFO << result;

      QVariantMap result_map = result.toMap();
      
      if (result_map.contains("id")) {
          QUrl url( QString("http://api.discogs.com/artists/%1/releases").arg(result_map["id"].toString()) );
 
          QObject* reply = HTTP()->get( url );
          m_requests[reply] = request;          
          connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_parse_artist_release_response(QByteArray)));
          connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
          return;
      }
    }
}




void ServiceDiscogs::slot_parse_artist_release_response(QByteArray bytes)
{
    Debug::debug() << Q_FUNC_INFO;

    /*-------------------------------------------------*/
    /* Get id from sender reply                        */
    /* ------------------------------------------------*/
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply)) {
      return;
    }
    
    INFO::InfoRequestData request =  m_requests.take(reply);
    INFO::InfoStringHash input = request.data.value< INFO::InfoStringHash >();
    
    /*-------------------------------------------------*/
    /* Parse response                                  */
    /* ------------------------------------------------*/
#if QT_VERSION >= 0x050000
    bool ok = true;
    QVariantMap reply_map = QJsonDocument::fromJson(bytes).toVariant().toMap();
#else
    QJson::Parser parser;
    bool ok;
    QVariantMap reply_map = parser.parse(bytes, &ok).toMap();
#endif

    if (!reply_map.contains("releases"))
      emit finished(request);
    

    QVariantList output_releases;
    
    foreach (const QVariant& release, reply_map.value("releases").toList())
    {
        QVariantMap release_map = release.toMap();
        QVariantMap output_release;

        if (release_map.contains("type"))
          if (release_map.value("type").toString() != "master")
            continue;

        output_release["artist"]  = input.value("artist"); 
        output_release["album"]   = release_map.value("title").toString();  
        output_release["year"]    = release_map.value("year").toString(); 
        
        /* WARNING remove discogs image uri download as we need authentification */
        /*output_release["uri"]     = release_map.value("thumb").toString();*/
    
        output_releases << output_release;  

        if(output_releases.size() >= 11) break;
    }
    
    QVariantMap output;    
    output["releases"] = output_releases;
    emit info(request, QVariant(output) );    
}


/*------------------------------------------------------------------------------*/
/*                                                                              */
/*     ALBUM SEARCH                                                             */
/*                                                                              */
/*------------------------------------------------------------------------------*/
void ServiceDiscogs::fetch_album_info( INFO::InfoRequestData requestData )
{
    Debug::debug() << Q_FUNC_INFO;
    INFO::InfoStringHash hash = requestData.data.value< INFO::InfoStringHash >();

    if ( !hash.contains( "artist" ) && !hash.contains( "album" ) )
    {
        emit info( requestData, QVariant() );
        return;
    }

    QUrl url("http://api.discogs.com/database/search");
    //UTIL::urlAddQueryItem( url, "api_key", DISCOGS::API_KEY );
    UTIL::urlAddQueryItem( url, "page", QString::number( 1 ) );
    UTIL::urlAddQueryItem( url, "artist", hash["artist"] );
    UTIL::urlAddQueryItem( url, "release_title", hash["album"].toLower() );
    UTIL::urlAddQueryItem( url, "type", "release" );
 
    
    Debug::debug() << Q_FUNC_INFO << url;

    QObject* reply = HTTP()->get(url);
    m_requests[reply] = requestData;

    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_parse_album_search_response(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
  
}

void ServiceDiscogs::slot_parse_album_search_response(QByteArray bytes)
{
//     Debug::debug() << Q_FUNC_INFO;

    /*-------------------------------------------------*/
    /* Get id from sender reply                        */
    /* ------------------------------------------------*/
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply)) {
      Debug::debug() << Q_FUNC_INFO << " no reply found";
      return;
    }
    
    INFO::InfoRequestData request =  m_requests.take(reply);
    

#if QT_VERSION >= 0x050000
    bool ok = true;
    QVariantMap reply_map = QJsonDocument::fromJson(bytes).toVariant().toMap();
#else
    QJson::Parser parser;
    bool ok;
    QVariantMap reply_map = parser.parse(bytes, &ok).toMap();
#endif
    
    if (!ok || !reply_map.contains("results")) {
      return;
    }    
    
    const QVariantList results = reply_map.value( "results" ).toList();
    if(results.isEmpty())
      return;
       
    /* how to take the best result -> keep the first one */    
    const QVariantMap result = results.first().toMap();
    if ( result.contains( "id" ) )
    {       
      if ( request.type == INFO::InfoAlbumGenre ) 
      {
            QVariantList genres;
            genres = result.value("genre").toList();
            genres << result.value("style").toList();
    
            QVariantMap output;    
            output["genre"] = genres;
    
            emit info(request, QVariant(output));
            return;
      }
      else /* InfoAlbumInfo or InfoAlbumSongs */
      {
            QUrl url( QString("http://api.discogs.com/releases/%1").arg(result["id"].toString()) );
  
            QObject* reply = HTTP()->get( url );
            m_requests[reply] = request;          
            connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_parse_release_response(QByteArray)));
            connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
            return;
      }
      
      return;
    }
    
    Debug::debug() << Q_FUNC_INFO << " no result found";
    emit info(request, QVariant());
}

void ServiceDiscogs::slot_parse_release_response(QByteArray bytes)
{
//     Debug::debug() << Q_FUNC_INFO;
    /*-------------------------------------------------*/
    /* Get id from sender reply                        */
    /* ------------------------------------------------*/
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply)) {
      return;
    }
    
    INFO::InfoRequestData request =  m_requests.take(reply);
    INFO::InfoStringHash input = request.data.value< INFO::InfoStringHash >();
    
    /*-------------------------------------------------*/
    /* Parse response                                  */
    /* ------------------------------------------------*/
#if QT_VERSION >= 0x050000
    bool ok = true;
    QVariantMap reply_map = QJsonDocument::fromJson(bytes).toVariant().toMap();
#else
    QJson::Parser parser;
    bool ok;
    QVariantMap reply_map = parser.parse(bytes, &ok).toMap();
#endif

    if (reply_map.contains("status"))
    {
      QVariantMap output;
      output["site"] = "discogs";

      if (reply_map.contains("tracklist"))  
      {
        QStringList trackNameList;
        foreach ( const QVariant& v, reply_map.value("tracklist").toList() )
        {
            const QVariantMap track = v.toMap();
            if ( track.contains( "title" ) )
              trackNameList << track.value("title").toString();
        }

        output["tracks"] = trackNameList;
      }
      
      if (reply_map.contains("year"))  
        output["year"] = reply_map.value("year");
        
      if (reply_map.contains("uri"))
           output["url"] = reply_map.value("uri");

      emit info( request, output);
      return;
    }
  
    emit finished(request);
}


void ServiceDiscogs::slot_request_error()
{
    //Debug::debug() << Q_FUNC_INFO;

    /* get sender reply */
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))  {
      return;
    }

    INFO::InfoRequestData request = m_requests.take(reply);

    /* send process finished */
    emit info(request, QVariant());
}

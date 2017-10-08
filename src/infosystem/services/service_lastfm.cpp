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
#include "service_lastfm.h"
#include "networkaccess.h"
#include "debug.h"
#include "utilities.h"

namespace LASTFM {
static const QString API_KEY  = "b3717637c18071e1619e92ee2c3eb0f8";
}

// QJson
#if QT_VERSION >= 0x050000
#include <QtCore/QJsonDocument>
#else
#include <qjson/parser.h>
#endif

/*
********************************************************************************
*                                                                              *
*    Class ServiceLastFm                                                       *
*                                                                              *
********************************************************************************
*/
ServiceLastFm::ServiceLastFm() : InfoService()
{
    Debug::debug() << "    [ServiceLastFm] start";

    setName("Last.fm");
    
    m_supportedInfoTypes << INFO::InfoAlbumCoverArt
                         << INFO::InfoAlbumInfo
                         << INFO::InfoAlbumSongs  
                         << INFO::InfoArtistBiography
                         << INFO::InfoArtistImages
                         << INFO::InfoArtistSimilars
                         << INFO::InfoArtistTerms;
}


ServiceLastFm::~ServiceLastFm() {}



void ServiceLastFm::getInfo( INFO::InfoRequestData request )
{
    emit checkCache( request );
}


void ServiceLastFm::fetchInfo( INFO::InfoRequestData request )
{
    //Debug::debug() << "    [ServiceLastFm] fetchInfo";
    switch ( request.type )
    {
        case INFO::InfoAlbumCoverArt       : fetch_album_info( request );    break;
        case INFO::InfoAlbumSongs          : fetch_album_info( request );    break;  
        case INFO::InfoAlbumInfo           : fetch_album_info( request );    break;
        case INFO::InfoArtistBiography     : fetch_artist_info( request );   break;
	case INFO::InfoArtistImages        : fetch_artist_info( request );   break;
        case INFO::InfoArtistSimilars      : fetch_artist_similar( request );break;
        case INFO::InfoArtistTerms         : fetch_artist_tags( request );   break;
          
        default:
        {
            emit finished( request );
            return;
        }
    }  
}

/* methode : 
  use first album.getInfo method, 
  if no result -> use album.search method 
*/
// https://ws.audioscrobbler.com/2.0/?method=album.getinfo&api_key=e1db9fda381dea473df994bc26dfa1f1&artist=eric%20clapton&album=old%20sock&format=json

void ServiceLastFm::fetch_image_uri( INFO::InfoRequestData request )
{
    //Debug::debug() << "    [ServiceLastFm] fetch_image_uri";
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();
        
    QUrl url = QUrl( hash.value("#uri") );
    
    QObject* reply = HTTP()->get( url );
    m_requests[reply] = request;
    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_image_received(QByteArray)));
}


void ServiceLastFm::fetch_album_info( INFO::InfoRequestData request )
{
    //Debug::debug() << "    [ServiceLastFm] fetch_album_info";
    
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();
    
    if(request.type == INFO::InfoAlbumCoverArt && hash.contains( "#uri" ))
    {
        fetch_image_uri(request);
        return;
    }
    else if ( !hash.contains( "artist" ) && !hash.contains( "album" ) )
    {
        emit finished( request );
        return;
    }
    
    QUrl url("http://ws.audioscrobbler.com/2.0/");
    UTIL::urlAddQueryItem( url, "method", "album.getinfo");
    UTIL::urlAddQueryItem( url, "api_key", LASTFM::API_KEY);
    UTIL::urlAddQueryItem( url, "artist", hash["artist"]);
    UTIL::urlAddQueryItem( url, "album",  hash["album"]);
    UTIL::urlAddQueryItem( url, "format",  "json");
          
    QObject *reply = HTTP()->get(url);
    m_requests[reply] = request;

    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_parse_album_info(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
}


void ServiceLastFm::slot_parse_album_info(QByteArray bytes)
{
    //Debug::debug() << "    [ServiceLastFm] slot_parse_album_info";

    /*-------------------------------------------------*/
    /* Get id from sender reply                        */
    /* ------------------------------------------------*/
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply)) {
      return;
    }
    
    INFO::InfoRequestData request =  m_requests.take(reply);

    bool imageFound = false;
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
    
    if (!ok || !reply_map.contains("album")) {
      Debug::debug() << "    [ServiceLastFm] no album found";
      return;
    }    
    
    QVariantMap album_map =  qvariant_cast<QVariantMap>(reply_map.value("album"));
    
    if(request.type == INFO::InfoAlbumCoverArt) 
    {
        if (album_map.contains("image"))
        {
            foreach (const QVariant& image, album_map.value("image").toList())
            {
               if( image.toMap().value("size").toString() == "extralarge" )
               {
                  QUrl url = QUrl(image.toMap().value("#text").toString());
                  if(url.path().isEmpty()) break;
                  
                  QObject *reply = HTTP()->get(url);
                  m_requests[reply] = request;
    
                  connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_image_received(QByteArray)));
                  connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
                  imageFound = true;    
                  break;
                }
             }
        }
    }
    else if (request.type == INFO::InfoAlbumInfo || request.type == INFO::InfoAlbumSongs) 
    {
        QVariantMap output;
        output["site"] = "Last.fm";

        if (album_map.contains("wiki"))  {
            QVariantMap wiki_map =  qvariant_cast<QVariantMap>(album_map.value("wiki"));
            output["wiki"] =  wiki_map.value("content").toString();
        }     
        
        if (album_map.contains("tracks"))  {
            QVariantMap tracksMap =  qvariant_cast<QVariantMap>(album_map.value("tracks"));


            QVariantList tracks;
            foreach (const QVariant& track, tracksMap.value("track").toList() )
            {
              QVariantMap trackMap =  qvariant_cast<QVariantMap>(track);
              //Debug::debug() << "    [ServiceLastFm] " << trackMap["name"].toString();
              tracks << trackMap["name"].toString();
            }
            
            output["tracks"] = tracks;
        }
        
        if(album_map.contains("releasedate"))
        {
           QString lfm_date =  album_map.value("releasedate").toString().simplified(); 
           QDateTime releaseDate = QDateTime::fromString(lfm_date, "d MMM yyyy, hh:mm");
                             
           output["year"] = releaseDate.date().year(); /* int value */
        }
        
        if (album_map.contains("url"))  {
           output["url"] = album_map.value("url");
        }
        
        //Debug::debug() << "    [ServiceLastFm] OUTPUT ==" << output;
        emit info( request, output);
    }

    if(request.type == INFO::InfoAlbumCoverArt && !imageFound)
      emit finished( request );
}



void ServiceLastFm::fetch_artist_info( INFO::InfoRequestData request )
{
    //Debug::debug() << "    [ServiceLastFm] fetch_artist_info";
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();

    if(request.type == INFO::InfoArtistImages && hash.contains( "#uri" ))
    {
        fetch_image_uri(request);
        return;
    }
    

    QUrl url("http://ws.audioscrobbler.com/2.0/");
    UTIL::urlAddQueryItem( url, "method", "artist.getinfo");
    UTIL::urlAddQueryItem( url, "api_key", LASTFM::API_KEY);
    UTIL::urlAddQueryItem( url, "artist", hash["artist"]);
    UTIL::urlAddQueryItem( url, "format", "json");     
    
    QObject *reply = HTTP()->get(url);
    m_requests[reply] = request;        
    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_parse_artist_info(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
}

void ServiceLastFm::slot_parse_artist_info( QByteArray bytes )
{
    //Debug::debug() << "    [ServiceLastFm] ******** slot_parse_artist_info ******** ";

    /*-------------------------------------------------*/
    /* Get id from sender reply                        */
    /* ------------------------------------------------*/
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply)) {
      return;
    }
    
    INFO::InfoRequestData request =  m_requests.take(reply);

    bool imageFound = false;
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
    
    if (!ok || !reply_map.contains("artist")) {
      Debug::debug() << "    [ServiceLastFm] no artist found";
      return;
    }    
    
    if ( request.type == INFO::InfoArtistImages )
    {
       QVariantMap artist_map =  qvariant_cast<QVariantMap>(reply_map.value("artist"));
 
       if (artist_map.contains("image"))
        {
            foreach (const QVariant& image, artist_map.value("image").toList())
            {
               QVariantMap image_map = image.toMap();
               if(image_map.value("size").toString() == "extralarge")
               {
                  if( image_map.value("#text").toString().isEmpty() )
                    break;

                  QUrl url = QUrl(image_map.value("#text").toString());
                  QObject *reply = HTTP()->get(url);
                  m_requests[reply] = request;
    
                  connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_image_received(QByteArray)));
                  connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
                  imageFound = true;       
                  break;        
               }
            }
        }
        
        if(!imageFound)
          emit finished( request );
    }
    else if ( request.type == INFO::InfoArtistBiography )
    {
        QVariantMap outputBio;

        QVariantMap artist_map =  qvariant_cast<QVariantMap>(reply_map.value("artist"));

        if (artist_map.contains("bio"))
        {
          QVariantMap bio_map =  qvariant_cast<QVariantMap>(artist_map.value("bio"));

          outputBio[ "site" ]       = "LastFm";
          outputBio[ "url" ]        = artist_map.value("url");
          outputBio[ "text" ]       = bio_map.value("content");
          
          emit info( request, outputBio );        
       }
    }
}

void ServiceLastFm::fetch_artist_similar( INFO::InfoRequestData request )
{
    //Debug::debug() << "    [ServiceLastFm] fetch_artist_similar";
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();

    QUrl url("http://ws.audioscrobbler.com/2.0/");
    UTIL::urlAddQueryItem( url, "method", "artist.getsimilar");
    UTIL::urlAddQueryItem( url, "api_key", LASTFM::API_KEY);
    UTIL::urlAddQueryItem( url, "artist", hash["artist"]);
    UTIL::urlAddQueryItem( url, "format", "json");     
    
    QObject *reply = HTTP()->get(url);
    m_requests[reply] = request;        
    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_parse_artist_similar(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
}


void ServiceLastFm::slot_parse_artist_similar( QByteArray bytes )
{
    //Debug::debug() << "    [ServiceLastFm] slot_parse_artist_similar";

    /*-------------------------------------------------*/
    /* Get id from sender reply                        */
    /* ------------------------------------------------*/
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply)) {
      return;
    }
    
    INFO::InfoRequestData request =  m_requests.take(reply);

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
    
    if (!ok || !reply_map.contains("similarartists")) {
      Debug::debug() << "    [ServiceLastFm] no similar artists found";
      return;
    }    
    
    if ( request.type == INFO::InfoArtistSimilars )
    {
       QVariantMap similar_artist_map =  qvariant_cast<QVariantMap>(reply_map.value("similarartists"));
 
       if (similar_artist_map.contains("artist"))
       {
            int i = 0;         
            QVariantList similars;    
            foreach (const QVariant& artist, similar_artist_map.value("artist").toList())
            {
              
               QVariantMap artist_map =  qvariant_cast<QVariantMap>(artist);

               QVariantMap outArtistMap;
               outArtistMap["name"]         = artist_map["name"].toString(); 

               /* search artist image uri */             
               if (artist_map.contains("image")) 
               {
                   foreach (const QVariant& image, artist_map.value("image").toList())
                   {
                       QVariantMap image_map = image.toMap();
                       if(image_map.value("size").toString() == "large")
                       {
                          outArtistMap["uri"] = image_map.value("#text").toString();
                          break;        
                       }
                   }
               } 
               similars << outArtistMap;
               if(i++ >= 6) break;
            }
            emit info(request, similars); 
      }
    }
}


void ServiceLastFm::fetch_artist_tags( INFO::InfoRequestData request )
{
    //Debug::debug() << "    [ServiceLastFm] fetch_artist_tags";
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();
    
    QUrl url("http://ws.audioscrobbler.com/2.0/");
    UTIL::urlAddQueryItem( url, "method", "artist.gettoptags");
    UTIL::urlAddQueryItem( url, "api_key", LASTFM::API_KEY);
    UTIL::urlAddQueryItem( url, "artist", hash["artist"]);
    UTIL::urlAddQueryItem( url, "format", "json");
    
    QObject *reply = HTTP()->get(url);
    m_requests[reply] = request;        
    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_parse_artist_tags(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));    
}


void ServiceLastFm::slot_parse_artist_tags( QByteArray bytes )
{
    //Debug::debug() << "    [ServiceLastFm] slot_parse_artist_tags";

    /*-------------------------------------------------*/
    /* Get id from sender reply                        */
    /* ------------------------------------------------*/
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply)) {
      return;
    }
    
    INFO::InfoRequestData request =  m_requests.take(reply);

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
    
    if (!ok || !reply_map.contains("toptags")) {
      Debug::debug() << "    [ServiceLastFm] no artist tags found";
      return;
    }    
    
    if ( request.type == INFO::InfoArtistTerms )
    {
       QVariantMap vmap =  qvariant_cast<QVariantMap>(reply_map.value("toptags"));

       QVariantList tags;
       foreach (const QVariant& tag, vmap.value("tag").toList())
       {           
           QVariantMap tagMap = tag.toMap();

           int score = tagMap["count"].toInt();

           if(score > 75 )
             tags <<  tagMap["name"];

           if(tags.count() >= 4)
             break;
       }

       emit info(request, tags);
    }
}





void ServiceLastFm::slot_image_received(QByteArray bytes)
{
    //Debug::debug() << "    [ServiceLastFm] slot_image_received";
    
    // Get id from sender reply
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))   {
      return;
    }

    INFO::InfoRequestData request = m_requests.take(reply);

    // Send signal for data reception
    emit info(request, QVariant(bytes));
}

void ServiceLastFm::slot_request_error()
{
    //Debug::debug() << "    [ServiceLastFm] slot_request_error";
    
    /* get sender reply */
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))  {
      return;
    }

    INFO::InfoRequestData request = m_requests.take(reply);

    /* send process finished */
    emit finished( request );
}

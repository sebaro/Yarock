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
#include "service_echonest.h"
#include "networkaccess.h"
#include "utilities.h"
#include "debug.h"

// QJson
#if QT_VERSION >= 0x050000
#include <QtCore/QJsonDocument>
#else
#include <qjson/parser.h>
#endif


namespace Echonest {
static const QString API_KEY  = "DFLFLJBUF4EGTXHIG";
QByteArray escapeSpacesAndPluses(const QString& in);
}

/*
********************************************************************************
*                                                                              *
*    Class ServiceEchonest                                                     *
*                                                                              *
********************************************************************************
*/
ServiceEchonest::ServiceEchonest() : InfoService()
{
    Debug::debug() << Q_FUNC_INFO;

    setName("echonest");
    
    m_supportedInfoTypes << INFO::InfoArtistBiography 
                         << INFO::InfoArtistImages 
                         << INFO::InfoArtistBlog 
                         << INFO::InfoArtistSongs
                         << INFO::InfoArtistSimilars
                         << INFO::InfoArtistTerms;
}


ServiceEchonest::~ServiceEchonest() {}


void ServiceEchonest::getInfo( INFO::InfoRequestData requestData )
{
    emit checkCache( requestData );
}


void ServiceEchonest::fetchInfo( INFO::InfoRequestData requestData )
{
    //Debug::debug() << Q_FUNC_INFO;

    switch ( requestData.type )
    {
      
      case INFO::InfoArtistBiography : fetch_artist_bio(requestData); break;
      case INFO::InfoArtistImages    : fetch_artist_image(requestData);break;
      case INFO::InfoArtistBlog      : break;
      case INFO::InfoArtistSongs     : break;
      case INFO::InfoArtistSimilars  : fetch_artist_similar(requestData);break;
      case INFO::InfoArtistTerms     : break;

      default:
      {
          emit info( requestData, QVariant() );
          return;
      }
    }  
}

/*******************************************************************************
  Artists Biography
*******************************************************************************/
void ServiceEchonest::fetch_artist_bio(INFO::InfoRequestData request)
{
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();
    if ( hash.contains( "artist" ) )
    {
        QUrl url("http://developer.echonest.com/api/v4/artist/biographies");

        UTIL::urlAddQueryItem( url, QLatin1String("api_key"), Echonest::API_KEY );
        UTIL::urlAddQueryItem( url, QLatin1String("name"), QString::fromLatin1( Echonest::escapeSpacesAndPluses( hash["artist"] ) ) );
        UTIL::urlAddQueryItem( url, QLatin1String("results"), QString::number( 6 ) );
        UTIL::urlAddQueryItem( url, QLatin1String("format"), QLatin1String("json") );

        QObject *reply = HTTP()->get(url);
        m_requests[reply] = request;    
    
        connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_get_artist_biography(QByteArray)));
        connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
    }
}


void ServiceEchonest::slot_get_artist_biography(QByteArray bytes)
{
    /* get sender reply */
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply)) {
      return;
    }
    
    INFO::InfoRequestData request =  m_requests.take(reply);
        
    /* parse response */
    
#if QT_VERSION >= 0x050000
    bool ok = true;
    QVariantMap reply_map = QJsonDocument::fromJson(bytes).toVariant().toMap();
#else
    QJson::Parser parser;
    bool ok;
    QVariantMap reply_map = parser.parse(bytes, &ok).toMap();
#endif
    
    if (!ok || !reply_map.contains("response")) {
      Debug::debug() << "    [ServiceEchonest] bad response";
      return;
    }   

    QVariantMap resp_map =  qvariant_cast<QVariantMap>(reply_map.value("response"));

    if (resp_map.contains("status") && resp_map.contains("biographies"))
    {
        QVariantMap outputBio;
        QVariantMap foundBio;

        /* search for last fm */
        foreach (const QVariant& bio, resp_map.value("biographies").toList()) {
          if(bio.toMap().value("site").toString() == "last.fm") {
            foundBio = bio.toMap();
            break;
          }
        }

        /* search for wikipedia */
        if(foundBio.isEmpty()) {
          foreach (const QVariant& bio, resp_map.value("biographies").toList()) {
            if(bio.toMap().value("site").toString() == "wikipedia") {
              foundBio = bio.toMap();
              break;
            }
          }
        }
        
        /* take the first one otherwise */
        if(foundBio.isEmpty()) {
          foreach (const QVariant& bio, resp_map.value("biographies").toList()) {
              foundBio = bio.toMap();
              break;
          }
        }
        
        
        if(!foundBio.isEmpty()) 
        {
            outputBio[ "site" ]       = foundBio.value("site");
            outputBio[ "url" ]        = foundBio.value("url");
            outputBio[ "text" ]       = foundBio.value("text");
            emit info( request, outputBio );
        }
    }
}


/*******************************************************************************
  Artists Images
*******************************************************************************/
void ServiceEchonest::fetch_image_uri( INFO::InfoRequestData request )
{
    //Debug::debug() << Q_FUNC_INFO;
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();
        
    QUrl url = QUrl( hash.value("#uri") );
    
    QObject* reply = HTTP()->get( url );
    m_requests[reply] = request;
    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_image_received(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
}


void ServiceEchonest::fetch_artist_image(INFO::InfoRequestData request)
{
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();
    
    if ( hash.contains( "#uri" ) )
    {
        fetch_image_uri( request );
    }
    else if ( hash.contains( "artist" ) )
    {
        QUrl url("http://developer.echonest.com/api/v4/artist/images");

        UTIL::urlAddQueryItem( url, QLatin1String("api_key"), Echonest::API_KEY );
        UTIL::urlAddQueryItem( url, QLatin1String("name"), QString::fromLatin1( Echonest::escapeSpacesAndPluses( hash["artist"] ) ) );
        UTIL::urlAddQueryItem( url, QLatin1String("results"), QString::number( 1 ) );
        UTIL::urlAddQueryItem( url, QLatin1String("format"), QLatin1String("json") );
              
        QObject *reply = HTTP()->get(url);
        m_requests[reply] = request;    
    
        connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_get_artist_images(QByteArray)));
        connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
    }    
}


void ServiceEchonest::slot_get_artist_images(QByteArray bytes)
{
    /* get sender reply */
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))
      return;
    
    INFO::InfoRequestData request =  m_requests.take(reply);

    /* Parse response */
#if QT_VERSION >= 0x050000
    bool ok = true;
    QVariantMap reply_map = QJsonDocument::fromJson(bytes).toVariant().toMap();
#else
    QJson::Parser parser;
    bool ok;
    QVariantMap reply_map = parser.parse(bytes, &ok).toMap();
#endif

    if (!ok || !reply_map.contains("response")) {
      Debug::debug() << "    [ServiceEchonest] bad response";
      return;
    }   

    QVariantMap resp_map =  qvariant_cast<QVariantMap>(reply_map.value("response"));

    if (resp_map.contains("status") && resp_map.contains("images"))
    {
        foreach (const QVariant& image, resp_map.value("images").toList())
        {
            QUrl url = QUrl( image.toMap().value("url").toString() );
    
            QObject* reply = HTTP()->get( url );
            m_requests[reply] = request;
            connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_image_received(QByteArray)));
            break;
        }
    }
}


void ServiceEchonest::slot_image_received(QByteArray bytes)
{
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))
      return;

    INFO::InfoRequestData request =  m_requests.take(reply);

    emit info( request, QVariant(bytes) );
}


/*******************************************************************************
  Similar artists
*******************************************************************************/
void ServiceEchonest::fetch_artist_similar(INFO::InfoRequestData request)
{
    //Debug::debug() << Q_FUNC_INFO;
  
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();
    if ( !hash.contains( "artist" ) )
    {
        return;
    }

    QUrl url("http://developer.echonest.com/api/v4/artist/similar");
    
    
    UTIL::urlAddQueryItem( url, QLatin1String("api_key"), Echonest::API_KEY );
    UTIL::urlAddQueryItem( url, QLatin1String("name"), QString::fromLatin1( Echonest::escapeSpacesAndPluses( hash["artist"] ) ) );
    
    UTIL::urlAddQueryItem( url, QLatin1String("bucket"), QLatin1String("images") );
    UTIL::urlAddQueryItem( url, QLatin1String("min_hotttnesss"), QLatin1String("0.5") );
    
    UTIL::urlAddQueryItem( url, QLatin1String("results"), QString::number( 6 ) );
    UTIL::urlAddQueryItem( url, QLatin1String("format"), QLatin1String("json") );
              
    QObject *reply = HTTP()->get(url);
    m_requests[reply] = request;    
    
    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_get_artist_similar(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
}

void ServiceEchonest::slot_get_artist_similar(QByteArray bytes)
{
    /* get sender reply */
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))
      return;
    
    INFO::InfoRequestData request =  m_requests.take(reply);

    /* Parse response */
#if QT_VERSION >= 0x050000
    bool ok = true;
    QVariantMap reply_map = QJsonDocument::fromJson(bytes).toVariant().toMap();
#else
    QJson::Parser parser;
    bool ok;
    QVariantMap reply_map = parser.parse(bytes, &ok).toMap();
#endif

    if (!ok || !reply_map.contains("response")) {
      Debug::debug() << "    [ServiceEchonest] bad response";
      return;
    }   

    QVariantMap resp_map =  qvariant_cast<QVariantMap>(reply_map.value("response"));

    if (resp_map.contains("status") && resp_map.contains("artists"))
    {
        QVariantList output_similars;        
        foreach (const QVariant& artist, resp_map.value("artists").toList())
        {
            QVariantMap output_artist_map;
            QVariantMap artist_map = artist.toMap();
            
            output_artist_map["name"] =  artist_map.value("name");
            
            foreach (const QVariant& image, artist_map.value("images").toList())
            {
               output_artist_map["uri"] =  image.toMap().value("url");
               break;
            }
        
            output_similars << output_artist_map;
            if(output_similars.size() >= 6) break;
        }
        
        emit info(request, output_similars);        
    }
}


void ServiceEchonest::slot_request_error()
{
    Debug::debug() << Q_FUNC_INFO;
    /* get sender reply */
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))  {
      return;
    }

    INFO::InfoRequestData request = m_requests.take(reply);

    /* send process finished */
    emit finished( request );
}



QByteArray Echonest::escapeSpacesAndPluses(const QString& in)
{
    // Echonest wants " " treated as "+", so we force QUrl to encode spaces as +es rather than %20
    QByteArray escaped = QUrl::toPercentEncoding( in, " " );
    escaped.replace( " ", "+" );
    return escaped;
}

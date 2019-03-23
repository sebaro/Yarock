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
#include "service_spotify.h"
#include "networkaccess.h"

#include "utilities.h"
#include "debug.h"
#include "config.h"

// QJson
#if QT_VERSION >= 0x050000
#include <QtCore/QJsonDocument>
#else
#include <qjson/parser.h>
#endif


namespace Spotify 
{
  static const QString CLIENT_ID     = "53235d5dc7ef42c1add557dae2cfd3fd";

  static const QString CLIENT_SECRET = "4cf739045a234e16a2083a3a047e0e75";

  QByteArray paramToBytearray( QMap<QString, QString>& params )
  {
    QByteArray data;
    QMapIterator<QString, QString> i(params);
    while (i.hasNext()) {
        i.next();
        data += QUrl::toPercentEncoding(i.key())
             + '='
             + QUrl::toPercentEncoding(i.value())
             + '&';
    }
    return data;
  }  
}

const QString USER_AGENT = QString( QString(APP_NAME) + "/" + QString(VERSION) );


/*
********************************************************************************
*                                                                              *
*    Class ServiceSpotify                                                      *
*                                                                              *
********************************************************************************
*/
ServiceSpotify::ServiceSpotify() : InfoService()
{
    Debug::debug() << "    [ServiceSpotify] start";

    setName("spotify");
    
    m_supportedInfoTypes << INFO::InfoArtistImages;

    getAuthentification();
}


ServiceSpotify::~ServiceSpotify() 
{
}



void ServiceSpotify::getInfo( INFO::InfoRequestData requestData )
{
    emit checkCache( requestData );
}


void ServiceSpotify::fetchInfo( INFO::InfoRequestData requestData )
{
    Debug::debug() << "    [ServiceSpotify::] fetchInfo";
    
    switch ( requestData.type )
    {      
      case INFO::InfoArtistImages    : fetch_artist_image(requestData);break;
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
  Authentification
*******************************************************************************/
void ServiceSpotify::getAuthentification()
{
    Debug::debug() << "    [ServiceSpotify] getAuthentification";
  
    QUrl url("https://accounts.spotify.com/api/token");

    QNetworkRequest request(url);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    QByteArray ba;
    ba.append(Spotify::CLIENT_ID + ":" + Spotify::CLIENT_SECRET );
    request.setRawHeader("Authorization", "Basic " + ba.toBase64());
    Debug::debug() << "    [ServiceSpotify]  ba.toBase64()" <<  ba.toBase64();


    QMap<QString, QString> params;
    params["grant_type"]    = "client_credentials";
    
    QObject *reply = HTTP()->post(request,Spotify::paramToBytearray(params));
 
    connect(reply, SIGNAL(finished(QNetworkReply*)), SLOT(slot_getAuthentification(QNetworkReply*)));  
}


void ServiceSpotify::slot_getAuthentification(QNetworkReply* reply)
{
    Debug::debug() << "    [ServiceSpotify] slot_getAuthentification";
    
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200)
    {
        Debug::debug() << "    [ServiceSpotify] http error" << status;
        return;
    }

#if QT_VERSION >= 0x050000
    bool ok = true;
    QVariantMap reply_map = QJsonDocument::fromJson(reply->readAll()).toVariant().toMap();
#else
    QJson::Parser parser;
    bool ok;
    QVariantMap reply_map = parser.parse(reply->readAll(), &ok).toMap();
#endif
    
    if (!ok || !reply_map.contains("access_token")) {
      m_auth_info["access_token"] = "";
      Debug::debug() << "    [ServiceSpotify] no authentication token";
      return;
    }    
    
    
    m_auth_info["access_token"] = reply_map.value("access_token").toString();
    m_auth_info["token_type"] = reply_map.value("token_type").toString();
}


/*******************************************************************************
  Artists Images
*******************************************************************************/
void ServiceSpotify::fetch_artist_image(INFO::InfoRequestData request)
{
    Debug::debug() << "    [ServiceSpotify] fetch_artist_image";
    
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();
    
    if ( hash.contains( "artist" ) )
    {
        QUrl url("https://api.spotify.com/v1/search");

        UTIL::urlAddQueryItem( url, QLatin1String("type"), "artist");
        UTIL::urlAddQueryItem( url, QLatin1String("q"), hash["artist"] );
        UTIL::urlAddQueryItem( url, QLatin1String("limit"), QString::number( 1 ) );

        QNetworkRequest req(url);
        req.setRawHeader("Authorization", 
            m_auth_info.value("token_type").toUtf8() + " " + m_auth_info.value("access_token").toUtf8()
        );
        
        QObject *reply = HTTP()->get( req/*url*/);
        m_requests[reply] = request;    
    
        connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_get_artist_images(QByteArray)));
        connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_request_error()));
    }
}


void ServiceSpotify::slot_get_artist_images(QByteArray bytes)
{
    Debug::debug() << "    [ServiceSpotify] slot_get_artist_images";
  
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

    if (!ok || !reply_map.contains("artists")) {
      Debug::debug() << "    [ServiceSpotify] bad response " << reply_map;
      return;
    }   

    QVariantMap resp_map =  qvariant_cast<QVariantMap>(reply_map.value("artists"));

    if ( resp_map.contains("items") )
    {      
        foreach (const QVariant& artist, resp_map.value("items").toList())
        {
           if( artist.toMap().contains("images") )
           {
              foreach (const QVariant& image, artist.toMap().value("images").toList())
              {
                 QUrl url = QUrl( image.toMap().value("url").toString() );
            
                 QObject* reply = HTTP()->get( url );
                 m_requests[reply] = request;
                 connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_image_received(QByteArray)));
                 break;
              }
           }
           break;
        }
    }
}


void ServiceSpotify::slot_image_received(QByteArray bytes)
{
    Debug::debug() << "    [ServiceSpotify] slot_image_received";
  
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))
      return;

    INFO::InfoRequestData request =  m_requests.take(reply);

    emit info( request, QVariant(bytes) );  
}


/*******************************************************************************
  Similar artists
*******************************************************************************/
void ServiceSpotify::fetch_artist_similar(INFO::InfoRequestData request)
{
Q_UNUSED(request)
  
}

void ServiceSpotify::slot_get_artist_similar(QByteArray bytes)
{
Q_UNUSED(bytes)    
}


/*******************************************************************************
  Error
*******************************************************************************/
void ServiceSpotify::slot_request_error()
{
    Debug::debug() << "    [ServiceSpotify] slot_request_error";
    /* get sender reply */
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))  {
      return;
    }

    INFO::InfoRequestData request = m_requests.take(reply);

    /* send process finished */
    emit finished( request );
}



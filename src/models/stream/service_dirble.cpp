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
#include "service_dirble.h"
#include "networkaccess.h"
#include "covers/covercache.h"
#include "views/item_button.h"

#include "utilities.h"
#include "debug.h"

#include <QtCore>
#if QT_VERSION >= 0x050000
#include <QtCore/QJsonDocument>
#else
#include <qjson/parser.h>
#endif


/* KEY
 *   b6909ed70f32338fe171e4df174c1eb6e388ca98
 * NEW URL BASE API v2:
 *   http://api.dirble.com/v2/
 * PRIMARY GENRE
 *   http://api.dirble.com/v2/categories/primary?token=<api-key>
 * SEARCH
 *   http://api.dirble.com/v2/search/<search_query>?token=<api-key>
 */
/*
********************************************************************************
*                                                                              *
*  Global                                                                      *
*                                                                              *
********************************************************************************
*/
namespace DIRBLE {
    QString key_id;
}


/*
********************************************************************************
*                                                                              *
*    Class Dirble                                                              *
*                                                                              *
********************************************************************************
*/
Dirble::Dirble() : Service("Dirble", SERVICE::DIRBLE)
{
    Debug::debug() << "    [Dirble] create";
    
    DIRBLE::key_id   = "b6909ed70f32338fe171e4df174c1eb6e388ca98";
      
    /* root link */
    m_root_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_root_link->url  = QString("http://api.dirble.com/v2/categories/primary?token=%1").arg(DIRBLE::key_id);
    m_root_link->name = QString("dirble directory");
    m_root_link->state = int (SERVICE::NO_DATA);
   
    /* search link */
    m_search_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_search_link->name = QString("search result");
    m_search_link->state = int (SERVICE::NO_DATA);
      
    /* register state */
    m_active_link = m_root_link;
    set_state(SERVICE::NO_DATA);
}

void Dirble::reload()
{
    m_root_link.reset();
    delete m_root_link.data();
    
    /* root link */
    m_root_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_root_link->url  = QString("http://api.dirble.com/v2/categories/primary?token=%1").arg(DIRBLE::key_id);
    m_root_link->name = QString("dirble directory");
    m_root_link->state = int (SERVICE::NO_DATA);
   
    /* search link */
    m_search_term.clear();
    
    m_search_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_search_link->name = QString("search result");
    m_search_link->state = int (SERVICE::NO_DATA);
    
    /* register update */    
    m_active_link = m_root_link;
    m_active_link->state = int(SERVICE::NO_DATA);
    set_state(SERVICE::NO_DATA);    
    
    emit stateChanged();
}

void Dirble::load()
{
    Debug::debug() << "    [Dirble] load";
    if(state() == SERVICE::DOWNLOADING)
      return;
    
    m_active_link->state = int(SERVICE::DOWNLOADING);
    set_state(SERVICE::DOWNLOADING);
    
    emit stateChanged();

    browseLink(m_active_link);
}

QList<MEDIA::LinkPtr> Dirble::links()
{
    Debug::debug() << "    [Dirble] links";
    QList<MEDIA::LinkPtr> links;

    bool found_root = false;
    MEDIA::LinkPtr p_link = m_active_link;
    while(p_link)
    {
       if(p_link == m_root_link)
         found_root = true;
       
        int i = 0;

        foreach(MEDIA::MediaPtr media, p_link->children()) {
          if(media->type() == TYPE_LINK)
            links.insert(i++, MEDIA::LinkPtr::staticCast(media));
        }    

        p_link = p_link->parent();
    }
    
    /* always shall root link if not done */
    if(!found_root) {
      foreach(MEDIA::MediaPtr media, m_root_link->children()) {
        int i = 0;

        if(media->type() == TYPE_LINK)
          links.insert(i++, MEDIA::LinkPtr::staticCast(media));
      }    
    }    

    return links;
}


QList<MEDIA::TrackPtr> Dirble::streams()
{
    //Debug::debug() << "Dirble::streams";
    QList<MEDIA::TrackPtr> streams;
  
    foreach(MEDIA::MediaPtr media, m_active_link->children()) {
      if(media->type() == TYPE_STREAM) {
        MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast(media);
        streams << stream;
      }
    }
      
    return streams;
}

void Dirble::browseLink(MEDIA::LinkPtr link)
{
    Debug::debug() << "    [Dirble] browseLink " << link->url;
    QUrl url(link->url);

    QObject *reply = HTTP()->get(url);
    m_requests[reply] = link;    
    connect(reply, SIGNAL(data(QByteArray)), SLOT(slotBrowseLinkDone(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_error()));
} 

void Dirble::slotBrowseLinkDone(QByteArray bytes)
{
    Debug::debug() << "    [Dirble] slotBrowseLinkDone ";
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply)) {
      return;
    }
    
    MEDIA::LinkPtr link = m_requests.take(reply);
    
    /* parse response */
#if QT_VERSION >= 0x050000
    QVariantList reply_list = QJsonDocument::fromJson(bytes).toVariant().toList();
#else
    QJson::Parser parser;
    bool ok;
    QVariantList reply_list = parser.parse(bytes, &ok).toList();

    if (!ok) return;
#endif

    foreach (const QVariant& genre_link, reply_list) 
    {
        QVariantMap map = genre_link.toMap();

	/* http://api.dirble.com/v2/categories/primary?token=b6909ed70f32338fe171e4df174c1eb6e388ca98 */
        if (link->url.contains("primary")) 
        {
              /*   http://api.dirble.com/v2/category/:id/childs?token=API_KEY */
              MEDIA::LinkPtr link2 = MEDIA::LinkPtr::staticCast( link->addChildren(TYPE_LINK) );
              link2->setType(TYPE_LINK);
              link2->name  = map["title"].toString();
              link2->url   = QString("http://api.dirble.com/v2/category/%1/childs?token=%2").
                             arg(map["id"].toString(), DIRBLE::key_id);
              link2->state = int(SERVICE::NO_DATA);
              link2->categorie = link->name;
              link2->setParent(link);
        }
        else if (link->url.contains("childs")) 
        {
              /*   http://api.dirble.com/v2/category/:id/stations?token=API_KEY */
              MEDIA::LinkPtr link2 = MEDIA::LinkPtr::staticCast( link->addChildren(TYPE_LINK) );
              link2->setType(TYPE_LINK);
              link2->name  = map["title"].toString();
              link2->url   = QString("http://api.dirble.com/v2/category/%1/stations?token=%2").
                               arg(map["id"].toString(), DIRBLE::key_id);
              link2->state = int(SERVICE::NO_DATA);
              link2->categorie = link->name;
              link2->setParent(link);
        }
    }

    if(link == m_search_link) 
    {
        Debug::debug() << "    [Dirble] slotBrowseLinkDone search link : station found";
      
        foreach (const QVariant& station, reply_list) 
        {
            QVariantMap map = station.toMap();

            foreach (const QVariant& station_stream, map["streams"].toList())
            {
              QVariantMap stream_map = station_stream.toMap();

              /* check status of station */
              if(stream_map["status"].toInt() != 1) continue;
              /* check if url of stream is empty */
              if(stream_map["stream"].toString().isEmpty()) continue;
      
              MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast( link->addChildren(TYPE_TRACK) );
              stream->setType(TYPE_STREAM);
              stream->name = map["name"].toString();
              stream->url  = stream_map["stream"].toString();
              stream->categorie = link->name;
              stream->setParent(link);  
              
              const QString cover = map["image"].toMap()["url"].toString();
              if( !cover.isEmpty() )
              {
                  QObject* reply = HTTP()->get( QUrl(cover) );
                  m_image_requests[reply] = stream;
                  connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_stream_image_received(QByteArray)));
              }              
            }
        }
    }

    /* if link is not root or search link => request stations */
    if(link != m_root_link && link != m_search_link) 
    {
      browseStation(link);
    }

    /* register update */    
    link->state = int(SERVICE::DATA_OK);
    set_state(SERVICE::DATA_OK);
    emit stateChanged();
}

void Dirble::browseStation(MEDIA::LinkPtr link)
{
     QStringList list = link->url.split("/");
     list.removeLast();
     
     QString station_id = list.last();
     
     QUrl url;
     url.setUrl( 
         QString("http://api.dirble.com/v2/category/%1/stations?token=%2").
         arg(station_id, DIRBLE::key_id)
     );
    
    QObject *reply = HTTP()->get(url);
    m_requests[reply] = link;
    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slotBrowseStationDone(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_error()));       
}

//! ------------------  station request ----------------------------------------
void Dirble::slotBrowseStationDone(QByteArray bytes)
{
    Debug::debug() << "    [Dirble] slotBrowseStationDone ";
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))
      return;
    
    MEDIA::LinkPtr link = m_requests.take(reply);
    
    /* parse response */
#if QT_VERSION >= 0x050000
    QVariantList reply_list = QJsonDocument::fromJson(bytes).toVariant().toList();
#else
    QJson::Parser parser;
    bool ok;
    QVariantList reply_list = parser.parse(bytes, &ok).toList();

    if (!ok) return;
#endif

    foreach (const QVariant& station, reply_list) 
    {
       QVariantMap map = station.toMap();

       foreach (const QVariant& station_stream, map["streams"].toList())
       {
          QVariantMap stream_map = station_stream.toMap();
  
          /* check status of station */
          if(stream_map["status"].toInt() != 1) continue;
          /* check if url of stream is empty */
          if(stream_map["stream"].toString().isEmpty()) continue;
  
          MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast( link->addChildren(TYPE_TRACK) );
          stream->setType(TYPE_STREAM);
          stream->name = map["name"].toString();
          stream->url  = stream_map["stream"].toString();
          stream->categorie = link->name;
          stream->setParent(link);  
          
          const QString cover = map["image"].toMap()["url"].toString();
          if( !cover.isEmpty() )
          {
              QObject* reply = HTTP()->get( QUrl(cover) );
              m_image_requests[reply] = stream;
              connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_stream_image_received(QByteArray)));
          }
       }
    }
    
    /* register update */    
    link->state = int(SERVICE::DATA_OK);
    set_state(SERVICE::DATA_OK);    
    
    emit stateChanged();
}

/*******************************************************************************
  Dirble::slot_stream_image_received
*******************************************************************************/
void Dirble::slot_stream_image_received(QByteArray bytes)
{
    //Debug::debug() << "    [Dirble] slot_stream_image_received";

    // Get id from sender reply
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_image_requests.contains(reply))   return;

    MEDIA::TrackPtr stream = m_image_requests.take(reply);

    QImage image = QImage::fromData(bytes);
    if( !image.isNull() ) 
    {
        CoverCache::instance()->addStreamCover(stream, image);
        emit dataChanged();
    }
}


void Dirble::slot_error()
{
    Debug::debug() << "    [Dirble] slot_error";
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))
      return;
    
    MEDIA::LinkPtr link = m_requests.take(reply);    
    
    /* register update */      
    link->state = int(SERVICE::ERROR);
    set_state(SERVICE::ERROR);    
    emit stateChanged();
}

void Dirble::slot_activate_link(MEDIA::LinkPtr link)
{
    Debug::debug() << "    [Dirble] slot_activate_link";
  
    if(!link) 
    {
      ButtonItem* button = qobject_cast<ButtonItem*>(sender());
    
      if (!button) return;  
  
      m_active_link = qvariant_cast<MEDIA::LinkPtr>( button->data() );
    }
    else
    {
      m_active_link = link;
    }


    if(m_active_link == m_search_link )
    {
      m_search_link->url   = QString("http://api.dirble.com/v2/search/%1?token=%2").arg(m_search_term, DIRBLE::key_id);
      m_search_link->state = int (SERVICE::NO_DATA);
      m_search_link->deleteChildren();
    }
    else
    {
      m_search_term.clear();      
    }
    
    /* register update */        
    set_state(SERVICE::State(m_active_link->state));
    emit stateChanged();
}


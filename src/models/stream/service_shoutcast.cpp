/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2014 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

#include "service_shoutcast.h"
#include "networkaccess.h"
#include "views/item_button.h"

#include "utilities.h"
#include "debug.h"

#include <QtCore>

/*
 * API : http://wiki.winamp.com/wiki/SHOUTcast_Radio_Directory_API
 * PRIMARY GENRE
 *    http://api.shoutcast.com/genre/primary?k=[Your Dev ID]&f=xml
 * SECONDARY GENRE
 *   http://api.shoutcast.com/genre/secondary?parentid=0&k=[Your Dev ID]&f=xml
 * Stations by genre
 *   http://api.shoutcast.com/legacy/genresearch?k=[Your Dev ID]&genre=classic
 * SEARCH
 *   http://api.shoutcast.com/legacy/stationsearch?k=[Your Dev ID]&search=ambient+beats
 */


/*
********************************************************************************
*                                                                              *
*  Global                                                                      *
*                                                                              *
********************************************************************************
*/
namespace SHOUTCAST {
    QString key_id;
}

/*
********************************************************************************
*                                                                              *
*    Class ShoutCast                                                           *
*                                                                              *
********************************************************************************
*/
ShoutCast::ShoutCast() : Service("Shoutcast", SERVICE::SHOUTCAST)
{
    Debug::debug() << "ShoutCast -> create";
    
    SHOUTCAST::key_id   = "fa1669MuiRPorUBw";
      
    /* root link */
    m_root_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_root_link->url  = QString("http://api.shoutcast.com/genre/primary");
    m_root_link->name = QString("shoutcast directory");
    m_root_link->state = int (SERVICE::NO_DATA);
   
    /* search link */
    m_search_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_search_link->name = QString("search result");
    m_search_link->state = int (SERVICE::NO_DATA);
    
    m_active_link = m_root_link;
    set_state(SERVICE::NO_DATA);
}

void ShoutCast::reload()
{
    m_root_link.reset();
    delete m_root_link.data();
    
    /* root link */
    m_root_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_root_link->url  = QString("http://api.shoutcast.com/genre/primary");
    m_root_link->name = QString("shoutcast directory");
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

void ShoutCast::load()
{
    Debug::debug() << "ShoutCast::load";
    if(state() == SERVICE::DOWNLOADING)
      return;
    
    m_active_link->state = int(SERVICE::DOWNLOADING);
    set_state(SERVICE::DOWNLOADING);
    
    emit stateChanged();

    browseLink(m_active_link);
}

QList<MEDIA::LinkPtr> ShoutCast::links()
{
    Debug::debug() << "ShoutCast::links";
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

QList<MEDIA::TrackPtr> ShoutCast::streams()
{
    //Debug::debug() << "ShoutCast::streams";
    QList<MEDIA::TrackPtr> streams;
  
    foreach(MEDIA::MediaPtr media, m_active_link->children()) {
      if(media->type() == TYPE_STREAM) {
        MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast(media);
        streams << stream;
      }
    }
      
    return streams;
}

void ShoutCast::browseLink(MEDIA::LinkPtr link)
{
    QUrl url(link->url);
    
    if(!UTIL::urlHasQueryItem(url, "k"))
      UTIL::urlAddQueryItem( url, "k", SHOUTCAST::key_id);    
    if(!UTIL::urlHasQueryItem(url, "f"))
      UTIL::urlAddQueryItem( url, "f", "xml");

    Debug::debug() << "ShoutCast::browseLink " << url.toString();
    
    QObject *reply = HTTP()->get(url);
    m_requests[reply] = link;    
    connect(reply, SIGNAL(data(QByteArray)), SLOT(slotBrowseLinkDone(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_error()));
} 

void ShoutCast::slotBrowseLinkDone(QByteArray bytes)
{
    Debug::debug() << "ShoutCast::slotBrowseLinkDone ";
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))
      return;
    
    MEDIA::LinkPtr link = m_requests.take(reply);
    
    
    /* parse response */
    QXmlStreamReader xml(bytes);

    if(m_active_link != m_search_link) 
    {
        while(!xml.atEnd() && !xml.hasError()) {
          xml.readNext();
          if (xml.isStartElement() && xml.name() == "genre")
          {
              //Debug::debug() << "ShoutCast::link found";
              MEDIA::LinkPtr link2 = MEDIA::LinkPtr::staticCast( link->addChildren(TYPE_LINK) );
              link2->setType(TYPE_LINK);
              link2->name = xml.attributes().value("name").toString();
              link2->state = int(SERVICE::NO_DATA);
              link2->categorie = link->name;
              link2->setParent(link);      
      
              QString parent_id = xml.attributes().value("id").toString();
              link2->url =  QString("http://api.shoutcast.com/genre/secondary?parentid=%1").arg(parent_id);
          }
        }  
    }
    else
    {
        while(!xml.atEnd() && !xml.hasError()) {
          xml.readNext();
          if (xml.isStartElement() && xml.name() == "station") 
          {
              MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast( link->addChildren(TYPE_TRACK) );
              stream->setType(TYPE_STREAM);
              stream->name = xml.attributes().value("name").toString();

              QString id = xml.attributes().value("id").toString();
              stream->url  = QString("http://yp.shoutcast.com/sbin/tunein-station.pls?id=%1").arg(id);

              stream->categorie = link->name;
              stream->setParent(link);
          }
      }
    }
    

    /* if link is not root or search link => request stations */
    if(link != m_root_link && link != m_search_link) 
    {
      browseStation(link);
    }
    else 
    {
      /* register update */    
      link->state = int(SERVICE::DATA_OK);
      set_state(SERVICE::DATA_OK);
      emit stateChanged();
    }
}

void ShoutCast::browseStation(MEDIA::LinkPtr link)
{
    Debug::debug() << "ShoutCast -> browseStation for genre" << link->name;

    QUrl url;
    url.setUrl(QString("http://api.shoutcast.com/legacy/genresearch?genre=%1").arg(link->name));
    UTIL::urlAddQueryItem( url, "k", SHOUTCAST::key_id);   
    UTIL::urlAddQueryItem( url, "f", "xml");   

    QObject *reply = HTTP()->get(url);
    m_requests[reply] = link;
    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slotBrowseStationDone(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_error()));    
}

//! ------------------  station request ----------------------------------------
void ShoutCast::slotBrowseStationDone(QByteArray bytes)
{
    Debug::debug() << "ShoutCast -> slotBrowseStationDone ";
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))
      return;
    
    MEDIA::LinkPtr link = m_requests.take(reply);
    
    
    /* parse response */
    QXmlStreamReader xml(bytes);

    while(!xml.atEnd() && !xml.hasError()) {
      xml.readNext();
      if (xml.isStartElement() && xml.name() == "station") 
      {
          MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast( link->addChildren(TYPE_TRACK) );
          stream->setType(TYPE_STREAM);
          stream->name = xml.attributes().value("name").toString();
              
          QString id = xml.attributes().value("id").toString();
          stream->url  = QString("http://yp.shoutcast.com/sbin/tunein-station.pls?id=%1").arg(id);

          stream->categorie = link->name;
          stream->setParent(link);
      }
    }
    
    /* register update */    
    link->state = int(SERVICE::DATA_OK);
    set_state(SERVICE::DATA_OK);    
    
    emit stateChanged();
}

void ShoutCast::slot_error()
{
    Debug::debug() << "ShoutCast::slot_error";
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))
      return;
    
    MEDIA::LinkPtr link = m_requests.take(reply);    
    
    /* register update */     
    link->state = int(SERVICE::ERROR);
    set_state(SERVICE::ERROR);    
    emit stateChanged();
}



void ShoutCast::slot_activate_link(MEDIA::LinkPtr link)
{
    Debug::debug() << "ShoutCast::slot_activate_link";
  
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

    if(m_active_link == m_search_link ) {
      m_search_link->url   = QString("http://api.shoutcast.com/legacy/stationsearch?k=%1&f=xml&search=%2").arg(SHOUTCAST::key_id, m_search_term);
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

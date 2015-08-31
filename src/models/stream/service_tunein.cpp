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

#include "service_tunein.h"
#include "covers/covercache.h"
#include "networkaccess.h"
#include "views/item_button.h"
#include "utilities.h"
#include "debug.h"

/* 
API : 
 Local Radio -> http://opml.radiotime.com/Browse.ashx?c=local
 Talk        -> http://opml.radiotime.com/Browse.ashx?c=talk
 Sports      -> http://opml.radiotime.com/Browse.ashx?c=sports
 Music       -> http://opml.radiotime.com/Browse.ashx?c=music
 By Location -> http://opml.radiotime.com/Browse.ashx?id=r0
 SEARCH      -> http://opml.radiotime.com/Search.ashx?query=<text>
*/
        
/*
********************************************************************************
*                                                                              *
*  Global                                                                      *
*                                                                              *
********************************************************************************
*/
namespace TUNEIN {
    QString partner_id;
    QString username;
    QString password;
}


/*
********************************************************************************
*                                                                              *
*    Class TuneIn                                                              *
*                                                                              *
********************************************************************************
*/
TuneIn::TuneIn() : Service("Tunein", SERVICE::TUNEIN)
{
    Debug::debug() << "    [Tunein] create";
    
    /* restore settings */
    TUNEIN::partner_id   = "yvcOjvJP";
    
    /* root link */
    m_root_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_root_link->url  = QString("http://opml.radiotime.com/Browse.ashx");
    m_root_link->name = QString("tunein directory");
    m_root_link->state = int (SERVICE::NO_DATA);
   
    /* search link */
    m_search_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_search_link->name = QString("search result");
    m_search_link->state = int (SERVICE::NO_DATA);
    
    m_active_link = m_root_link;
    set_state(SERVICE::NO_DATA);    
}

void TuneIn::reload()
{
    m_root_link.reset();
    delete m_root_link.data();
    
    /* root link */
    m_root_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_root_link->url  = QString("http://opml.radiotime.com/Browse.ashx");
    m_root_link->name = QString("tunein directory");
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

void TuneIn::load()
{
    Debug::debug() << "    [Tunein] load";
    if(state() == SERVICE::DOWNLOADING)
      return;
    
    m_active_link->state = int(SERVICE::DOWNLOADING);
    set_state(SERVICE::DOWNLOADING);
    
    emit stateChanged();

    browseLink(m_active_link);
}

QList<MEDIA::LinkPtr> TuneIn::links()
{
    Debug::debug() << "    [Tunein] links";
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

QList<MEDIA::TrackPtr> TuneIn::streams()
{
    QList<MEDIA::TrackPtr> streams;
  
    foreach(MEDIA::MediaPtr media, m_active_link->children()) {
      if(media->type() == TYPE_STREAM) {
        MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast(media);
        streams << stream;
      }
    }
      
    return streams;
}


void TuneIn::browseLink(MEDIA::LinkPtr link)
{
    //http://opml.radiotime.com/Browse.ashx?c=local&partnerId=<partnerid>&serial=<serial>
    Debug::debug() << "    [Tunein] browseLink : " << link->url;
    
    QUrl url(link->url);
    if(!UTIL::urlHasQueryItem(url, "partnerId"))
      UTIL::urlAddQueryItem( url, "partnerId", TUNEIN::partner_id);    
    if(!UTIL::urlHasQueryItem(url, "render"))
      UTIL::urlAddQueryItem( url, "render", "xml");
    
    QObject *reply = HTTP()->get(url);
    m_requests[reply] = link;    
    connect(reply, SIGNAL(data(QByteArray)), SLOT(slotBrowseLinkDone(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slotBrowseLinkError()));
} 

void TuneIn::slotBrowseLinkDone(QByteArray bytes)
{
    Debug::debug() << "    [Tunein] slotBrowseLinkDone";
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))
      return;
    
    MEDIA::LinkPtr link = m_requests.take(reply);    
    
    /* parse response */
    QXmlStreamReader xml(bytes);

    while(!xml.atEnd() && !xml.hasError())
    {
      xml.readNext();
      if (xml.isStartElement()) {
          if(xml.name() == "outline")
          {
            if(xml.attributes().value("type").toString() == "audio")
            {
              //Debug::debug() << "    [Tunein] stream found";
              MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast( link->addChildren(TYPE_TRACK) );
              stream->setType(TYPE_STREAM);
              stream->name = xml.attributes().value("text").toString();
              stream->url  = xml.attributes().value("URL").toString();

              if(xml.attributes().hasAttribute("image")) {
                QString i_url = xml.attributes().value("image").toString();
                //Debug::debug() << "    [Tunein] url image " << i_url ;

                  QUrl image_url = QUrl(i_url);
                  if(image_url.isValid()) {
                    QObject* reply = HTTP()->get( image_url );
                    m_image_requests[reply] = stream;
                    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_stream_image_received(QByteArray)));
                  }
              }

              stream->genre = link->name;
              stream->setParent(link);
            }
            else if (xml.attributes().value("type").toString() == "link")
            {
              //Debug::debug() << "    [Tunein] link found";
              MEDIA::LinkPtr link2 = MEDIA::LinkPtr::staticCast( link->addChildren(TYPE_LINK) );
              link2->setType(TYPE_LINK);
              link2->name   = xml.attributes().value("text").toString();
              link2->url    = xml.attributes().value("URL").toString();
              link2->state  = int(SERVICE::NO_DATA);
              link2->genre  = link->name;
              link2->setParent(link);
            }
          }
      }
    }    
    
    
    /* register update */    
    link->state = int(SERVICE::DATA_OK);
    set_state(SERVICE::DATA_OK);    
    
    emit stateChanged();
}  

/*******************************************************************************
  TuneIn::slot_stream_image_received
*******************************************************************************/
void TuneIn::slot_stream_image_received(QByteArray bytes)
{
    //Debug::debug() << "    [Tunein] slot_stream_image_received";

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


void TuneIn::slotBrowseLinkError()
{
    Debug::debug() << "    [Tunein] slotBrowseLinkError";
   
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))
      return;
    
    MEDIA::LinkPtr link = m_requests.take(reply);    
    
    link->state = int(SERVICE::ERROR);
    set_state(SERVICE::ERROR);    
    emit stateChanged();
}  


void TuneIn::slot_activate_link(MEDIA::LinkPtr link)
{
    Debug::debug() << "    [Tunein] slot_activate_link";
  
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
      m_search_link->url   = QString("http://opml.radiotime.com/Search.ashx?partnerId=%1&query=%2").arg(TUNEIN::partner_id, m_search_term);
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

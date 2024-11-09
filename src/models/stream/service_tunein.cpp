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

#include "service_tunein.h"
#include "covers/covercache.h"
#include "networkaccess.h"
#include "views/item_button.h"
#include "utilities.h"
#include "debug.h"

#include <QtCore>
#if QT_VERSION >= 0x050000
#include <QtCore/QJsonDocument>
#else
#include <qjson/parser.h>
#endif

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
    m_root_link->name = QString("TuneIn");
    m_root_link->state = int (SERVICE::NO_DATA);

    /* search link */
    m_search_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_search_link->name = QString("search result");
    m_search_link->state = int (SERVICE::NO_DATA);

    m_active_link = m_root_link;
    set_state(SERVICE::NO_DATA);

    loadGenres();
}

void TuneIn::reload()
{
    m_root_link.reset();
    delete m_root_link.data();

    /* root link */
    m_root_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_root_link->url  = QString("http://opml.radiotime.com/Browse.ashx");
    m_root_link->name = QString("TuneIn");
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

    //! If search link, sort stream by categorie
    if(m_active_link == m_search_link )
      std::sort(streams.begin(), streams.end(),MEDIA::compareStreamCategorie);

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
      UTIL::urlAddQueryItem( url, "render", "json");

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
#if QT_VERSION >= 0x050000
    QVariantMap replyMap = QJsonDocument::fromJson(bytes).toVariant().toMap();
#else
    QJson::Parser parser;
    bool ok;
    QVariantMap replyMap = parser.parse(bytes, &ok).toMap();

    if (!ok) return;
#endif

    QVariantList list =     replyMap["body"].toList();

    foreach (const QVariant& element, list)
    {
        QVariantMap map = element.toMap();
        if( map["element"].toString() == "outline")
        {
            if( map.contains("type") )
            {
                parseTuneInJsonElement(map,link);
            }
            else if (map.contains("children"))
            {
              foreach (QVariant va,map["children"].toList())
              {
                parseTuneInJsonElement(va.toMap(),link);
              }
            }
        }
    }


    /* register update */
    if(link != m_genre_link)
    {
      link->state = int(SERVICE::DATA_OK);
      set_state(SERVICE::DATA_OK);
    }

    emit stateChanged();
}



/*******************************************************************************
  TuneIn::parseTuneInJsonElement
*******************************************************************************/
void TuneIn::parseTuneInJsonElement(QVariantMap map, MEDIA::LinkPtr link)
{
    if ( map["type"].toString() == "text" )
    {
        //Debug::debug() << "    [Tunein] genres: " << map["guide_id"].toString() << " : " << map["text"].toString();

        m_genres[map["guide_id"].toString()] = map["text"].toString();
    }
    else if ( map["type"].toString() == "link" && link != m_genre_link)
    {
        //Debug::debug() << "    [Tunein] link found";
        MEDIA::LinkPtr link2 = MEDIA::LinkPtr::staticCast( link->addChildren(TYPE_LINK) );
        link2->setType(TYPE_LINK);

        link2->name   = map["text"].toString();
        link2->url    = map["URL"].toString();
        link2->state  = int(SERVICE::NO_DATA);
        link2->setParent(link);
    }
    else if( map["type"].toString() == "audio" )
    {
        //Debug::debug() << "    [Tunein] stream found";
        MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast( link->addChildren(TYPE_TRACK) );
        stream->setType(TYPE_STREAM);

        stream->url               = map["URL"].toString();
        stream->genre             = link->name;
        stream->extra["station"]  = map["text"].toString();
        stream->extra["provider"] = this->name();
        stream->extra["website"]  = QString("http://tunein.com/station/?stationId=%1").arg(map["guide_id"].toString().mid(1));
        stream->extra["bitrate"]  = map["bitrate"].toString();

        if( map.contains("image") )
        {
            QString url = map["image"].toString();

            if( !url.isEmpty() && CoverCache::instance()->coverPath(stream).isEmpty() )
            {
            QObject* reply = HTTP()->get( url );
            m_image_requests[reply] = stream;
            connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_stream_image_received(QByteArray)));
            }
        }

        if(m_search_link->name == link->name)
        {
            if(m_genres.contains(map["genre_id"].toString()))
                stream->genre = m_genres[map["genre_id"].toString()];
            else
                stream->genre = "Unknown";
        }
        else
        {
            stream->genre = link->name;
        }

        stream->setParent(link);
    }
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


void TuneIn::loadGenres()
{
    m_genre_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_genre_link->url  = QString("http://opml.radiotime.com/Describe.ashx?c=genres");
    m_genre_link->state = int (SERVICE::NO_DATA);

    browseLink(m_genre_link);
}

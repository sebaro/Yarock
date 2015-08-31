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

#include "service_radionomy.h"
#include "covers/covercache.h"
#include "networkaccess.h"
#include "views/item_button.h"

#include "utilities.h"
#include "debug.h"


#include <QtCore>

/*
********************************************************************************
*                                                                              *
*    Class Radionomy                                                           *
*                                                                              *
********************************************************************************
*/
Radionomy::Radionomy() : Service("Radionomy", SERVICE::RADIONOMY)
{
    Debug::debug() << "    [Radionomy] create";
    
    /* root link */
    m_root_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_root_link->url  = QString("https://www.radionomy.com/en/style");
    m_root_link->name = QString("radionomy directory");
    m_root_link->state = int (SERVICE::NO_DATA);
   
    /* search link */
    m_search_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_search_link->name = QString("search result");
    m_search_link->state = int (SERVICE::NO_DATA);
      
    /* register state */
    m_active_link = m_root_link;
    set_state(SERVICE::NO_DATA);
}

void Radionomy::reload()
{
    m_root_link.reset();
    delete m_root_link.data();
    
    /* root link */
    m_root_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_root_link->url  = QString("https://www.radionomy.com/en/style");
    m_root_link->name = QString("radionomy directory");
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

void Radionomy::load()
{
    Debug::debug() << "    [Radionomy] load";
    if(state() == SERVICE::DOWNLOADING)
      return;
    
    m_active_link->state = int(SERVICE::DOWNLOADING);
    set_state(SERVICE::DOWNLOADING);
    
    emit stateChanged();

    browseLink(m_active_link);
}

QList<MEDIA::LinkPtr> Radionomy::links()
{
    Debug::debug() << "    [Radionomy] links";
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

QList<MEDIA::TrackPtr> Radionomy::streams()
{
    //Debug::debug() << "Radionomy::streams";
    QList<MEDIA::TrackPtr> streams;
  
    foreach(MEDIA::MediaPtr media, m_active_link->children()) {
      if(media->type() == TYPE_STREAM) {
        MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast(media);
        streams << stream;
      }
    }
      
    return streams;
}

void Radionomy::browseLink(MEDIA::LinkPtr link)
{
    QUrl url(link->url);

    Debug::debug() << "    [Radionomy] browseLink " << url.toString();
    
    QObject *reply = HTTP()->get(url);
    m_requests[reply] = link;    
    connect(reply, SIGNAL(data(QByteArray)), SLOT(slotBrowseLinkDone(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(slot_error()));
} 

void Radionomy::slotBrowseLinkDone(QByteArray bytes)
{
using namespace htmlcxx;    
    Debug::debug() << "    [Radionomy] slotBrowseLinkDone ";
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))
      return;
    
    MEDIA::LinkPtr link = m_requests.take(reply);
    
    
    HTML::ParserDom parser;
    tree<HTML::Node> dom = parser.parseTree(bytes.constData());

   
    /* get categories */
    if( link == m_root_link )
        categoryLinkFromHtml(dom, link, "mainGenre");
    else if (link->parent() == m_root_link)
        categoryLinkFromHtml(dom, link, "subGenre");


    /* get radio entry */
    tree <HTML::Node> :: iterator it = dom.begin ();
    tree <HTML::Node> :: iterator end = dom.end ();
    
    for (; it != end; ++it)
    {
        if( it->isTag() && QString::fromStdString( it->tagName() ) == "div" )
        {
            it->parseAttributes();
            if( "browseRadioWrap" == QString::fromStdString(it->attribute("class").second) )
            {
                
                std::string s = bytes.constData();
                std::string content = 
                 s.substr(it->offset()
                 + it->text().length(), it->length() 
                 - (it->text().length() + it->closingText().length()));
                 
                   tree<HTML::Node> dom2 = parser.parseTree(content);
                   streamFromHtml(dom2,link);
                   
               
            }
        }
    } // main loop on document


    /* register update */    
    link->state = int(SERVICE::DATA_OK);
    set_state(SERVICE::DATA_OK);
    emit stateChanged();
}

void Radionomy::categoryLinkFromHtml(tree<htmlcxx::HTML::Node> dom, MEDIA::LinkPtr link, QString className)
{
using namespace htmlcxx; 
    
    //Debug::debug() << "primaryLinkFromHtml";
    
    QString href;    
                 
    tree <HTML::Node>:: iterator it = dom.begin ();
    tree <HTML::Node>:: iterator end = dom.end ();
    
    for (; it != end; ++it)
    {
        if( it->isTag() && QString::fromStdString( it->tagName() ) == "select" )
        {            
            it->parseAttributes();
            if( className ==  QString::fromStdString(it->attribute("class").second) )
            {
                //Debug::debug() << "TAG NAME:" << QString::fromStdString( it->tagName() ) ;
                //Debug::debug() << "CLASS:" << QString::fromStdString(it->attribute("class").second);
                //Debug::debug() << "TEXT:" << QString::fromStdString(it->text()) ;

                for ( unsigned i=0; i<dom.number_of_children(it); i++ )
                {
                    tree<HTML::Node>::iterator itChild = dom.child(it,i).begin();

                    if( itChild->isTag() && QString::fromStdString( itChild->tagName() ) == "option")
                    {
                            itChild->parseAttributes();
                            href = "http://www.radionomy.com" +
                            QString::fromStdString(itChild->attribute("href").second);
                    }
                    else if ( ! it->isComment () && !itChild->isTag())
                    {
                            //Debug::debug() << " TRY PARSE CATEGORIE:";
                            if( !href.isEmpty() )
                            {                        
                                QString name = QString::fromStdString(itChild->text());
                                //Debug::debug() << "CATEGORIE:" << name;

                                MEDIA::LinkPtr link2 = MEDIA::LinkPtr::staticCast( link->addChildren(TYPE_LINK) );
                                link2->setType(TYPE_LINK);
                                link2->name = name;
                                link2->state = int(SERVICE::NO_DATA);
                                link2->genre = link->name;
                                link2->setParent(link);      
                        
                                link2->url = href;
                                href = "";
                            }
                    }
                }
            }
        }
    }
}


void Radionomy::streamFromHtml(tree<htmlcxx::HTML::Node> dom, MEDIA::LinkPtr link)
{
using namespace htmlcxx; 
    
    //Debug::debug() << "streamFromHtml";
    
    QString name, cover, url;
    
    tree <HTML::Node> :: iterator it = dom.begin ();
    tree <HTML::Node> :: iterator end = dom.end ();
    for (; it != end; ++it)
    {
        if( it->isTag())
        {
            it->parseAttributes();
            
            if( "radioName" == QString::fromStdString(it->attribute("class").second) )
            {
                ++it;
                name = QString::fromStdString(it->text());
            }
            else if("radioCover" == QString::fromStdString(it->attribute("class").second) )
            {                
                it->parseAttributes();
                cover = QString::fromStdString(it->attribute("src").second);
            }
            else if ("radioPlayBtn" == QString::fromStdString(it->attribute("class").second)  )
            {
                it->parseAttributes();
                //QRegExp regExp ("(http://)&");
                QRegExp regExp ("(http://([^&()\"' ]*))");
                
                regExp.indexIn(QString::fromStdString(it->attribute("data-play-stream").second));
                url = regExp.capturedTexts().first();
            }
        }
    }    
    
    MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast( link->addChildren(TYPE_TRACK) );
    stream->setType(TYPE_STREAM);

    stream->name  = name;
    stream->url   = url;
    stream->genre = link->name;
    stream->setParent(link);    
    
    QObject* reply = HTTP()->get( QUrl(cover) );
    m_image_requests[reply] = stream;
    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(slot_stream_image_received(QByteArray)));
}


/*******************************************************************************
  Radionomy::slot_stream_image_received
*******************************************************************************/
void Radionomy::slot_stream_image_received(QByteArray bytes)
{
    //Debug::debug() << "    [Radionomy] slot_stream_image_received";

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


/*******************************************************************************
  Radionomy::slot_error
*******************************************************************************/
void Radionomy::slot_error()
{
    Debug::debug() << "    [Radionomy] slot_error";
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))
      return;
    
    MEDIA::LinkPtr link = m_requests.take(reply);    
    
    /* register update */     
    link->state = int(SERVICE::ERROR);
    set_state(SERVICE::ERROR);    
    emit stateChanged();
}

/*******************************************************************************
  Radionomy::slot_activate_link
*******************************************************************************/
void Radionomy::slot_activate_link(MEDIA::LinkPtr link)
{
    Debug::debug() << "    [Radionomy] slot_activate_link";
  
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
      m_search_link->url   = QString("https://www.radionomy.com/en/search/index?query=%1").arg(m_search_term);
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
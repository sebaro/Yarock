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

#include "models/stream/service_xspf.h"
#include "core/mediaitem/playlist_parser.h"

#include "utilities.h"
#include "debug.h"
#include "views/item_button.h"

/*
********************************************************************************
*                                                                              *
*    Class XspfStreams                                                         *
*                                                                              *
********************************************************************************
*/
XspfStreams::XspfStreams() : Service(tr("Favorite streams"), SERVICE::LOCAL)
{
    m_filename = QString(UTIL::CONFIGDIR + "/radio/#favorite.xspf");

    /* root link */
    m_root_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_root_link->name = QString("favorites streams");
    m_root_link->state = int (SERVICE::NO_DATA);
   
    m_active_link = m_root_link;
    
    /* restore state  */
    set_state(SERVICE::NO_DATA);
}

XspfStreams::~XspfStreams() 
{
}


void XspfStreams::updateItem(MEDIA::TrackPtr stream)
{
    Debug::debug() << "    [XspfStreams] updateItem";
  
    if(stream->isFavorite)
    {
      /* remove it from favorite */ 
      foreach(MEDIA::TrackPtr _stream, m_streams) {
        if(_stream == stream) {
          m_streams.removeOne(stream);
          stream->isFavorite = false;
          break;
        }
      }
    }
    else
    {
      /* add it to favorite */  
      m_streams << stream;
      stream->isFavorite = true;
    }
}


QList<MEDIA::LinkPtr> XspfStreams::links()
{
    Debug::debug() << "    [XspfStreams] links";
  
    QList<MEDIA::LinkPtr> links;

    foreach(MEDIA::MediaPtr media, m_active_link->children()) {
        if(media->type() == TYPE_LINK)
          links << MEDIA::LinkPtr::staticCast(media);
    }    
    
    MEDIA::LinkPtr p_link = m_active_link->parent();
    while(p_link)
    {
        int i = 0;

        foreach(MEDIA::MediaPtr media, p_link->children()) {
          if(media->type() == TYPE_LINK)
            links.insert(i++, MEDIA::LinkPtr::staticCast(media));
        }    

        p_link = p_link->parent();
    }
    
    return links;
}


QList<MEDIA::TrackPtr> XspfStreams::streams()
{
    QList<MEDIA::TrackPtr> streams;
  
    foreach(MEDIA::MediaPtr media, m_active_link->children())
    {
      if(media->type() == TYPE_STREAM) 
      {
        MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast(media);
        streams << stream;
      }
      /* !! add auto recursion to see all favorite stream when opening view */
      else if(media->type() == TYPE_LINK) 
      {
         foreach(MEDIA::MediaPtr child, media->children()) {
            if(child->type() == TYPE_STREAM) {
              MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast(child);
              streams << stream;
            }
         }
      }
    }
      
    return streams;
}

void XspfStreams::reload() 
{
    m_root_link.reset();
    delete m_root_link.data();
    
    /* root link */
    m_root_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_root_link->name = QString("favorites streams");
    m_root_link->state = int (SERVICE::NO_DATA);
   
    m_active_link = m_root_link;


    /* register update */    
    m_active_link->state = int(SERVICE::NO_DATA);
    set_state(SERVICE::NO_DATA);    
    
    emit stateChanged();
}
    
void XspfStreams::load()
{
    Debug::debug() << "    [XspfStreams] load";

    /* read file */       
    m_streams.clear();
    m_streams = MEDIA::PlaylistFromFile(m_filename);

    qSort(m_streams.begin(), m_streams.end(), MEDIA::compareStreamCategorie);
    
    /* build model */   
    MEDIA::LinkPtr sub_link;
    QString current_cat;
    foreach (MEDIA::TrackPtr stream, m_streams)
    {
        if( stream->genre.isEmpty() )
        {
            stream->genre = "unknown";
        }

        if( current_cat != stream->genre )
        {
            sub_link = MEDIA::LinkPtr::staticCast( m_root_link->addChildren(TYPE_LINK) );
            sub_link->setType(TYPE_LINK);
            sub_link->name   = stream->genre;
            sub_link->state  = int(SERVICE::DATA_OK);
            sub_link->genre  = m_root_link->name;
            sub_link->setParent(m_root_link);  

            current_cat = stream->genre;
        }
      
        sub_link->insertChildren( stream );
        stream->setParent( sub_link );
    }
   
    /* emit change */    
    m_root_link->state = int (SERVICE::DATA_OK);
    set_state(SERVICE::DATA_OK);
    emit stateChanged();
}


void XspfStreams::saveToFile()
{
    Debug::debug() << "    [XspfStreams] saveToFile";
  
    MEDIA::PlaylistToFile(m_filename, m_streams);
}


void XspfStreams::slot_activate_link(MEDIA::LinkPtr link)
{
    Debug::debug() << "    [XspfStreams] slot_activate_link";
  
    MEDIA::LinkPtr activated_link;
    
    activated_link = link;
    
    if(!link) {
      ButtonItem* button = qobject_cast<ButtonItem*>(sender());
    
      if (!button) return;  
  
      activated_link = qvariant_cast<MEDIA::LinkPtr>( button->data() );
    }

    if(activated_link) {
      m_active_link = activated_link;
      set_state(SERVICE::State(m_active_link->state));
      emit stateChanged();
    }
}


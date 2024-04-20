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

#include "models/stream/service_favorite.h"
#include "core/database/database.h"
#include "core/database/database_cmd.h"
#include "threadmanager.h"

#include "utilities.h"
#include "debug.h"
#include "views/item_button.h"

#include <QSqlQuery>
#include <QSqlError>
/*
********************************************************************************
*                                                                              *
*    Class FavoriteStreams                                                     *
*                                                                              *
********************************************************************************
*/
FavoriteStreams::FavoriteStreams() : Service(tr("Favorite streams"), SERVICE::LOCAL)
{
    /* root link */
    m_root_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_root_link->name  = QString(tr("Favorite"));
    m_root_link->state = int (SERVICE::NO_DATA);
   
    m_active_link = m_root_link;
    
    /* restore state  */
    set_state(SERVICE::NO_DATA);
    
    connect(ThreadManager::instance(), SIGNAL(modelPopulationFinished(E_MODEL_TYPE)), this, SLOT(slot_dbBuilder_stateChange()));
}

FavoriteStreams::~FavoriteStreams() 
{
}



void FavoriteStreams::slot_dbBuilder_stateChange()
{
    reload();
}


void FavoriteStreams::updateStreamFavorite(MEDIA::TrackPtr stream)
{
    //Debug::debug() << "    [FavoriteStreams] updateStreamFavorite " << stream->id;
    DatabaseCmd::updateStreamFavorite(stream);
}

void FavoriteStreams::addOrRemovetoFavorite(MEDIA::TrackPtr stream)
{
    Debug::debug() << "    [FavoriteStreams] addOrRemovetoFavorite";
  
    if(stream->isFavorite)
    {
        DatabaseCmd::removeStreamToFavorite(stream);
        stream->isFavorite = false;
    }
    else
    {
        DatabaseCmd::addStreamToFavorite(stream);
        stream->isFavorite = true;
    }
}


QList<MEDIA::LinkPtr> FavoriteStreams::links()
{
    Debug::debug() << "    [FavoriteStreams] links";
  
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


QList<MEDIA::TrackPtr> FavoriteStreams::streams()
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

void FavoriteStreams::reload() 
{
    m_root_link.reset();
    delete m_root_link.data();
    
    /* root link */
    m_root_link = MEDIA::LinkPtr(new MEDIA::Link());
    m_root_link->name  = QString(tr("Favorite"));
    m_root_link->state = int (SERVICE::NO_DATA);
   
    m_active_link = m_root_link;


    /* register update */    
    m_active_link->state = int(SERVICE::NO_DATA);
    set_state(SERVICE::NO_DATA);    
    
    emit stateChanged();
}
    
void FavoriteStreams::load()
{
    Debug::debug() << "    [FavoriteStreams] load";

    m_streams.clear();


    /*-----------------------------------------------------------*/
    /* Get connection                                            */
    /* ----------------------------------------------------------*/
    if (!Database::instance()->open()) {
        Debug::warning() << "  [FavoriteStreams] db connect failed";
        /* emit change */    
        m_root_link->state = int (SERVICE::ERROR);
        set_state(SERVICE::ERROR);
        emit stateChanged();
        return;
    }


    MEDIA::LinkPtr currentLink = MEDIA::LinkPtr(0);

    QSqlQuery query("SELECT id,url,name,genre,website,provider,bitrate,samplerate,format FROM favorite_stream ORDER BY genre COLLATE NOCASE ASC",*Database::instance()->db());

    while ( query.next() )
    {
        /* new genre link */
        if( !currentLink || currentLink->name != query.value(3).toString() )
        {
            //Debug::debug() << "new Link";
            currentLink         = MEDIA::LinkPtr::staticCast( m_root_link->addChildren(TYPE_LINK) );
            currentLink->setType(TYPE_LINK);
            currentLink->name   = query.value(3).toString();
            currentLink->state  = int(SERVICE::DATA_OK);
            currentLink->setParent(m_root_link);
        }


        /* new stream */
        MEDIA::TrackPtr stream = MEDIA::TrackPtr(new MEDIA::Track());
        stream->setType(TYPE_STREAM);
        stream->id                  = query.value(0).toInt();
        stream->url                 = query.value(1).toString();
        stream->extra["station"]    = query.value(2).toString();
        stream->genre               = query.value(3).toString();
        stream->extra["website"]    = query.value(4).toString();
        stream->extra["provider"]   = query.value(5).toString();
        stream->extra["bitrate"]    = query.value(6).toString();
        stream->extra["samplerate"] = query.value(7).toString();
        stream->extra["format"]     = query.value(8).toString();
        stream->isFavorite          = true;
        stream->isPlaying           = false;
        stream->isBroken            = false;
        stream->isPlayed            = false;
        stream->isStopAfter         = false;
        
        currentLink->insertChildren( stream );
        stream->setParent( currentLink );
        
        m_streams << stream;
    }
    
    /* emit change */    
    m_root_link->state = int (SERVICE::DATA_OK);
    set_state(SERVICE::DATA_OK);
    emit stateChanged();
}

bool FavoriteStreams::findStream(MEDIA::TrackPtr stream)
{
    /* loop over streams list */
    foreach(MEDIA::TrackPtr _stream, m_streams) 
    {
        if( _stream->title == stream->title &&
            _stream->url == stream->url ) 
        {
          return true;
          break;
        }
      }
      
    return false;
}


void FavoriteStreams::slot_activate_link(MEDIA::LinkPtr link)
{
    Debug::debug() << "    [FavoriteStreams] slot_activate_link";
  
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


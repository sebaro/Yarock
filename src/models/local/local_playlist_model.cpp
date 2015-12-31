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


#include "local_playlist_model.h"
#include "core/mediaitem/mediaitem.h"
#include "core/mediasearch/media_search_engine.h"

//#include "debug.h"


LocalPlaylistModel* LocalPlaylistModel::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class LocalPlaylistModel                                                  *
*                                                                              *
********************************************************************************
*/
LocalPlaylistModel::LocalPlaylistModel(QObject *parent) : QObject(parent)
{
    INSTANCE = this;

    m_rootItem          = MEDIA::MediaPtr(new MEDIA::Media());
}

LocalPlaylistModel::~LocalPlaylistModel()
{
    m_rootItem.reset();
}

MEDIA::MediaPtr LocalPlaylistModel::rootItem()
{
    return m_rootItem;
}


void LocalPlaylistModel::clear()
{
    m_rootItem.reset();
    m_rootItem        = MEDIA::MediaPtr(new MEDIA::Media());
}

//! ----------------------- Get urls method ------------------------------------
QList<MEDIA::TrackPtr> LocalPlaylistModel::getItemChildrenTracks(const MEDIA::MediaPtr parent)
{
    QList<MEDIA::TrackPtr> result;

    if(!parent) return result;

    MEDIA::MediaPtr media = parent;

    if (media->type() == TYPE_TRACK || media->type() == TYPE_STREAM)
    {
      SearchEngine se;
      bool match = se.mediaMatch(
                       qvariant_cast<MediaSearch>(m_search), 
                       MEDIA::TrackPtr::staticCast(media) );
      if(match)
        result.append( media );
    }
    else {
      //! Recursive !!
      for (int i = 0; i < media->childCount(); i++) {
        result.append( getItemChildrenTracks(media->child(i)) );
      }
    }
    return result;
}


/*******************************************************************************
    isMediaMatch
*******************************************************************************/
bool LocalPlaylistModel::isMediaMatch(MEDIA::MediaPtr media)
{
    if(m_search.isNull())
      return true;

    if (media->type() == TYPE_TRACK || media->type() == TYPE_STREAM)
    {
      SearchEngine se;
      bool match = se.mediaMatch(
                       qvariant_cast<MediaSearch>(m_search),
                       MEDIA::TrackPtr::staticCast(media) );
      if(match) return true;
    }
    else
    {
      for (int i = 0; i < media->childCount(); i++)
        if( isMediaMatch( media->child(i) ) )
          return true;
    }
    
    return false;
}

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

#include "local_track_model.h"

#include "core/mediaitem/mediaitem.h"
#include "core/mediasearch/media_search_engine.h"

#include "debug.h"

#include <QRegExp>

LocalTrackModel* LocalTrackModel::INSTANCE = 0;


/*
********************************************************************************
*                                                                              *
*    Class LocalTrackModel                                                     *
*                                                                              *
********************************************************************************
*/
LocalTrackModel::LocalTrackModel(QObject *parent) : QObject(parent)
{
    INSTANCE         = this;

    m_rootItem       = MEDIA::MediaPtr(new MEDIA::Media());
    m_playing_track  = MEDIA::TrackPtr(0);
}

LocalTrackModel::~LocalTrackModel()
{
    m_rootItem.reset();
}


MEDIA::MediaPtr LocalTrackModel::rootItem()
{
    return m_rootItem;
}


void LocalTrackModel::clear()
{
    m_rootItem.reset();
    m_rootItem = MEDIA::MediaPtr(new MEDIA::Media());

    trackItemHash.clear();
    albumItemList.clear();
    trackByGenre.clear();

    m_playing_track  = MEDIA::TrackPtr(0);

    emit modelCleared();
}


bool LocalTrackModel::isEmpty() const
{
    return m_rootItem->childCount() == 0;
}
     
     

//! ----------------------- Get tracks method ----------------------------------
QList<MEDIA::TrackPtr> LocalTrackModel::getItemChildrenTracks(const MEDIA::MediaPtr parent)
{
    //Debug::debug() << "LocalTrackModel::getItemChildrenTracks";

    QList<MEDIA::TrackPtr> result;
    if(!parent)
      return result;

    MEDIA::MediaPtr media = parent;

    if (media->type() == TYPE_TRACK) 
    {
      SearchEngine se;
      bool match = se.mediaMatch(
                       qvariant_cast<MediaSearch>(m_search), 
                       MEDIA::TrackPtr::staticCast(media) );
      if(match)
        result.append( MEDIA::TrackPtr::staticCast(media) );
    }
    else 
    {
      //! Recursive !!
      for (int i = 0; i < media->childCount(); i++) {
        result.append( getItemChildrenTracks(media->child(i)) );
      }
    }

    return result;
}

QList<MEDIA::TrackPtr> LocalTrackModel::getAlbumChildrenTracksGenre(const MEDIA::AlbumPtr album,const QString& genre)
{
    QList<MEDIA::TrackPtr> result;

    if(!album) return result;
    
    if(album->type() != TYPE_ALBUM) return result;

    for (int i = 0; i < album->childCount(); i++) 
    {
      MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast( album->child(i) );
      
      if (!QRegExp(genre, Qt::CaseSensitive).exactMatch(track->genre)) continue;
      
      SearchEngine se;
      bool match = se.mediaMatch(
                       qvariant_cast<MediaSearch>(m_search), 
                       track );
      if(match)
        result.append( track );      
    }
    return result;
}


//! ----------------------- Get rating method ----------------------------------
float LocalTrackModel::getItemAutoRating(const MEDIA::MediaPtr media)
{
    float auto_rating = 0.0;
    int   count = 0;
    if(!media) return auto_rating;

    if (media->type() == TYPE_TRACK)
    {
        //Debug::debug() << " => getItemAutoRating : return track rating = " << media->data.rating;
        return MEDIA::TrackPtr::staticCast(media)->rating;
    }
    else {
      //! Recursive !!
      for (/*count*/; count < media->childCount(); count++)
      {
        auto_rating += getItemAutoRating( media->child(count) );
      }
    }

    float result =  media->childCount()!=0 ? float(auto_rating/(media->childCount())) : 0 ;
    result = double(int(result * 5 * 2 + 0.5)) / (5 * 2);
    //Debug::debug() << " => getItemAutoRating : rating = " << result;
    return result;
}


//! ------------------------- filtering method ---------------------------------
bool LocalTrackModel::isMediaMatch(MEDIA::MediaPtr media)
{
    if(m_search.isNull())
      return true;

    if (media->type() == TYPE_TRACK)
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
      

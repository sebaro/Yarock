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

#include "histo_model.h"
#include "local_track_model.h"

#include "core/mediaitem/mediaitem.h"
#include "core/mediasearch/media_search_engine.h"
#include "core/database/database.h"
#include "debug.h"


//! Qt
#include <QFile>
#include <QSqlQuery>
#include <QSqlDriver>

HistoModel* HistoModel::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class HistoModel                                                          *
*                                                                              *
********************************************************************************
*/
HistoModel::HistoModel(QObject *parent) : QObject(parent)
{
    //Debug::debug() << "      [HistoModel] creation";
    INSTANCE            = this;
}

void HistoModel::clear()
{
    foreach(MEDIA::TrackPtr track, m_tracks)
      track.reset();

    m_tracks.clear();
}


//! ------------------ HistoModel::updateModel ---------------------------------
void HistoModel::updateModel()
{
    Debug::debug() << "      [HistoModel] updateModel";
    this->clear();

    if (!Database::instance()->open())
        return;

    QSqlQuery query("SELECT id,url,name,date,track_id FROM view_histo ORDER BY date DESC",*Database::instance()->db());

    while (query.next())
    {
      QVariant track_id = query.value(4);
      QString url       = query.value(1).toString();

      if(!track_id.isNull())
      {
          // increment pointer counter of mediaItem
          MEDIA::TrackPtr media = MEDIA::TrackPtr(
               LocalTrackModel::instance()->trackItemHash.value(track_id.toInt())
               );

          if(media)
            media->lastPlayed = (query.value(3).isNull() ? -1 : query.value(3).toInt());

          addItem(media);
      }
      else
      {
          //! track or stream not id database
          if(MEDIA::isLocal(url))
          {
            // do not search in file : to long for histo model !!
            MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());
            media->id           = -1;
            media->url          = query.value(1).toString();
            media->title        = query.value(2).toString();
            media->lastPlayed   = (query.value(3).isNull() ? -1 : query.value(3).toInt());

            //! default value
            media->isPlaying    =  false;
            media->isBroken     =  !QFile::exists(url);
            media->isPlayed     =  false;
            media->isStopAfter  =  false;
            addItem(media);
          }
          else
          {
            MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());
            media->setType(TYPE_STREAM);
            media->id          = -1;
            media->url         = query.value(1).toString();
            media->extra["station"]  = query.value(2).toString();
            media->lastPlayed  = (query.value(3).isNull() ? -1 : query.value(3).toInt());
            media->isFavorite  = false;
            media->isPlaying   = false;
            media->isBroken    = false;
            media->isPlayed    = false;
            media->isStopAfter = false;
            addItem(media);
          }
      }
    } // end while
}

void HistoModel::addItem(MEDIA::TrackPtr media)
{
    if( !media.isNull() ) {
      m_mutex.lock();
      m_tracks.append(media);
      m_mutex.unlock();
    }
}


//!------------HistoModel::trackAt ---------------------------------------------
MEDIA::TrackPtr HistoModel::trackAt(int row) const
{
    if (row < 0 || row >= m_tracks.size())
      return MEDIA::TrackPtr(0);

    return m_tracks.at(row);
}


//! ------------------------- filtering method ---------------------------------
bool HistoModel::isMediaMatch(MEDIA::MediaPtr media)
{ 
    if(m_search.isNull())
      return true;  

    SearchEngine se;
    bool match = se.mediaMatch(
                     qvariant_cast<MediaSearch>(m_search),
                     MEDIA::TrackPtr::staticCast(media) );
    return match;
}
    

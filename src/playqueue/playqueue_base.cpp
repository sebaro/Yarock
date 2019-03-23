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

#include "playqueue_base.h"
#include "playqueue_proxymodel.h"

#include "utilities.h"      // pour randomInt
#include "settings.h"
#include "debug.h"

#include "core/mediaitem/mediaitem.h"

enum RepeatMode {RepeatOff = 0,RepeatTrack = 1, RepeatAll = 2 };
enum ShuffleMode {ShuffleOff = 0 ,ShuffleOn = 1};
    
/*
******************************************************************************************
*                                                                                        *
*    Class PlayqueueBase                                                                 *
*                                                                                        *
******************************************************************************************
*/
PlayqueueBase::PlayqueueBase()
{
    m_playing_track   = MEDIA::TrackPtr(0);
    m_requested_track = MEDIA::TrackPtr(0);
    
    m_proxy_model     =  0;
}

    
void PlayqueueBase::clear(bool initall)
{
    m_tracks.clear();

    if(initall) {
      m_playing_track   = MEDIA::TrackPtr(0);
      m_requested_track = MEDIA::TrackPtr(0);
    }
}



void PlayqueueBase::addTrack(const MEDIA::TrackPtr track)
{
     m_tracks << MEDIA::TrackPtr(track);
}


void PlayqueueBase::addTracks(QList<MEDIA::TrackPtr> tracks)
{
     m_tracks << tracks;
}


void PlayqueueBase::insertTrack(const MEDIA::TrackPtr track, int pos)
{
    const int start = (pos == -1) ? m_tracks.size() : pos;
    m_tracks.insert(start, track);
}
    
   
    
MEDIA::TrackPtr PlayqueueBase::removeTrackAt(int i)
{
    if(rowExists(i))
      return m_tracks.takeAt(i);
      
    return MEDIA::TrackPtr(0);
}


void PlayqueueBase::setPlayingTrack(MEDIA::TrackPtr tk)
{
    m_playing_track  = tk;
}


MEDIA::TrackPtr PlayqueueBase::playingTrack() const
{
    return m_playing_track;
}


int PlayqueueBase::rowForTrack(const MEDIA::TrackPtr track)
{
    return m_tracks.indexOf(track);
}


bool PlayqueueBase::filterContainsRow(int source_row)
{
Q_UNUSED(source_row)  
    if (source_row < 0 || source_row >= m_tracks.size())
      return false;
    
    if(m_proxy_model)
      return m_proxy_model->filterAcceptsRow(source_row);
    
    return true;
}


MEDIA::TrackPtr PlayqueueBase::nextTrack()
{
    RepeatMode  repeat_m  = (RepeatMode)SETTINGS()->_repeatMode;
    ShuffleMode shuffle_m = (ShuffleMode)SETTINGS()->_shuffleMode;
    int activerow = rowForTrack( playingTrack() );
    int i = -1;

    if(!filterContainsRow(activerow)) {
     // do nothing and return null track  
    }
    //! normal mode get next track
    else if(repeat_m == RepeatOff && shuffle_m == ShuffleOff) {
        i = skipForward();
    }
    //! repeat track mode
    else if(repeat_m == RepeatTrack) {
        i = activerow;
    }    
    //! repeat all mode
    else if(repeat_m == RepeatAll && shuffle_m == ShuffleOff) {
        i = skipForward(true);
    }
    //! shuffle ==> random prev until all is played
    else if(shuffle_m == ShuffleOn) {
      QList<MEDIA::TrackPtr> list = unplayedTracks();
      
      if(!list.isEmpty()) {
        int index = UTIL::randomInt(0, list.size() -1);
        MEDIA::TrackPtr track = list.at(index);
        i = rowForTrack(track);
      }
    }
    
    setRequestedTrackAt(i);
    return requestedTrack(); 
}


MEDIA::TrackPtr PlayqueueBase::prevTrack()
{
    RepeatMode  repeat_m  = (RepeatMode)SETTINGS()->_repeatMode;
    ShuffleMode shuffle_m = (ShuffleMode)SETTINGS()->_shuffleMode;
    int activerow = rowForTrack( playingTrack() );
    int i = -1;

    if(!filterContainsRow(activerow)) {
     // do nothing and return null track  
    }
    //! normal mode get prev track
    else if(repeat_m == RepeatOff && shuffle_m == ShuffleOff) {
        i = skipBackward();
    }
    //! repeat track mode
    else if(repeat_m == RepeatTrack) {
        i = activerow;
    }
    //! repeat all mode
    else if(repeat_m == RepeatAll && shuffle_m == ShuffleOff) {
        i = skipBackward(true);
    }    
    //! shuffle ==> random prev until all is played
    else if(shuffle_m == ShuffleOn) {
      QList<MEDIA::TrackPtr> list = unplayedTracks();
      if(!list.isEmpty()) {
        int index = UTIL::randomInt(0, list.size() -1);
        MEDIA::TrackPtr track = list.at(index);
        i = rowForTrack(track);
      }
    }

    setRequestedTrackAt(i);
    return requestedTrack(); 
}


int PlayqueueBase::skipBackward(bool repeat /*=false*/)
{
   /* tant que l'on ne trouve pas de ligne située avant presente dans le proxy */
   int i = rowForTrack(m_requested_track); /* source row requested */ 
   --i;
   while(i >= 0 && !filterContainsRow(i))
     --i;
   
   if(repeat && i == -1) {
     int j = m_tracks.size() -1;
     while(j > i && !filterContainsRow(j))
       --j;
       
     i = j;
   }
   return i; 
}


int PlayqueueBase::skipForward(bool repeat /*=false*/)
{
   /* tant que l'on ne trouve pas de ligne située aprés presente dans le proxy */
   int i = rowForTrack(m_requested_track); /* source row requested */ 
   int source_row  = i;
   ++i;
   while(i < m_tracks.size() && !filterContainsRow(i))
     ++i;

   if(i >= m_tracks.size()) i = -1;
   
   if(repeat && i == -1) {
     int j = 0;
     while(j < source_row && !filterContainsRow(j))
       ++j;

     i = j;
   }
   return i;    
}


void PlayqueueBase::setRequestedTrackAt(int idx)
{
    if(idx >= 0 && idx < m_tracks.size())
    {
      m_requested_track = m_tracks.at(idx);
    }
    else
    {
      m_requested_track = MEDIA::TrackPtr(0);
    }
}


MEDIA::TrackPtr PlayqueueBase::requestedTrack()
{
    return m_requested_track;
}


QList<MEDIA::TrackPtr>  PlayqueueBase::unplayedTracks()
{
    QList<MEDIA::TrackPtr> list;

    foreach (MEDIA::TrackPtr track, m_tracks)
    {
        if(!track->isPlayed && filterContainsRow(rowForTrack(track)) )
          list << track;
    }

    return list;
}


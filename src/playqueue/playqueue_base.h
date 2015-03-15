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

#ifndef _PLAYQUEUE_BASE_H_
#define _PLAYQUEUE_BASE_H_

#include "core/mediaitem/mediaitem.h"

#include <QList>
/*
******************************************************************************************
*                                                                                        *
*    Class PlayqueueBase                                                                 *
*                                                                                        *
******************************************************************************************
*/
class PlayqueueProxyModel;

class PlayqueueBase
{
  public:
    PlayqueueBase();
      
    void clear(bool initall=true);

    /* insertion method */
    void addTrack(const MEDIA::TrackPtr track);
    void addTracks(QList<MEDIA::TrackPtr> tracks);
    void insertTrack(const MEDIA::TrackPtr track, int pos=-1);
    MEDIA::TrackPtr removeTrackAt(int i);

    /* getters */
    QList<MEDIA::TrackPtr> tracks() const {return m_tracks;}    
    MEDIA::TrackPtr requestedTrack();    
    MEDIA::TrackPtr playingTrack();    
    MEDIA::TrackPtr nextTrack();
    MEDIA::TrackPtr prevTrack();
    int rowForTrack(MEDIA::TrackPtr track);
    bool rowExists( int row ) const { return (( row >= 0 ) && ( row < m_tracks.size() ) ); }
    int size() const {return m_tracks.size();}

    /* playing method */
    void    setPlayingTrack(MEDIA::TrackPtr tk);
    QList<MEDIA::TrackPtr> unplayedTracks();
    void setRequestedTrackAt(int row);
    
    /* proxy & filter */
    bool filterContainsRow(int source_row);
    void setProxy(PlayqueueProxyModel* proxy) {m_proxy_model = proxy;}
    PlayqueueProxyModel* proxy() {return m_proxy_model;}

  private :
    int skipForward(bool repeat  = false);
    int skipBackward(bool repeat = false);
    
  private :
    QList<MEDIA::TrackPtr>    m_tracks;
    MEDIA::TrackPtr           m_requested_track;
    MEDIA::TrackPtr           m_playing_track;
    
    PlayqueueProxyModel       *m_proxy_model;    
};

#endif // _PLAYQUEUE_BASE_H_

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

#ifndef _VIRTUAL_PLAYQUEUE_H_
#define _VIRTUAL_PLAYQUEUE_H_

#include <QObject>
#include <QList>

#include "core/mediaitem/mediaitem.h"
#include "playqueue/playqueue_base.h"
/*
******************************************************************************************
*                                                                                        *
*    Class VirtualPlayqueue                                                              *
*                                                                                        *
******************************************************************************************
*/
class VirtualPlayqueue : public QObject, public PlayqueueBase
{
static VirtualPlayqueue* INSTANCE;

Q_OBJECT
  public:
    VirtualPlayqueue(QObject *parent=0);
    static VirtualPlayqueue* instance() { return INSTANCE; }
  
    void addTracksAndPlayAt(QList<MEDIA::TrackPtr> tracks, int startIdx=0);
    void addTrackAndPlay(const MEDIA::TrackPtr track);
    
    void updatePlayingItem(MEDIA::TrackPtr);
    
  signals:
    void signal_collection_playTrack();
    void signal_playing_status_change();
};

#endif // _VIRTUAL_PLAYQUEUE_H_

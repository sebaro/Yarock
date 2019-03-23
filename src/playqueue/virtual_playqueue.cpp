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

#include "virtual_playqueue.h"
#include "debug.h"

VirtualPlayqueue* VirtualPlayqueue::INSTANCE = 0;

/*
******************************************************************************************
*                                                                                        *
*    Class VirtualPlayqueue                                                              *
*                                                                                        *
******************************************************************************************
*/
VirtualPlayqueue::VirtualPlayqueue(QObject *parent) : QObject(parent)
{
    INSTANCE        = this;
}


/*****************************************************************************************
    VirtualPlayqueue::addTracksAndPlayAt
*****************************************************************************************/
void VirtualPlayqueue::addTracksAndPlayAt(QList<MEDIA::TrackPtr> tracks, int startIdx)
{
    //Debug::debug() << "[VirtualPlayqueue] addTracksAndPlayAt";
    if (tracks.isEmpty()) 
      return;

    PlayqueueBase::clear();
    PlayqueueBase::addTracks(tracks);
    PlayqueueBase::setRequestedTrackAt(startIdx);
    emit signal_collection_playTrack();
}

/*****************************************************************************************
    VirtualPlayqueue::addTrackAndPlay
*****************************************************************************************/
void VirtualPlayqueue::addTrackAndPlay(const MEDIA::TrackPtr track)
{
    //Debug::debug() << "[VirtualPlayqueue] addTrackAndPlay";
    if (!track) 
      return;

    PlayqueueBase::clear();
    PlayqueueBase::addTrack(track);
    PlayqueueBase::setRequestedTrackAt(0);
    emit signal_collection_playTrack();
}

/*****************************************************************************************
    VirtualPlayqueue::updatePlayingItem
*****************************************************************************************/
void VirtualPlayqueue::updatePlayingItem(MEDIA::TrackPtr tk)
{
    Debug::debug() << "    [VirtualPlayqueue] updatePlayingItem";
    PlayqueueBase::setPlayingTrack(tk);
    emit signal_playing_status_change();
}


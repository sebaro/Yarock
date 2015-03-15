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
#ifdef ENABLE_VLC
#ifndef _ENGINE_VLC_H_
#define _ENGINE_VLC_H_

#include <QtCore/QObject>
#include "engine_base.h"

struct libvlc_event_t;
struct libvlc_media_player_t;
struct libvlc_event_manager_t;

class VlcLib;
class VlcMedia;
/*
********************************************************************************
*                                                                              *
*    Class EngineVlc                                                          *
*                                                                              *
********************************************************************************
*/
class EngineVlc : public EngineBase
{
Q_OBJECT
public:
    EngineVlc();
    ~EngineVlc();
    
    /* play/pause/stop */
    void play();
    void pause();
    void stop();
    
    /* media management */
    void setMediaItem(MEDIA::TrackPtr track);
    void setNextMediaItem(MEDIA::TrackPtr track);
    
    /* audio */ 
    int volume() const;
    void setVolume(const int &);
    bool isMuted() const;
    void setMuted( bool mute );
    
    /* time */
    void seek( qint64 );
    
private:
    void setAudioOutput();
    void createCoreConnections();
    void removeCoreConnections();
    static void libvlc_callback(const libvlc_event_t *event,void *data);
    void setVlcMedia(const QString&);
    void update_total_time();

private slots:
    void slot_on_media_change();
    void slot_on_duration_change(qint64);
    void slot_on_time_change(qint64);
    void slot_on_media_finished();
    void slot_on_media_about_to_finish();
    void slot_on_metadata_change();
    
private:
    VlcLib                    *m_vlclib;
    VlcMedia                  *m_vlc_media;
    libvlc_media_player_t     *m_vlc_player;
    libvlc_event_manager_t    *m_vlc_events;
};

#endif // _ENGINE_VLC_H_
#endif // ENABLE_VLC
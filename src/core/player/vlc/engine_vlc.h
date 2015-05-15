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
#include "vlc/libvlc_version.h"


struct libvlc_event_t;
struct libvlc_media_player_t;
struct libvlc_event_manager_t;
#if (LIBVLC_VERSION_INT >= LIBVLC_VERSION(2, 2, 0, 0))
struct libvlc_equalizer_t;
#endif
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
Q_INTERFACES(EngineBase)
#if QT_VERSION >= 0x050000
Q_PLUGIN_METADATA(IID "EngineVlc")
#endif 

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
    
    /* effect */ 
#if (LIBVLC_VERSION_INT >= LIBVLC_VERSION(2, 2, 0, 0))    
    bool isEqualizerAvailable();
    void addEqualizer();
    void removeEqualizer();
    void applyEqualizer(QList<int>);    
#endif
    
public slots:
    void volumeMute( );
    void volumeInc( );
    void volumeDec( );
    
private:
    void applyInternalVolume();
    void applyInternalMute();
    void createCoreConnections();
    void removeCoreConnections();
    static void libvlc_callback(const libvlc_event_t *event,void *data);
    void setVlcMedia(const QString&);
    void update_total_time();
    void loadEqualizerSettings();
    
private slots:
    void slot_on_media_change();
    void slot_on_duration_change(qint64);
    void slot_on_time_change(qint64);
    void slot_on_media_finished();
    void slot_on_media_about_to_finish();
    void slot_on_metadata_change();
    void internal_vlc_stateChanged(ENGINE::E_ENGINE_STATE state);
    
private:
    VlcLib                    *m_vlclib;
    VlcMedia                  *m_vlc_media;
    libvlc_media_player_t     *m_vlc_player;
    libvlc_event_manager_t    *m_vlc_events;
#if (LIBVLC_VERSION_INT >= LIBVLC_VERSION(2, 2, 0, 0))
    libvlc_equalizer_t        *m_equalizer;
#else
    void                      *m_equalizer;
#endif
    int                        m_internal_volume;
    bool                       m_is_volume_changed;
    
    bool                       m_internal_is_mute;
    bool                       m_is_muted_changed;
};

#endif // _ENGINE_VLC_H_
#endif // ENABLE_VLC
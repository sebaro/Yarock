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

#include <vlc/vlc.h>

#include "engine_vlc.h"
#include "vlc/vlc_lib.h"
#include "vlc/vlc_media.h"
#include "debug.h"

// like phonon
static const int ABOUT_TO_FINISH_TIME = 2000;
static const int TICK_INTERVAL        = 100;

/*
********************************************************************************
*                                                                              *
*    Class EngineVlc                                                           *
*                                                                              *
********************************************************************************
*/
EngineVlc::EngineVlc() : EngineBase("vlc")
{
    //Debug::debug() << "[PLAYER] create";

    /* create vlc lib instance */
    m_vlclib = new VlcLib();
    
    if(!m_vlclib->init()) {
      Debug::warning() << "[PLAYER] warning vlc initialisation failed !";
      return;
    }

    /* create vlc media player */
    m_vlc_player = libvlc_media_player_new(m_vlclib->core());
    m_vlc_events = libvlc_media_player_event_manager(m_vlc_player);
 
    /* Disable mouse and keyboard events */
    libvlc_video_set_key_input(m_vlc_player, false);
    libvlc_video_set_mouse_input(m_vlc_player, false);
    
    if(VlcLib::isError())
      VlcLib::print_error();
    else
      Debug::debug() << "[PLAYER] vlc initialisation OK !";
    
    
    m_vlc_media    = 0;
    m_tickInterval = TICK_INTERVAL;
    
    /* internal vlc connection */
    createCoreConnections();
    
    /* audio output */
    setAudioOutput();
}

EngineVlc::~EngineVlc()
{
    Debug::debug() << "[PLAYER] delete";
  
    removeCoreConnections();

    libvlc_media_player_release(m_vlc_player);

    VlcLib::print_error();
}

/* ---------------------------------------------------------------------------*/
/* EngineVlc::setAudioOutput                                                  */
/* ---------------------------------------------------------------------------*/
void EngineVlc::setAudioOutput()
{
//     QList<QByteArray> knownSoundSystems;
//     knownSoundSystems << QByteArray("pulse")
//                       << QByteArray("alsa")
//                       << QByteArray("oss")
//                       << QByteArray("jack");
// 
//     libvlc_audio_output_device_t* prefered_device;
//     QByteArray soundSystem=0;
//     foreach (soundSystem, knownSoundSystems) 
//     {
//       libvlc_audio_output_device_t* outputDevices = libvlc_audio_output_device_list_get(
//              m_vlclib->core(), soundSystem);
//       
//       while(outputDevices)
//       {
//         Debug::debug() << "found device" << soundSystem << QString::fromUtf8( outputDevices->psz_device );
//         Debug::debug() << "      " << QString::fromUtf8( outputDevices->psz_description );
// 
//         if( soundSystem== QByteArray("alsa") && 
//             QString::fromUtf8( outputDevices->psz_device ) == QString("sysdefault:CARD=Audio"))
//         {
//           prefered_device = outputDevices;
// 	  break;
//         }
// 
//         outputDevices = outputDevices->p_next;
//       }
//     }    
//     libvlc_audio_output_set(m_vlc_player, soundSystem.data() );
//     libvlc_audio_output_device_set(m_vlc_player, soundSystem.data(), prefered_device->psz_device);
}

/* ---------------------------------------------------------------------------*/
/* EngineVlc::play                                                            */
/* ---------------------------------------------------------------------------*/
void EngineVlc::play()
{
    //Debug::debug() << "[PLAYER] -> play";

    libvlc_media_player_play(m_vlc_player);

    VlcLib::print_error();
}

/* ---------------------------------------------------------------------------*/
/* EngineVlc::pause                                                           */
/* ---------------------------------------------------------------------------*/
void EngineVlc::pause()
{
    //Debug::debug() << "[PLAYER] -> pause";

    if (libvlc_media_player_can_pause(m_vlc_player))
        libvlc_media_player_set_pause(m_vlc_player, true);

    VlcLib::print_error();
}

/* ---------------------------------------------------------------------------*/
/* EngineVlc::stop                                                            */
/* ---------------------------------------------------------------------------*/
void EngineVlc::stop()
{
    //Debug::debug() << "[PLAYER] -> stop";

    libvlc_media_player_stop(m_vlc_player);

    VlcLib::print_error();
    
    EngineBase::stop();
}

/* ---------------------------------------------------------------------------*/
/* Media management                                                           */
/* ---------------------------------------------------------------------------*/
void EngineVlc::setMediaItem(MEDIA::TrackPtr track)
{
    Debug::debug() << "[PLAYER] -> setMediaItem";
    libvlc_media_player_stop(m_vlc_player);

    if(m_currentMediaItem) {
      MEDIA::registerTrackPlaying(m_currentMediaItem, false);
      m_currentMediaItem.reset();
      m_currentMediaItem = MEDIA::TrackPtr(0);
    }

    m_currentMediaItem = MEDIA::TrackPtr(track);


    if(m_nextMediaItem) {
      m_nextMediaItem.reset();
      m_nextMediaItem    = MEDIA::TrackPtr(0);
    }  

    /* checks media validity */
    if( m_currentMediaItem->isBroken )
    {
      Debug::warning() << "[PLAYER] Track path seems to be broken:" << m_currentMediaItem->url;
      stop();
      return;
    }

    /* load media */
    m_aboutToFinishEmitted = false;
    m_lastTick = 0;
    
    const QString path = MEDIA::Track::path(track->url);
    //Debug::debug() << "[PLAYER] -> set url " << path;
    
    setVlcMedia(path);    
    
    this->play();
}
 
void EngineVlc::setNextMediaItem(MEDIA::TrackPtr track)
{
    if(m_nextMediaItem)
      m_nextMediaItem.reset();

    m_nextMediaItem = MEDIA::TrackPtr(track);
}

void EngineVlc::setVlcMedia(const QString& url)
{
    //Debug::debug() << "[PLAYER] -> setVlcMedia";
    if (m_vlc_media) {
        m_vlc_media->disconnect(this);
        m_vlc_media->deleteLater();
        m_vlc_media = 0;
    }
    
    m_vlc_media = new VlcMedia();
    
    m_vlc_media->init(url, MEDIA::isLocal(url));

    // Connect to Media signals. Disconnection is done at unloading.
    connect(m_vlc_media, SIGNAL(durationChanged(qint64)), this, SLOT(slot_on_duration_change(qint64)));
    connect(m_vlc_media, SIGNAL(metaDataChanged()), this, SLOT(slot_on_metadata_change()));
    
    libvlc_media_player_set_media(m_vlc_player, m_vlc_media->core());
}

/* ---------------------------------------------------------------------------*/
/* Audio management                                                           */
/* ---------------------------------------------------------------------------*/
int EngineVlc::volume() const
{
    Debug::debug() << "[PLAYER] -> volume";

    int volume_in_percent = 0;
    if (m_vlc_player) {
        volume_in_percent = libvlc_audio_get_volume(m_vlc_player);
        VlcLib::print_error();
    }
    /* return volume in percent */
    return volume_in_percent;
}

void EngineVlc::setVolume(const int &volume_in_percent)
{
    Debug::debug() << "[PLAYER] -> setVolume";
    // Don't change if volume is the same
    if (volume_in_percent != this->volume()) {
        libvlc_audio_set_volume(m_vlc_player, volume_in_percent);
        VlcLib::print_error();

        emit volumeChanged();
    }
}


bool EngineVlc:: isMuted() const
{
    Debug::debug() << "[PLAYER] -> isMuted";
   
    bool mute = libvlc_audio_get_mute(m_vlc_player);
    
    /* at startup no active ouput error -> return not muted */
    if(VlcLib::isError())
    {
      mute = false;
      VlcLib::print_error();
    }

    return mute;
}


void EngineVlc::setMuted( bool mute )
{
    Debug::debug() << "[PLAYER] -> setMuted " << mute;
//     bool vlc_muted = libvlc_audio_get_mute(m_vlc_player);
    
    libvlc_audio_set_mute(m_vlc_player, int(mute));
    VlcLib::print_error();

    emit muteStateChanged();
}

/* ---------------------------------------------------------------------------*/
/* Vlc lib callback connection                                                */
/* ---------------------------------------------------------------------------*/
void EngineVlc::createCoreConnections()
{
    QList<libvlc_event_e> list;
    list << libvlc_MediaPlayerMediaChanged
         << libvlc_MediaPlayerNothingSpecial
         << libvlc_MediaPlayerOpening
         << libvlc_MediaPlayerBuffering
         << libvlc_MediaPlayerPlaying
         << libvlc_MediaPlayerPaused
         << libvlc_MediaPlayerStopped
         << libvlc_MediaPlayerForward
         << libvlc_MediaPlayerBackward
         << libvlc_MediaPlayerEndReached
         << libvlc_MediaPlayerEncounteredError
         << libvlc_MediaPlayerTimeChanged
         << libvlc_MediaPlayerPositionChanged
         << libvlc_MediaPlayerSeekableChanged
         << libvlc_MediaPlayerPausableChanged
         << libvlc_MediaPlayerTitleChanged
         << libvlc_MediaPlayerLengthChanged;

    foreach(const libvlc_event_e &event, list) {
        libvlc_event_attach(m_vlc_events, event, libvlc_callback, this);
    }
} 

void EngineVlc::removeCoreConnections()
{
    QList<libvlc_event_e> list;
    list << libvlc_MediaPlayerMediaChanged
         << libvlc_MediaPlayerNothingSpecial
         << libvlc_MediaPlayerOpening
         << libvlc_MediaPlayerBuffering
         << libvlc_MediaPlayerPlaying
         << libvlc_MediaPlayerPaused
         << libvlc_MediaPlayerStopped
         << libvlc_MediaPlayerForward
         << libvlc_MediaPlayerBackward
         << libvlc_MediaPlayerEndReached
         << libvlc_MediaPlayerEncounteredError
         << libvlc_MediaPlayerTimeChanged
         << libvlc_MediaPlayerPositionChanged
         << libvlc_MediaPlayerSeekableChanged
         << libvlc_MediaPlayerPausableChanged
         << libvlc_MediaPlayerTitleChanged
         << libvlc_MediaPlayerLengthChanged;

    foreach(const libvlc_event_e &event, list) {
        libvlc_event_detach(m_vlc_events, event, libvlc_callback, this);
    }
}

void EngineVlc::libvlc_callback(const libvlc_event_t *event,void *data)
{
    //Debug::debug() << "[PLAYER] -> libvlc_callback";

    EngineVlc *that = (EngineVlc *)data;

    switch(event->type)
    {
    case libvlc_MediaPlayerMediaChanged:
      that->slot_on_media_change();
      break;
      
    case libvlc_MediaPlayerPlaying:
      that->m_current_state = ENGINE::PLAYING;
      break;
    case libvlc_MediaPlayerPaused:
      that->m_current_state = ENGINE::PAUSED;
      break;
    case libvlc_MediaPlayerStopped:
      that->m_current_state = ENGINE::STOPPED;
      break;
    case libvlc_MediaPlayerEncounteredError:
      that->m_current_state = ENGINE::ERROR;
      break;
      
    case libvlc_MediaPlayerEndReached:
        QMetaObject::invokeMethod(
                    that, "slot_on_media_finished",
                    Qt::QueuedConnection);
        break;      
      break;

    case libvlc_MediaPlayerTimeChanged:
        QMetaObject::invokeMethod(
                    that, "slot_on_time_change",
                    Qt::QueuedConnection,
                    Q_ARG(qint64, event->u.media_player_time_changed.new_time));      
        break;

    case libvlc_MediaPlayerSeekableChanged:
        QMetaObject::invokeMethod(
                    that, "mediaSeekableChanged",
                    Qt::QueuedConnection,
                    Q_ARG(bool, event->u.media_player_seekable_changed.new_seekable));
        break;

    case libvlc_MediaPlayerPausableChanged:
        //  emit that->pausableChanged(event->u.media_player_pausable_changed.new_pausable);
      break;
    case libvlc_MediaPlayerTitleChanged:
        //  emit that->titleChanged(event->u.media_player_title_changed.new_title);
      break;
    case libvlc_MediaPlayerLengthChanged:
        // WARNING : nothing done here because work is done according media signal
        //  emit that->lengthChanged(event->u.media_player_length_changed.new_length);
      break;

    case libvlc_MediaPlayerNothingSpecial:
    case libvlc_MediaPlayerOpening:
    case libvlc_MediaPlayerBuffering:
    case libvlc_MediaPlayerForward:
    case libvlc_MediaPlayerBackward:
    case libvlc_MediaPlayerPositionChanged:
    default:
        break;
        QString msg = QString("Unknown event: ") + QString(libvlc_event_type_name(event->type));
        Q_ASSERT_X(false, "event_cb", qPrintable(msg));
        break;
    }
   
    if(that->m_old_state != that->m_current_state)
    {
        /* emit signal engineStateChange */
        Debug::debug() << "[PLAYER] -> state change :" << that->stateToString(that->m_current_state);

        emit that->engineStateChanged();
        that->m_old_state = that->m_current_state;
    }    
}

/* ---------------------------------------------------------------------------*/
/* EngineVlc::slot_on_media_change                                            */
/*   -> private slot                                                          */
/* ---------------------------------------------------------------------------*/
void EngineVlc::slot_on_media_change()
{
    //Debug::debug() << "[PLAYER] -> slot_on_media_change"; 
    if(!m_currentMediaItem)
    {
        Debug::error() << "[PLAYER] no media set";
        return;
    }
  
    update_total_time();
  
    emit mediaChanged();
}

/* ---------------------------------------------------------------------------*/
/* EngineVlc::slot_on_media_finished                                          */
/*   -> private slot                                                          */
/* ---------------------------------------------------------------------------*/
void EngineVlc::slot_on_media_finished()
{
    //Debug::debug() << "[PLAYER] -> slot_on_media_finished"; 
    
    emit mediaFinished();
    
    if(m_nextMediaItem)
    {
      Debug::debug() << "[PLAYER] -> slot_queue_finished next mediaitem present !!";
      setMediaItem(m_nextMediaItem);
    }
    else
    {
      emit engineRequestStop();
    }
}


/* ---------------------------------------------------------------------------*/
/* EngineVlc::slot_on_duration_change                                         */
/*   -> private slot                                                          */
/* ---------------------------------------------------------------------------*/   
void EngineVlc::slot_on_duration_change(qint64 duration)
{
Q_UNUSED(duration)  
    //Debug::debug() << "[PLAYER] -> slot_on_duration_change"; 
    if(m_nextMediaItem) {
       // totalTimeChanged has been sent before currentSourceChanged
       return;
    }
    
    update_total_time();
}

/* ---------------------------------------------------------------------------*/
/* EngineVlc::update_total_time                                               */
/*   -> private slot                                                          */
/* ---------------------------------------------------------------------------*/
void EngineVlc::update_total_time()
{
    if(m_currentMediaItem && m_currentMediaItem->type() == TYPE_TRACK)
    {
      if( m_currentMediaItem->duration > 0 )
      {
        m_totalTime  = m_currentMediaItem->duration * 1000;
      }
      else
      {
        /* return current time in ms */
        m_totalTime = (qint64)libvlc_media_player_get_time(m_vlc_player);

      }
    }
    else
    {
      m_totalTime = (qint64)libvlc_media_player_get_time(m_vlc_player);
    }
    
    emit mediaTotalTimeChanged(m_totalTime);
}

/* ---------------------------------------------------------------------------*/
/* EngineVlc::slot_on_time_change                                             */
/*   -> private slot                                                          */
/* ---------------------------------------------------------------------------*/
void EngineVlc::slot_on_time_change(qint64 time)
{
    //Debug::debug() << "[PLAYER] -> slot_on_time_change";
    
    if (time + m_tickInterval >= m_lastTick) {
        m_lastTick = time;
        emit mediaTick(time);
    }
    

    if(state() == ENGINE::PLAYING) 
    {
      const qint64 totalTime = m_totalTime;
      if (totalTime > 0 && time >= totalTime - ABOUT_TO_FINISH_TIME)
        slot_on_media_about_to_finish();
    }
}

/* ---------------------------------------------------------------------------*/
/* EngineVlc::slot_on_media_about_to_finish                                   */
/*   -> private slot                                                          */
/* ---------------------------------------------------------------------------*/
void EngineVlc::slot_on_media_about_to_finish()
{
    //Debug::debug() << "[PLAYER] -> slot_on_media_about_to_finish";

    if( m_currentMediaItem && !m_currentMediaItem->isStopAfter )
    {
        /* needed to set next media item */
        if(!m_nextMediaItem) {
          if (!m_aboutToFinishEmitted) {
            m_aboutToFinishEmitted = true;
            emit mediaAboutToFinish(); // needed to set next media item  
          }      
        }
    }
}

/* ---------------------------------------------------------------------------*/
/* EngineVlc::slot_on_metadata_change                                         */
/*   -> private slot                                                          */
/* ---------------------------------------------------------------------------*/
void EngineVlc::slot_on_metadata_change()
{
    //Debug::debug() << "[PLAYER] -> slot_on_metadata_change"; 
    if(m_currentMediaItem->type() != TYPE_STREAM) 
      return;
    
    
    const QString artist     = m_vlc_media->meta(libvlc_meta_Artist);
    const QString album      = m_vlc_media->meta(libvlc_meta_Album);
    const QString title      = m_vlc_media->meta(libvlc_meta_Title);
    QString nowPlaying       = m_vlc_media->meta(libvlc_meta_NowPlaying);

    /* Streams sometimes have the artist and title munged in nowplaying */
    if (artist.isEmpty() && !nowPlaying.isEmpty())
    {
        if(nowPlaying.contains("-")) 
        {
          QStringList list = nowPlaying.split(" - ");
          m_currentMediaItem->artist = list.first();
          m_currentMediaItem->title = list.last();
        }
    }

//     Debug::debug() << "[PLAYER]  slot_on_metadata_change title: " << m_currentMediaItem->title;
//     Debug::debug() << "[PLAYER]  slot_on_metadata_change album: " << m_currentMediaItem->album;
//     Debug::debug() << "[PLAYER]  slot_on_metadata_change artist: " << m_currentMediaItem->artist;

    emit mediaMetaDataChanged();
}


/* ---------------------------------------------------------------------------*/
/* Time management                                                            */
/* ---------------------------------------------------------------------------*/
void EngineVlc::seek(qint64 milliseconds)
{
    Debug::debug() << "[PLAYER] -> seek";

    libvlc_media_player_set_time(m_vlc_player, milliseconds);

    const qint64 time  = currentTime();
    const qint64 total = currentTotalTime();

    if (time < m_lastTick)
        m_lastTick = time;

    if (time < total - ABOUT_TO_FINISH_TIME)
        m_aboutToFinishEmitted = false;    
};

#endif // ENABLE_VLC

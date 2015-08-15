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
#include "vlc/libvlc_version.h"
 
#include "settings.h" 
#include "debug.h"

#include <QtPlugin>

/* Nota callbacks come from a VLC thread. In some cases Qt fails to detect this and
   tries to invoke directly (i.e. from same thread). This can lead to thread pollution.*/
#define P_CHANGE_STATE(__state) \
    QMetaObject::invokeMethod(\
        that, "internal_vlc_stateChanged", \
        Qt::QueuedConnection, \
        Q_ARG(ENGINE::E_ENGINE_STATE, __state))
        
// like phonon
static const int ABOUT_TO_FINISH_TIME = 2000;
static const int TICK_INTERVAL        = 100;

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(enginevlc, EngineVlc) 
#endif 

/*
********************************************************************************
*                                                                              *
*    Class EngineVlc                                                           *
*                                                                              *
********************************************************************************
*/
EngineVlc::EngineVlc() : EngineBase("vlc")
{
    qRegisterMetaType<ENGINE::E_ENGINE_STATE>("ENGINE::E_ENGINE_STATE");

    /* create vlc lib instance */
    m_vlclib = new VlcLib();
    
    if(!m_vlclib->init()) {
      Debug::warning() << "[EngineVlc] -> warning vlc initialisation failed !";
      m_isEngineOK = false;
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
      Debug::debug() << "[EngineVlc] vlc initialisation OK !";
    
    /* internal vlc connection */
    createCoreConnections();
    
    /* internal inits */
    m_vlc_media         = 0;
    m_tickInterval      = TICK_INTERVAL;    
    
    /* internal volume & mute */
    m_internal_volume   = 75;
    m_internal_is_mute  = false;
    
    m_is_volume_changed = true;
    m_is_muted_changed  = true;
    
    /* init equalizer */
    m_equalizer = 0;
#if (LIBVLC_VERSION_INT >= LIBVLC_VERSION(2, 2, 0, 0))    
    m_equalizer = libvlc_audio_equalizer_new();
    if( SETTINGS()->_enableEq ) {
      addEqualizer();
      loadEqualizerSettings();
    }
#endif

#if (LIBVLC_VERSION_INT >= LIBVLC_VERSION(2, 1, 0, 0))
    libvlc_media_player_set_video_title_display(m_vlc_player, libvlc_position_disable, 0);
#endif
}

EngineVlc::~EngineVlc()
{
    Debug::debug() << "[EngineVlc] -> delete";
    if( m_isEngineOK ) 
    {  
      removeCoreConnections();

      libvlc_media_player_release(m_vlc_player);

      VlcLib::print_error();
    }
}


/* ---------------------------------------------------------------------------*/
/* EngineVlc::play                                                            */
/* ---------------------------------------------------------------------------*/
void EngineVlc::play()
{
    Debug::debug() << "[EngineVlc] -> play";
    libvlc_media_player_play(m_vlc_player);
    
    VlcLib::print_error();
}

/* ---------------------------------------------------------------------------*/
/* EngineVlc::pause                                                           */
/* ---------------------------------------------------------------------------*/
void EngineVlc::pause()
{
    //Debug::debug() << "[EngineVlc] -> pause";

    if (libvlc_media_player_can_pause(m_vlc_player))
        libvlc_media_player_set_pause(m_vlc_player, true);
}

/* ---------------------------------------------------------------------------*/
/* EngineVlc::stop                                                            */
/* ---------------------------------------------------------------------------*/
void EngineVlc::stop()
{
    //Debug::debug() << "[EngineVlc] -> stop";

    libvlc_media_player_stop(m_vlc_player);

    EngineBase::stop();
}

/* ---------------------------------------------------------------------------*/
/* Media management                                                           */
/* ---------------------------------------------------------------------------*/
void EngineVlc::setMediaItem(MEDIA::TrackPtr track)
{
    Debug::debug() << "[EngineVlc] -> setMediaItem";
    libvlc_media_player_stop(m_vlc_player);

    if(m_currentMediaItem) 
    {
      MEDIA::registerTrackPlaying(m_currentMediaItem, false);
    }

    m_currentMediaItem = MEDIA::TrackPtr(track);

    if(m_nextMediaItem) 
    {
      m_nextMediaItem    = MEDIA::TrackPtr(0);
    }  

    /* checks media validity */
    if( m_currentMediaItem->isBroken )
    {
      Debug::warning() << "[EngineVlc] Track path seems to be broken:" << m_currentMediaItem->url;
      stop();
      return;
    }

    /* load media */
    m_aboutToFinishEmitted = false;
    m_lastTick = 0;
    
    const QString path = MEDIA::Track::path(track->url);
    Debug::debug() << "[EngineVlc] -> set url " << path;
    
    setVlcMedia( path );
    
    this->play();
}
 
void EngineVlc::setNextMediaItem(MEDIA::TrackPtr track)
{
    m_nextMediaItem = MEDIA::TrackPtr(track);
}

void EngineVlc::setVlcMedia(const QString& url)
{
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
    Debug::debug() << "[EngineVlc] -> volume";

    /* return volume in percent */
    return m_internal_volume;
}

void EngineVlc::setVolume(const int &volume_in_percent)
{
    Debug::debug() << "[EngineVlc] -> setVolume";
    
    /* Don't change if volume is the same */
    if( m_internal_volume != volume_in_percent )
    {
      m_internal_volume   = volume_in_percent;
      m_is_volume_changed = true;
      
      applyInternalVolume();

      emit volumeChanged();
    }
}


void EngineVlc::applyInternalVolume()
{
    Debug::debug() << "[EngineVlc] -> applyInternalVolume";
  
    /* vlc can not handle volume if no playing output is active */  
    if( m_current_state == ENGINE::PLAYING ) 
    {
        libvlc_audio_set_volume(m_vlc_player, m_internal_volume);
        
        VlcLib::print_error();
        
        m_is_volume_changed = false;
    }
}

bool EngineVlc:: isMuted() const
{
    Debug::debug() << "[EngineVlc] -> is muted";
    return m_internal_is_mute;
}

void EngineVlc::setMuted( bool mute )
{
    Debug::debug() << "[EngineVlc] -> set muted";
    if( m_internal_is_mute != mute )
    {
      m_internal_is_mute = mute;
      m_is_muted_changed = true;

      applyInternalMute();

      emit muteStateChanged();
    }
}

void EngineVlc::applyInternalMute()
{
    /* vlc can not handle mute changes if no playing output is active */  
    if( m_current_state == ENGINE::PLAYING || m_current_state == ENGINE::PAUSED )
    {
        int m = m_internal_is_mute ? 1 : 0;
        
        libvlc_audio_set_mute(m_vlc_player, int(m));
        
        VlcLib::print_error();

        m_is_muted_changed = false;
    }
}

void EngineVlc::volumeMute( ) 
{
    setMuted( !isMuted() );
};

void EngineVlc::volumeInc( ) 
{
    int percent = volume() < 100 ? volume() + 1 : 100;
    setVolume(percent);
};

void EngineVlc::volumeDec( )
{
    int percent = volume() > 0 ? volume() -1 : 0;
    setVolume(percent);
};

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
    //Debug::debug() << "[EngineVlc] libvlc_callback:" << QString(libvlc_event_type_name(event->type));
    EngineVlc *that = reinterpret_cast<EngineVlc *>(data);
    Q_ASSERT(that);

    switch(event->type)
    {
    case libvlc_MediaPlayerMediaChanged:
        QMetaObject::invokeMethod(
                    that, "slot_on_media_change",
                    Qt::QueuedConnection);
       break;
    case libvlc_MediaPlayerPlaying:
       P_CHANGE_STATE(ENGINE::PLAYING);
       break;
    case libvlc_MediaPlayerPaused:
       P_CHANGE_STATE(ENGINE::PAUSED);
       break;
    case libvlc_MediaPlayerStopped:
       P_CHANGE_STATE(ENGINE::STOPPED);
       break;
    case libvlc_MediaPlayerEncounteredError:
       P_CHANGE_STATE(ENGINE::ERROR);
       break;
      
    case libvlc_MediaPlayerEndReached:
        QMetaObject::invokeMethod(
                    that, "slot_on_media_finished",
                    Qt::QueuedConnection);
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
}


void EngineVlc::internal_vlc_stateChanged(ENGINE::E_ENGINE_STATE state)
{
    //Debug::debug() << "EngineVlc::internal_vlc_stateChanged:" << state;
    m_current_state = state;
    if(m_old_state != m_current_state)
    {
        /* emit signal engineStateChange */
        Debug::debug() << "[EngineVlc] -> state change :" << stateToString(this->m_current_state);

        m_old_state = m_current_state;

        emit engineStateChanged();
    }       
    
    if( m_is_volume_changed )
      applyInternalVolume();

    if( m_is_muted_changed )
      applyInternalMute();    
}

    
/* ---------------------------------------------------------------------------*/
/* EngineVlc::slot_on_media_change                                            */
/*   -> private slot                                                          */
/* ---------------------------------------------------------------------------*/
void EngineVlc::slot_on_media_change()
{
    Debug::debug() << "[EngineVlc] -> slot_on_media_change"; 
    if(!m_currentMediaItem)
    {
        Debug::error() << "[EngineVlc] -> no media set";
        return;
    }

    /* register track change */
    update_total_time();
    
    MEDIA::registerTrackPlaying(m_currentMediaItem, true);
 
    emit mediaChanged();
}

/* ---------------------------------------------------------------------------*/
/* EngineVlc::slot_on_media_finished                                          */
/*   -> private slot                                                          */
/* ---------------------------------------------------------------------------*/
void EngineVlc::slot_on_media_finished()
{
    Debug::debug() << "[EngineVlc] -> slot_on_media_finished";
    
    emit mediaFinished();
    
    if(m_nextMediaItem)
    {
      Debug::debug() << "[EngineVlc] -> slot_queue_finished next mediaitem present !!";
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
    //Debug::debug() << "[EngineVlc] -> slot_on_duration_change"; 
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
    Debug::debug() << "[EngineVlc] -> update_total_time"; 
  
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
    if (time + m_tickInterval >= m_lastTick ||
        time - m_tickInterval <= m_lastTick)
    {
        m_lastTick = time;
        emit mediaTick(time);
    }
    

    if(m_current_state == ENGINE::PLAYING) 
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
    //Debug::debug() << "[EngineVlc] -> slot_on_media_about_to_finish";

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
    Debug::debug() << "[EngineVlc] -> slot_on_metadata_change"; 
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

    emit mediaMetaDataChanged();
}


/* ---------------------------------------------------------------------------*/
/* Time management                                                            */
/* ---------------------------------------------------------------------------*/
void EngineVlc::seek(qint64 milliseconds)
{
    Debug::debug() << "[EngineVlc] -> seek";

    libvlc_media_player_set_time(m_vlc_player, milliseconds);

    const qint64 time  = currentTime();
    const qint64 total = currentTotalTime();

    if (time < total - ABOUT_TO_FINISH_TIME)
        m_aboutToFinishEmitted = false;    
};


/* ---------------------------------------------------------------------------*/
/* Equalizer management                                                       */
/* ---------------------------------------------------------------------------*/
#if (LIBVLC_VERSION_INT >= LIBVLC_VERSION(2, 2, 0, 0))
bool EngineVlc::isEqualizerAvailable() 
{
    return true;
}

void EngineVlc::addEqualizer()
{
    libvlc_media_player_set_equalizer(m_vlc_player, m_equalizer);
}


void EngineVlc::removeEqualizer()
{
    libvlc_media_player_set_equalizer(m_vlc_player, NULL);
}

void EngineVlc::applyEqualizer(QList<int> gains)
{
    Debug::debug() << "[EngineVlc] -> apply equalizer settings";
  
    int u_band=-1;
    double scaled_value;
    foreach( int gain, gains )
    {
      scaled_value = gain;
      
      if (u_band == -1 /* for preamp */)
        libvlc_audio_equalizer_set_preamp(m_equalizer, scaled_value);
      else
        libvlc_audio_equalizer_set_amp_at_index(m_equalizer, scaled_value, u_band);
      
       u_band++;
    }
}

void EngineVlc::loadEqualizerSettings()
{
    const QString preset_name = SETTINGS()->_currentPreset;

    if( SETTINGS()->_presetEq.keys().contains(preset_name) ) {
      QList<int> gains;

      gains << SETTINGS()->_presetEq[preset_name].preamp;
      for (int i=0 ; i<Equalizer::kBands ; ++i)
        gains << SETTINGS()->_presetEq[preset_name].gain[i];

      applyEqualizer(gains);
    }
}
#endif // libvlc > 2.2.0   

#endif // ENABLE_VLC

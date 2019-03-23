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
#ifdef ENABLE_MPV

#include "engine_mpv.h"
#include "settings.h" 
#include "debug.h"

#include <QCoreApplication>
#include <QtPlugin>

//! used for setlocale function needed for libmpv
#include <locale.h>  

// like phonon
namespace MPV {
static const int ABOUT_TO_FINISH_TIME = 2000;
static const int TICK_INTERVAL        = 100;
};

#define MPV_CLIENT_API_VERSION_STRING QString::number(MPV_CLIENT_API_VERSION >> 16) + "." + QString::number(MPV_CLIENT_API_VERSION & 0x0000FFFF ) 

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(enginempv, EngineMpv) 
#endif 

/*
********************************************************************************
*                                                                              *
*    Class EngineMpv                                                           *
*                                                                              *
********************************************************************************
*/
static void wakeup(void *ctx)
{
    EngineMpv *engine = (EngineMpv*)ctx;
    
    QCoreApplication::postEvent(engine, new QEvent(QEvent::User));
}

EngineMpv::EngineMpv() : EngineBase("mpv")
{
    m_type = ENGINE::MPV;

    setlocale(LC_NUMERIC, "C"); 
     
    /* ----- create mpv instance ----- */
    m_mpv_core = mpv_create();

    if( !m_mpv_core ) {
      Debug::warning() << "[EngineMpv] warning mpv creation failed !";
      return;
    }

    /* ----- core mpv setup  [see mpv file DOCS/man/options.rst] ----- */
    mpv_set_option_string(m_mpv_core, "config", "no");    
    mpv_set_option_string(m_mpv_core, "audio-display", "no");          /* do not show image for audio tracks */

    mpv_set_option_string(m_mpv_core, "gapless-audio", "yes") ;        /* force gapless playback */
    
    mpv_set_option_string(m_mpv_core, "vo", "null");                   /* disable video */

    mpv_set_option_string(m_mpv_core, "idle", "yes");                  /* mpv wait idly instead of quitting */

    mpv_set_option_string(m_mpv_core, "input-default-bindings", "no"); /* no key binding */
    mpv_set_option_string(m_mpv_core, "input-vo-keyboard", "no");      /* no keyboard inputs */
    mpv_set_option_string(m_mpv_core, "input-cursor", "no");           /* no mouse handling */
    mpv_set_option_string(m_mpv_core, "ytdl", "no");                   /* no youtube-dl */
    mpv_set_option_string(m_mpv_core, "fs", "no");                     /* no fullscreen */
    mpv_set_option_string(m_mpv_core, "osd-level", "0");               /* no OSD */
    mpv_set_option_string(m_mpv_core, "quiet", "yes");                 /* console output less verbose */

    mpv_set_option_string(m_mpv_core, "softvol", "yes");               /* use mpv internal vol */
    mpv_set_option_string(m_mpv_core, "softvol-max", "150.0");         /* max vol */

    mpv_set_option_string(m_mpv_core, "audio-client-name", "yarock");
    
        
    /* ----- log message (none, info, trace) ----- */
#ifdef TEST_FLAG 
    mpv_request_log_messages(m_mpv_core, "v");
#else
    mpv_request_log_messages(m_mpv_core, "info");    
#endif    

    /* ----- setup callback event handling ----- */
    mpv_set_wakeup_callback(m_mpv_core, wakeup, this);
    
    m_tickInterval = MPV::TICK_INTERVAL;
       
    /* ----- core mpv init ----- */
    if( mpv_initialize(m_mpv_core) < 0 ) {
      Debug::warning() << "[EngineMpv] warning mpv initialisation failed !";
      m_isEngineOK = false;
      return;
    }
      
    /* ----- get updates when these properties change ----- */
    mpv_observe_property(m_mpv_core, 1, "pause",  MPV_FORMAT_FLAG);      

    /* ----- internal volume & mute ----- */
    m_internal_volume = -1;
    int restoredVolume = SETTINGS()->_volumeLevel > this->maxVolume() ? this->maxVolume() : SETTINGS()->_volumeLevel;
    setVolume( restoredVolume );
    
    m_internal_is_mute  = true;
    setMuted(false);
    
    Debug::debug() << "[EngineMpv] libmpv client version:" << MPV_CLIENT_API_VERSION_STRING;
    m_version = MPV_CLIENT_API_VERSION_STRING;
}

EngineMpv::~EngineMpv()
{
    Debug::debug() << "[EngineMpv] delete";

    if(m_mpv_core)
    {
        mpv_terminate_destroy(m_mpv_core);
        m_mpv_core = 0;
    }
}

bool EngineMpv::event(QEvent *event)
{
    if(event->type() == QEvent::User)
    {
        while(m_mpv_core)
        {
            mpv_event *m_event = mpv_wait_event(m_mpv_core, 0);
    
            if( !m_event || m_event->event_id == MPV_EVENT_NONE)
            {
                break;
            }

            if( m_event->error < 0 )
              Debug::warning() << mpv_error_string(m_event->error);

            if (m_event->event_id == MPV_EVENT_LOG_MESSAGE)
            {
              struct mpv_event_log_message *msg = (struct mpv_event_log_message *)m_event->data;
              Debug::debug() << "[" << msg->prefix << "] " << msg->level << ": " << msg->text;
            }    
            else if (m_event->event_id == MPV_EVENT_PROPERTY_CHANGE)
            {
              handle_mpv_property_change( m_event );
            }
            else if (m_event->event_id == MPV_EVENT_START_FILE)
            {
              Debug::debug() << "[EngineMpv] event MPV_EVENT_START_FILE";
              m_totalTime = 0;
              m_lastTick  = 0;
      
              on_media_change();  

            }
            else if (m_event->event_id == MPV_EVENT_END_FILE)
            {
              struct mpv_event_end_file *eof_event = (struct mpv_event_end_file*)m_event->data;

#if ( MPV_CLIENT_API_VERSION >= MPV_MAKE_VERSION(1, 9) )
              Debug::debug() << "[EngineMpv] event MPV_EVENT_END_FILE REASON:" << eof_event->reason;
              if( eof_event->reason ==  MPV_END_FILE_REASON_EOF)
                  on_media_finished();
              else if (eof_event->reason ==  MPV_END_FILE_REASON_ERROR)
                  on_media_error();                  
#else                  
              on_media_finished();
#endif
            }
        }
        return true;
    }
    return QObject::event(event);
}


void EngineMpv::handle_mpv_property_change(mpv_event *event)
{
    mpv_event_property *prop = (mpv_event_property*)event->data;
// #ifdef TEST_FLAG
//     Debug::debug() << "EngineMpv::handle_mpv_property_change:" << QString(prop->name);
// #endif    

    /* on pause change */
    if(QString(prop->name) == "pause")
    {
       if(prop->format == MPV_FORMAT_FLAG) 
       {
         int paused = *(int *)prop->data;

         if (paused)
         {
           this->m_current_state = ENGINE::PAUSED;
         }
         else if (this->m_current_state != ENGINE::STOPPED )
         {
           this->m_current_state = ENGINE::PLAYING;
         }
       }
    }
    /* on time change */
    else if(QString(prop->name) == "time-pos")
    {
       if(prop->format == MPV_FORMAT_DOUBLE)
       {
         double time = (*(double*)prop->data) * 1000; /* sec to mili */

         if (time + m_tickInterval >= m_lastTick || time - m_tickInterval <= m_lastTick)
         {
             m_lastTick = time;

             emit mediaTick(time);

             if (m_totalTime > 0 && m_lastTick >= m_totalTime - MPV::ABOUT_TO_FINISH_TIME)
                 on_media_about_to_finish();
         }
       }
    }
    /* on duration change */
    else if(QString(prop->name) == "length")
    {
        if(!m_nextMediaItem && m_currentMediaItem)
          update_total_time();
    }
    else if (QString(prop->name) == "metadata")
    {
        on_metadata_change();
    }
    
                
    if(this->m_old_state != this->m_current_state)
    {
        /* emit signal engineStateChange */
        Debug::debug() << "[EngineMpv] -> state change :" << this->stateToString(this->m_current_state);

        emit engineStateChanged();
        this->m_old_state = this->m_current_state;
    }        
    
    if( m_is_volume_changed )
      applyInternalVolume();

    if( m_is_muted_changed )
      applyInternalMute();
}


/* ---------------------------------------------------------------------------*/
/* EngineMpv::play                                                            */
/* ---------------------------------------------------------------------------*/
void EngineMpv::play()
{
    Debug::debug() << "[EngineMpv] -> play";
    mpv_observe_property(m_mpv_core, 2, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv_core, 3, "length",   MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv_core, 4, "metadata", MPV_FORMAT_NODE);
    
    if(m_current_state != ENGINE::PLAYING)
    {
        int f = 0;
        mpv_set_property_async(m_mpv_core, 0, "pause", MPV_FORMAT_FLAG, &f);
    }
}

/* ---------------------------------------------------------------------------*/
/* EngineMpv::pause                                                           */
/* ---------------------------------------------------------------------------*/
void EngineMpv::pause()
{
    Debug::debug() << "[EngineMpv] -> pause"; 
  
    if(m_current_state == ENGINE::PLAYING)
    {
        int f = 1;
        mpv_set_property_async(m_mpv_core, 0, "pause", MPV_FORMAT_FLAG, &f);
    }
}


/* ---------------------------------------------------------------------------*/
/* EngineMpv::stop                                                            */
/* ---------------------------------------------------------------------------*/
void EngineMpv::stop()
{
    Debug::debug() << "[EngineMpv] -> stop"; 
    mpv_unobserve_property(m_mpv_core, 2);
    mpv_unobserve_property(m_mpv_core, 3);
    mpv_unobserve_property(m_mpv_core, 4);
    
    const char *cmd[] = {"stop", NULL}; 

    mpv_command(m_mpv_core, cmd);

    EngineBase::stop();  
}

/* ---------------------------------------------------------------------------*/
/* EngineMpv::update_total_time                                               */
/* ---------------------------------------------------------------------------*/
void EngineMpv::update_total_time()
{
Q_ASSERT(  m_currentMediaItem);
    Debug::debug() << "[EngineMpv] -> update_total_time"; 
  
    double len =0;
    if(m_currentMediaItem && m_currentMediaItem->type() == TYPE_TRACK)
    {
      if( m_currentMediaItem->duration > 0 )
      {
        m_totalTime  = m_currentMediaItem->duration * 1000;
      }
      else
      {
        mpv_get_property(m_mpv_core, "length", MPV_FORMAT_DOUBLE, &len);
      }
    }
    else
    {
        mpv_get_property(m_mpv_core, "length", MPV_FORMAT_DOUBLE, &len);
    }

    if(len != 0)
      m_totalTime = len;

    
    emit mediaTotalTimeChanged(m_totalTime);
}


/* ---------------------------------------------------------------------------*/
/* EngineMpv::on_metadata_change                                              */
/* ---------------------------------------------------------------------------*/
void EngineMpv::on_metadata_change()
{
    //Debug::debug() << "[EngineMpv] -> on_metadata_change";

    if(!m_currentMediaItem || m_currentMediaItem->type() != TYPE_STREAM )
      return;

    mpv_node node;
    mpv_get_property(m_mpv_core, "metadata", MPV_FORMAT_NODE, &node);
    
    QString title,album,artist,nowPlaying,icy_br;
    if(node.format == MPV_FORMAT_NODE_MAP)
    {
        for(int n = 0; n < node.u.list->num; n++)
        {
            if(node.u.list->values[n].format == MPV_FORMAT_STRING)
            {
                QString s_key   =  node.u.list->keys[n];
                QString s_value =  node.u.list->values[n].u.string;
//#ifdef TEST_FLAG                 
//                Debug::debug() << "[EngineMpv] -> on_metadata_change " << s_key << ":" << s_value;
//#endif                
                
                if( s_key == "title")
                  title = s_value;
                else if( s_key == "album")
                  album = s_value;
                else if ( s_key == "artist")
                  artist = s_value;
                else if ( s_key == "icy-title")
                  nowPlaying = s_value;
                else if ( s_key == "icy-br")
                  icy_br = s_value;
            }
        }
     }           
     
     
    /* Streams sometimes have the artist and title munged in nowplaying */
    if ( artist.isEmpty() && !nowPlaying.isEmpty() && nowPlaying.contains("-") )
    {
          QStringList list = nowPlaying.split("-");
          artist = list.first().trimmed();
          title = list.last().trimmed();
    }

    if( !icy_br.isEmpty() )
    {
        m_currentMediaItem->extra["bitrate"] = QString(icy_br);
    }
    else
    {
        double bitrate;
        mpv_get_property(m_mpv_core, "audio-bitrate", MPV_FORMAT_DOUBLE, &bitrate);

        if(bitrate != 0)
            m_currentMediaItem->extra["bitrate"] = bitrate;
    }
    
    double samplingrate;
    mpv_get_property(m_mpv_core, "audio-samplerate", MPV_FORMAT_DOUBLE, &samplingrate);
 
    if(samplingrate != 0)
       m_currentMediaItem->extra["samplerate"] = samplingrate;
    
    char *cformat = NULL;
    mpv_get_property(m_mpv_core, "audio-format", MPV_FORMAT_STRING, &cformat);
    m_currentMediaItem->extra["format"] = QString(cformat);
   
   
    m_currentMediaItem->title  = title;
    m_currentMediaItem->album  = album;
    m_currentMediaItem->artist = artist;

    emit mediaMetaDataChanged();
}


/* ---------------------------------------------------------------------------*/
/* EngineMpv::on_media_change                                                 */
/* ---------------------------------------------------------------------------*/
void EngineMpv::on_media_change()
{
    Debug::debug() << "[EngineMpv] -> on_media_change"; 
    
    /* test if automatic playing next track */
    /* les operations suivantes sont celles de SetMediaItem */ 
    if(m_currentMediaItem && m_nextMediaItem) 
    {
      //Debug::debug() << "[EngineMpv] -> on_media_change AUTOMATIC switching to next media item !!"; 
      MEDIA::registerTrackPlaying(m_currentMediaItem, false);
    
      m_currentMediaItem = MEDIA::TrackPtr(m_nextMediaItem);
      
      m_nextMediaItem    = MEDIA::TrackPtr(0);

      m_aboutToFinishEmitted = false;
    }
    
    if(!m_currentMediaItem)
    {
        Debug::error() << "[EngineMpv] no media set";
        return;
    }
   
    m_current_state = ENGINE::PLAYING;
    
    /* register track change */
    update_total_time();
    
    MEDIA::registerTrackPlaying(m_currentMediaItem, true);
 
    emit mediaChanged();
}

/* ---------------------------------------------------------------------------*/
/* EngineMpv::on_media_about_to_finish                                        */
/* ---------------------------------------------------------------------------*/
void EngineMpv::on_media_about_to_finish()
{
    if( m_currentMediaItem && !m_currentMediaItem->isStopAfter )
    {
        /* needed to set next media item */
        if(!m_nextMediaItem) {
          if (!m_aboutToFinishEmitted) {
            Debug::debug() << "[EngineMpv] -> on_media_about_to_finish";
            m_aboutToFinishEmitted = true;
            emit mediaAboutToFinish(); // needed to set next media item  
          }      
        }
    }
}

/* ---------------------------------------------------------------------------*/
/* EngineMpv::on_media_finished                                               */
/* ---------------------------------------------------------------------------*/
void EngineMpv::on_media_finished()
{
    Debug::debug() << "[EngineMpv] -> on_media_finished"; 
    emit mediaFinished();

    if(m_nextMediaItem)
    {
      Debug::debug() << "[EngineMpv] -> next mediaitem present !!";
      /* NOTE : nextmedia item is queued in mpv playlist queue */
      /* do not use setMediaItem to avoid stopping playback between tracks for gapless */
      //setMediaItem(m_nextMediaItem);
    }
    else
    {
       emit engineRequestStop();
    }
}

/* ---------------------------------------------------------------------------*/
/* EngineMpv::on_media_error                                                  */
/* ---------------------------------------------------------------------------*/
void EngineMpv::on_media_error()
{
      m_current_state = ENGINE::ERROR;
      emit engineStateChanged();
}

/* ---------------------------------------------------------------------------*/
/* Media management                                                           */
/* ---------------------------------------------------------------------------*/
void EngineMpv::setMediaItem(MEDIA::TrackPtr track)
{
    Debug::debug() << "[EngineMpv] -> setMediaItem";

    /* stop complet avec blocage des signaux/event */
    mpv_unobserve_property(m_mpv_core, 2);
    mpv_unobserve_property(m_mpv_core, 3);
    mpv_unobserve_property(m_mpv_core, 4);
    mpv_unobserve_property(m_mpv_core, 5);
    
    const char *cmd1[] = {"stop", NULL}; 
    mpv_command(m_mpv_core, cmd1);
    
    
    if( m_currentMediaItem )
    {
      MEDIA::registerTrackPlaying(m_currentMediaItem, false);
    }

    m_currentMediaItem = MEDIA::TrackPtr(track);
    
    if( m_nextMediaItem )
    {
      m_nextMediaItem    = MEDIA::TrackPtr(0);
    }  

    /* checks media validity */
    if( m_currentMediaItem->isBroken )
    {
      Debug::warning() << "[EngineMpv] Track path seems to be broken:" << m_currentMediaItem->url;
      stop();
      return;
    }

    /* load media */
    m_aboutToFinishEmitted = false;
    m_lastTick = 0;
    
    const QString path = MEDIA::Track::path(track->url);
    Debug::debug() << "[EngineMpv] -> set url " << path;
    
    /* load file to mpv */
    const QByteArray path_ba = path.toUtf8();
    const char *cmd[] = {"loadfile", path_ba.constData(), "replace", NULL};
    mpv_command(m_mpv_core, cmd);
    
    
    this->play();
}
 
void EngineMpv::setNextMediaItem(MEDIA::TrackPtr track)
{
    Debug::debug() << "[EngineMpv] -> setNextMediaItem";
  
    m_nextMediaItem = MEDIA::TrackPtr(track);
        
    /* try to append next file in mpv internal queue */ 
    const QString path = MEDIA::Track::path(track->url);
 
    const QByteArray path_ba = path.toUtf8();
    const char *cmd[] = {"loadfile", path_ba.constData(), "append", NULL};
    mpv_command(m_mpv_core, cmd);
}

/* ---------------------------------------------------------------------------*/
/* Audio management                                                           */
/* ---------------------------------------------------------------------------*/
int EngineMpv::volume() const
{
    Debug::debug() << "[EngineMpv] -> volume";

    /* return volume in percent */
    return m_internal_volume;
}

void EngineMpv::setVolume(const int &volume_in_percent)
{
    Debug::debug() << "[EngineMpv] -> set volume";
    
    /* Don't change if volume is the same */
    if( m_internal_volume != volume_in_percent )
    {
      m_internal_volume   = volume_in_percent;
      m_is_volume_changed = true;
      
      applyInternalVolume();

      emit volumeChanged();
    }    
}

void EngineMpv::applyInternalVolume()
{
    //Debug::debug() << "[EngineMpv] -> applyInternalVolume";
    
    /* mpv can not handle volume if no playing output is active */  
    if( m_current_state == ENGINE::PLAYING || m_current_state == ENGINE::PAUSED ) 
    {
        double volume_in_percent = m_internal_volume;
        mpv_set_property_async(m_mpv_core, 0, "volume", MPV_FORMAT_DOUBLE, &volume_in_percent);
    
        m_is_volume_changed = false;
    }
}

bool EngineMpv:: isMuted() const
{
    Debug::debug() << "[EngineMpv] -> is muted";
    return m_internal_is_mute;
}

void EngineMpv::setMuted( bool mute )
{
    Debug::debug() << "[EngineMpv] -> set muted";
    if( m_internal_is_mute != mute )
    {
      m_internal_is_mute = mute;
      m_is_muted_changed = true;

      applyInternalMute();

      emit muteStateChanged();
    }
}

void EngineMpv::applyInternalMute()
{
    /* mpv can not handle mute changes if no playing output is active */  
    if( m_current_state == ENGINE::PLAYING || m_current_state == ENGINE::PAUSED )
    {
        int m = m_internal_is_mute ? 1 : 0;
        mpv_set_property_async(m_mpv_core, 0, "mute", MPV_FORMAT_FLAG, &m);

        m_is_muted_changed = false;
    }
}


void EngineMpv::volumeMute( ) 
{
    setMuted( !isMuted() );
};

void EngineMpv::volumeInc( ) 
{
    int percent = volume() < 100 ? volume() + 1 : 100;
    setVolume(percent);
};

void EngineMpv::volumeDec( )
{
    int percent = volume() > 0 ? volume() -1 : 0;
    setVolume(percent);
};


/* ---------------------------------------------------------------------------*/
/* Time management                                                            */
/* ---------------------------------------------------------------------------*/
void EngineMpv::seek(qint64 milliseconds)
{
    const qint64 seconds=milliseconds/1000;
    const QByteArray tmp = QString::number(seconds).toUtf8();
  
    const char *cmd[] = {"seek", tmp.constData(), "absolute", NULL};
    mpv_command(m_mpv_core, cmd);       
    
    
    const qint64 time  = currentTime();
    const qint64 total = currentTotalTime();

    if (time < total - MPV::ABOUT_TO_FINISH_TIME)
        m_aboutToFinishEmitted = false;    
};

#endif // ENABLE_MPV

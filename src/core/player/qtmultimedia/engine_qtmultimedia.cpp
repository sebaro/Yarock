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
#ifdef ENABLE_QTMULTIMEDIA

// local
#include "engine_qtmultimedia.h"
#include "utilities.h"
#include "settings.h"
#include "debug.h"


// Qt
#include <QFileInfo>
// #include <QList>
#include <QtPlugin>


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(engineqtmultimedia, EngineQtMultimedia) 
#endif
/*
********************************************************************************
*                                                                              *
*    Class EngineQtMultimedia                                                  *
*                                                                              *
********************************************************************************
*/
EngineQtMultimedia::EngineQtMultimedia() : EngineBase("qtmultimedia")
{
    m_type = ENGINE::QTMULTIMEDIA;
    
    m_player = new QMediaPlayer(this, QMediaPlayer::StreamPlayback);

    connect(m_player,SIGNAL(stateChanged(QMediaPlayer::State)),this,SLOT(slot_on_player_state_change(QMediaPlayer::State)),Qt::DirectConnection);
    connect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(slot_on_media_status_changed(QMediaPlayer::MediaStatus)));
//     connect(this->mediaPlayer, SIGNAL(metaDataChanged()), this, SLOT(metaDataUpdate()));

    
    /* ----- initial volume setup ----- */
    int restoredVolume = SETTINGS()->_volumeLevel > this->maxVolume() ? this->maxVolume() : SETTINGS()->_volumeLevel;
    setVolume( restoredVolume );
        
    m_current_state    = ENGINE::STOPPED;
    m_old_state        = ENGINE::STOPPED;
    
    m_version = QString();
}

EngineQtMultimedia::~EngineQtMultimedia()
{
}


/* ---------------------------------------------------------------------------*/
/* Playing method                                                             */
/* ---------------------------------------------------------------------------*/
void EngineQtMultimedia::play()
{

}

void EngineQtMultimedia::pause()
{

}

void EngineQtMultimedia::stop()
{
    
   EngineBase::stop();
}


/* ---------------------------------------------------------------------------*/
/* Media management method                                                    */
/* ---------------------------------------------------------------------------*/
void EngineQtMultimedia::setMediaItem(MEDIA::TrackPtr track)
{
    Debug::debug() << "[EngineQtMultimedia] -> setMediaItem";

    if(m_currentMediaItem) 
    {
      MEDIA::registerTrackPlaying(m_currentMediaItem, false);
    }

    m_currentMediaItem = MEDIA::TrackPtr(track);
    //Debug::debug() << "[EnginePhonon] -> setMediaItem mi url:" << mi->data.url;

    if(m_nextMediaItem) 
    {
      m_nextMediaItem    = MEDIA::TrackPtr(0);
    }

    /*  BEGIN crash on broken path with vlc backend*/
    if( m_currentMediaItem->isBroken )
    {
      Debug::warning() << "[EnginePhonon] Track path seems to be broken:" << m_currentMediaItem->url;
      //stop(); no stop FOR VLC BACKEND 
      return;
    }
    /* END */

    
    /* get replay gain info */
    if ( (m_currentMediaItem->type() == TYPE_TRACK) && 
         (SETTINGS()->_replaygain != SETTING::ReplayGainOff ) )
    {
        MEDIA::ReplayGainFromDataBase(m_currentMediaItem);    
    }
     
    //const QString path = MEDIA::Track::path(track->url);
    if( MEDIA::isLocal(track->url) )
      m_player->setMedia( QMediaContent(
          QUrl::fromLocalFile(QFileInfo(track->url).canonicalFilePath()) 
                          )
      );
    else
      m_player->setMedia( QUrl(track->url) );
    
    
    //m_player->play();
}

void EngineQtMultimedia::setNextMediaItem(MEDIA::TrackPtr track)
{
}


/* ---------------------------------------------------------------------------*/
/* Audio                                                                      */
/* ---------------------------------------------------------------------------*/
int EngineQtMultimedia::volume() const
{

    return 0;
}

void EngineQtMultimedia::setVolume(const int& percent)
{
}

bool EngineQtMultimedia::isMuted() const
{
}


void EngineQtMultimedia::setMuted( bool mute )
{
}

void EngineQtMultimedia::volumeMute( ) 
{
};

void EngineQtMultimedia::volumeInc( ) 
{
};

void EngineQtMultimedia::volumeDec( )
{
};


/* ---------------------------------------------------------------------------*/
/* engine state                                                               */
/* ---------------------------------------------------------------------------*/
void EngineQtMultimedia::slot_on_player_state_change(QMediaPlayer::State state)
{
    Debug::warning() << "[EngineQtMultimedia] -> slot_on_player_state_change : " << state ;

}

void EngineQtMultimedia::slot_on_media_status_changed(QMediaPlayer::MediaStatus status)
{
    Debug::warning() << "[EngineQtMultimedia] -> slot_on_media_status_changed : " << status ;
    
}

/* ---------------------------------------------------------------------------*/
/* EngineQtMultimedia::slot_on_duration_change                                */
/* ---------------------------------------------------------------------------*/
void  EngineQtMultimedia::slot_on_duration_change(qint64 total_time_ms)
{
Q_UNUSED(total_time_ms)  
}

/* ---------------------------------------------------------------------------*/
/* EngineQtMultimedia::update_total_time                                      */
/* ---------------------------------------------------------------------------*/
void EngineQtMultimedia::update_total_time()
{
}



/* ---------------------------------------------------------------------------*/
/* EngineQtMultimedia::slot_on_media_change                                   */
/* ---------------------------------------------------------------------------*/
void EngineQtMultimedia::slot_on_media_change()
{
    Debug::debug() << "[EngineQtMultimedia] -> slot_on_media_change";
}

/* ---------------------------------------------------------------------------*/
/* EngineQtMultimedia::slot_on_metadata_change                                */
/* ---------------------------------------------------------------------------*/
void EngineQtMultimedia::slot_on_metadata_change()
{
    //Debug::debug() << "[EngineQtMultimedia] -> slot_on_metadata_change";
}

/* ---------------------------------------------------------------------------*/
/* EngineQtMultimedia::slot_on_media_about_to_finish                          */
/* ---------------------------------------------------------------------------*/
void EngineQtMultimedia::slot_on_media_about_to_finish()
{
    Debug::debug() << "[EngineQtMultimedia] -> slot_on_media_about_to_finish";

}

/* ---------------------------------------------------------------------------*/
/* EngineQtMultimedia::slot_on_media_finished                                 */
/* ---------------------------------------------------------------------------*/
void  EngineQtMultimedia::slot_on_media_finished()
{
    //Debug::debug() << "[EngineQtMultimedia] slot_on_media_finished";
}

/* ---------------------------------------------------------------------------*/
/* Time management                                                            */
/* ---------------------------------------------------------------------------*/
void EngineQtMultimedia::seek( qint64 milliseconds )
{
}

void EngineQtMultimedia::slot_on_time_change(qint64 ms)
{
 
}

/* ---------------------------------------------------------------------------*/
/* Equalizer management                                                       */
/* ---------------------------------------------------------------------------*/
bool EngineQtMultimedia::isEqualizerAvailable() 
{
     return false;
}

void EngineQtMultimedia::addEqualizer()
{

}

void EngineQtMultimedia::removeEqualizer()
{
}

void EngineQtMultimedia::applyEqualizer(QList<int> gains)
{

}

void EngineQtMultimedia::loadEqualizerSettings()
{

}


#endif // ENABLE_QTMULTIMEDIA

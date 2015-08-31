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
#ifdef ENABLE_PHONON

// local
#include "engine_phonon.h"
#include "utilities.h"
#include "settings.h"
#include "debug.h"


// Qt
#include <QFileInfo>
#include <QList>
#include <QtPlugin>

// phonon
#include <phonon/backendcapabilities.h>
#include <phonon/effect.h>
#include <phonon/effectparameter.h>

#include <cmath>

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(enginephonon, EnginePhonon) 
#endif
/*
********************************************************************************
*                                                                              *
*    Class EnginePhonon                                                        *
*                                                                              *
********************************************************************************
*/
EnginePhonon::EnginePhonon() : EngineBase("phonon")
{
    m_mediaObject = new Phonon::MediaObject(this);
    m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);

    /* ----- by default tick every 1 second ----- */
    m_mediaObject->setTickInterval(100);
    Debug::debug() << "[EnginePhonon] -> tick Interval (actual): " << m_mediaObject->tickInterval();
    

    /* ----- get the next track when there is 2 seconds left on the current one ----- */
    /*       in case of playing track from track view */
    m_mediaObject->setPrefinishMark( 2000 );
    m_mediaObject->setTransitionTime(100);  /* GAPLESS/CROSSFADE */

    connect(m_mediaObject,SIGNAL(finished()),this,SLOT(slot_on_media_finished()));
    connect(m_mediaObject,SIGNAL(aboutToFinish()),this,SLOT(slot_on_media_about_to_finish()));

    connect(m_mediaObject,SIGNAL(stateChanged(Phonon::State,Phonon::State)),this,SLOT(slot_on_phonon_state_changed(Phonon::State,Phonon::State)),Qt::DirectConnection);

    connect(m_mediaObject,SIGNAL(tick(qint64)),this,SLOT(slot_on_time_change(qint64)));
    connect(m_mediaObject,SIGNAL(totalTimeChanged(qint64)),this,SLOT(slot_on_duration_change(qint64)));

    connect(m_mediaObject,SIGNAL(currentSourceChanged( const Phonon::MediaSource & )),this,SLOT(slot_on_media_change()));

    connect(m_mediaObject, SIGNAL(metaDataChanged()), this,SLOT(slot_on_metadata_change()));

    /* needed for Mplayer2 backend */
    connect( m_audioOutput, SIGNAL( volumeChanged( qreal ) ),this, SIGNAL( volumeChanged() ) );
    connect( m_audioOutput, SIGNAL( mutedChanged( bool ) ), this,  SIGNAL( muteStateChanged() ) );
    
    m_phononPath  = Phonon::createPath(m_mediaObject,m_audioOutput);


    /* ----- only create pre-amp if we have replaygain on, VolumeFaderEffect can cause phonon issues */
    m_preamp = 0;
    if( SETTINGS()->_replaygain != SETTING::ReplayGainOff )
    {
      m_preamp = new Phonon::VolumeFaderEffect( this );
      m_phononPath.insertEffect( m_preamp );
    }

    /* ----- add an equalizer effect if available */
    m_equalizer   = 0;
    QList<Phonon::EffectDescription> mEffectDescriptions = Phonon::BackendCapabilities::availableAudioEffects();
    foreach ( const Phonon::EffectDescription &mDescr, mEffectDescriptions )
    {
      if ( mDescr.name() == QLatin1String( "KEqualizer" ) )
      {
          m_equalizer = new Phonon::Effect( mDescr, this );

          if( SETTINGS()->_enableEq ) {
            addEqualizer();
            loadEqualizerSettings();
          }
      }
    }

    /* ----- initial volume setup ----- */
    setVolume( SETTINGS()->_volumeLevel );
        
    m_current_state    = ENGINE::STOPPED;
    m_old_state        = ENGINE::STOPPED;
}

EnginePhonon::~EnginePhonon()
{
    m_mediaObject->stop();
    delete m_mediaObject;
    delete m_audioOutput;
}


/* ---------------------------------------------------------------------------*/
/* Playing method                                                             */
/* ---------------------------------------------------------------------------*/
void EnginePhonon::play()
{
    m_mediaObject->play();
}

void EnginePhonon::pause()
{
    m_mediaObject->pause();
}

void EnginePhonon::stop()
{
   m_mediaObject->blockSignals(true);
   m_mediaObject->stop();
   m_mediaObject->clearQueue();
   m_mediaObject->blockSignals(false);

   EngineBase::stop();
}


/* ---------------------------------------------------------------------------*/
/* Media management method                                                    */
/* ---------------------------------------------------------------------------*/
void EnginePhonon::setMediaItem(MEDIA::TrackPtr track)
{
    Debug::debug() << "[EnginePhonon] -> setMediaItem";

    m_mediaObject->blockSignals(true);
    m_mediaObject->stop();
    m_mediaObject->clearQueue();
    m_mediaObject->blockSignals(false);

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
      m_mediaObject->blockSignals(false);
      //stop(); no stop FOR VLC BACKEND 
      return;
    }
    /* END */

    //const QString path = MEDIA::Track::path(track->url);
    //Debug::debug() << "[EnginePhonon] -> set url " << path;

    if( MEDIA::isLocal(track->url) )
      m_mediaObject->setCurrentSource( QUrl::fromLocalFile(QFileInfo(track->url).canonicalFilePath()) );
    else
      m_mediaObject->setCurrentSource( QUrl(track->url) );
    
    
    m_mediaObject->play();
}

void EnginePhonon::setNextMediaItem(MEDIA::TrackPtr track)
{
    //Debug::debug() << "[EnginePhonon] ->setNextMediaItem";
    m_nextMediaItem = MEDIA::TrackPtr(track);

    if( MEDIA::isLocal(track->url) )
      m_mediaObject->enqueue( QUrl::fromLocalFile(QFileInfo(track->url).canonicalFilePath()) );
    else
      m_mediaObject->enqueue( QUrl(track->url) );

    /* // DEBUG check queue
    foreach (Phonon::MediaSource source, m_mediaObject->queue() )
      Debug::debug() << "[EnginePhonon] -> #queue -> url :" << source.url() ;*/
}


/* ---------------------------------------------------------------------------*/
/* Audio                                                                      */
/* ---------------------------------------------------------------------------*/
int EnginePhonon::volume() const
{
    /* return volume in percent */
    int volume_in_percent = m_audioOutput->volume()*100;
    volume_in_percent = qBound<qreal>( 0, volume_in_percent, 100 );
    
    return volume_in_percent;
}

void EnginePhonon::setVolume(const int& percent)
{
    Debug::debug() << "[EnginePhonon] -> set percent : " << percent;
    int vp = qBound<qreal>( 0, percent, 100 );

    const qreal volume =  vp / 100.0;

    if ( m_audioOutput->volume() != volume )
    {
      m_audioOutput->setVolume( volume );
      emit volumeChanged();
    }
}

bool EnginePhonon:: isMuted() const
{
    return m_audioOutput->isMuted();
}


void EnginePhonon::setMuted( bool mute )
{
    bool ismuted = m_audioOutput->isMuted();
    
    if(mute != ismuted) {
      m_audioOutput->setMuted( mute );
      emit muteStateChanged();
    }
}

void EnginePhonon::volumeMute( ) 
{
    //Debug::debug() << "[EnginePhonon] -> volumeMute";
    setMuted( !isMuted() );
};

void EnginePhonon::volumeInc( ) 
{
    int percent = volume() < 100 ? volume() + 1 : 100;
    setVolume(percent);
};

void EnginePhonon::volumeDec( )
{
    int percent = volume() > 0 ? volume() -1 : 0;
    setVolume(percent);
};


/* ---------------------------------------------------------------------------*/
/* Phonon state                                                               */
/* ---------------------------------------------------------------------------*/
void EnginePhonon::slot_on_phonon_state_changed(Phonon::State newState, Phonon::State oldState)
{
    //Debug::warning() << "[EnginePhonon] -> slot_on_phonon_state_changed : " << newState ;
    if(newState == oldState)
      return;

    switch (newState) {
      case Phonon::LoadingState:  m_current_state = ENGINE::STOPPED; break;

      case Phonon::StoppedState:  m_current_state = ENGINE::STOPPED; break;

      case Phonon::PlayingState:  m_current_state = ENGINE::PLAYING; break;

      case Phonon::BufferingState:  break;

      case Phonon::PausedState:   m_current_state = ENGINE::PAUSED;  break;

      case Phonon::ErrorState:    m_current_state = ENGINE::ERROR;   break;

      default:break;
    }

    if(m_old_state != m_current_state)
    {
      if(m_current_state != ENGINE::STOPPED)
      {
        //Debug::debug() << "[EnginePhonon] -> engineStateChanged :" << stateToString(m_current_state);
        emit engineStateChanged();
      }
      else if(!m_currentMediaItem)
      {
        //Debug::debug() << "[EnginePhonon] -> engineStateChanged :" << stateToString(m_current_state);
        emit engineStateChanged();
      }

      m_old_state = m_current_state;
    }
}

    

/* ---------------------------------------------------------------------------*/
/* EnginePhonon::slot_on_duration_change                                      */
/* ---------------------------------------------------------------------------*/
void  EnginePhonon::slot_on_duration_change(qint64 total_time_ms)
{
Q_UNUSED(total_time_ms)  
    Debug::debug() << "[EnginePhonon] -> slot_on_duration_change" ;

    /* As Amarok note: don't rely on m_currentTrack here. At least some Phonon backends first emit
       totalTimeChanged(), then metaDataChanged() and only then currentSourceChanged()
      which currently sets correct m_currentTrack. */
    if(m_nextMediaItem) {
       /* totalTimeChanged has been sent before currentSourceChanged */
       return;
    }
    
    update_total_time();
}

/* ---------------------------------------------------------------------------*/
/* EnginePhonon::update_total_time                                            */
/* ---------------------------------------------------------------------------*/
void EnginePhonon::update_total_time()
{
    if(m_currentMediaItem && m_currentMediaItem->type() == TYPE_TRACK)
    {
      if( m_currentMediaItem->duration > 0 )
      {
        m_totalTime  = m_currentMediaItem->duration * 1000;
      }
      else
      {
        m_totalTime  = m_mediaObject->totalTime();
      }
    }
    else
    {
      m_totalTime  = m_mediaObject->totalTime();
    }
    
    emit mediaTotalTimeChanged(m_totalTime);    
}



/* ---------------------------------------------------------------------------*/
/* EnginePhonon::slot_on_media_change                                         */
/* ---------------------------------------------------------------------------*/
static const qreal log10over20 = 0.1151292546497022842; // ln(10) / 20

void EnginePhonon::slot_on_media_change()
{
    Debug::debug() << "[EnginePhonon] -> slot_on_media_change";

    if( m_nextMediaItem )
    {
      Debug::debug() << "[EnginePhonon] -> slot_on_media_change: next media item from queue";
      MEDIA::registerTrackPlaying(m_currentMediaItem, false);
      m_currentMediaItem = MEDIA::TrackPtr(m_nextMediaItem);

      m_nextMediaItem    = MEDIA::TrackPtr(0);
    }

    if(!m_currentMediaItem)
    {
        Debug::error() << "[EnginePhonon] -> no media set";
        //stop();
        return;
    }
    else if ( (m_currentMediaItem->type() == TYPE_TRACK) &&
              (SETTINGS()->_replaygain != SETTING::ReplayGainOff ) )
    {
        if( !m_preamp )  {
            m_preamp = new Phonon::VolumeFaderEffect( this );
            m_phononPath.insertEffect( m_preamp );
        }

        qreal gain, peak;
        if ( SETTINGS()->_replaygain != SETTING::AlbumReplayGain ) {
          gain =  m_currentMediaItem->trackGain;
          peak =  m_currentMediaItem->trackPeak;
        }
        else {
          gain =  m_currentMediaItem->albumGain;
          peak =  m_currentMediaItem->albumPeak;
        }

        if ( gain + peak > 0.0 )
        {
            Debug::debug() << "[EnginePhonon] -> slot_on_media_change Gain of" << gain << "would clip at absolute peak of" << gain + peak;
            gain -= gain + peak;
        }
        Debug::debug() << "[EnginePhonon] -> slot_on_media_change Using gain of" << gain << "with relative peak of" << peak;
        // we calculate the volume change ourselves, because m_preamp->setVolumeDecibel is
        // a little confused about minus signs
        m_preamp->setVolume( exp( gain * log10over20 ) );
        m_preamp->fadeTo( exp( gain * log10over20 ), 0 );
    }
    else if( m_preamp )
    {
        //Debug::debug() << "[EnginePhonon] reset preamp";
        m_preamp->setVolume( 1.0 );
        m_preamp->fadeTo( 1.0, 0 );
    }

 
    /* register track change */
    update_total_time();
    
    MEDIA::registerTrackPlaying(m_currentMediaItem, true);
 
    emit mediaChanged();
}

/* ---------------------------------------------------------------------------*/
/* EnginePhonon::slot_on_metadata_change                                      */
/* ---------------------------------------------------------------------------*/
void EnginePhonon::slot_on_metadata_change()
{
    //Debug::debug() << "[EnginePhonon] -> slot_on_metadata_change";

    if(m_currentMediaItem->type() != TYPE_STREAM) return;

    const QMap<QString, QString> &metaData = m_mediaObject->metaData();

    if( !metaData.value("TITLE").isEmpty() && metaData.value("TITLE") != "Streaming Data" )
      m_currentMediaItem->title = metaData.value("TITLE");

    if( !metaData.value("ALBUM").isEmpty() && metaData.value("ALBUM") != "Streaming Data" )
      m_currentMediaItem->album = metaData.value("ALBUM");
    
    if( !metaData.value("ARTIST").isEmpty() && metaData.value("ARTIST") != "Streaming Data" )
      m_currentMediaItem->artist = metaData.value("ARTIST");
        
    if(metaData.value("TITLE").contains("-")) 
    {
      QStringList list = metaData.value("TITLE").split(" - ");
      m_currentMediaItem->artist = list.first();
      m_currentMediaItem->title = list.last();
    }
    
#ifdef TEST_FLAG    
     Debug::debug() << "[EnginePhonon] -> slotMetaDataChanged title   :" << m_currentMediaItem->title;
     Debug::debug() << "[EnginePhonon] -> slotMetaDataChanged album   :" << m_currentMediaItem->album;
     Debug::debug() << "[EnginePhonon] -> slotMetaDataChanged artist  :" << m_currentMediaItem->artist;
#endif
    emit mediaMetaDataChanged();
}

/* ---------------------------------------------------------------------------*/
/* EnginePhonon::slot_on_media_about_to_finish                                */
/* ---------------------------------------------------------------------------*/
void EnginePhonon::slot_on_media_about_to_finish()
{
    Debug::debug() << "[EnginePhonon] -> slot_on_media_about_to_finish";

    if( m_currentMediaItem && !m_currentMediaItem->isStopAfter )
    {
        /* needed to set next media item */
        if(!m_nextMediaItem)
          emit mediaAboutToFinish(); 
    }
}

/* ---------------------------------------------------------------------------*/
/* EnginePhonon::slot_on_media_finished                                       */
/* ---------------------------------------------------------------------------*/
void  EnginePhonon::slot_on_media_finished()
{
    //Debug::debug() << "[EnginePhonon] slot_on_media_finished";
    emit mediaFinished();

    if(m_nextMediaItem)
    {
      Debug::debug() << "[EnginePhonon] -> slot_on_media_finished next mediaitem present !!";
      setMediaItem(m_nextMediaItem);
    }
    else if( m_mediaObject->queue().isEmpty())
    {
      Debug::debug() << "[EnginePhonon] -> slot_on_media_finished QUEUE IS EMPTY";
      emit engineRequestStop();
    }
}

/* ---------------------------------------------------------------------------*/
/* Time management                                                            */
/* ---------------------------------------------------------------------------*/
void EnginePhonon::seek( qint64 milliseconds )
{
    if( m_mediaObject->isSeekable() )
    {
        Debug::debug() << "[EnginePhonon] -> seek to: " << milliseconds;

        m_mediaObject->seek( milliseconds );
        emit mediaTick( milliseconds );
        m_lastTick = milliseconds;
    }
    else
        Debug::warning() << "[EnginePhonon] -> not seekable media";
}

void EnginePhonon::slot_on_time_change(qint64 ms)
{
    m_lastTick = ms;
    emit mediaTick(ms);
  
}

/* ---------------------------------------------------------------------------*/
/* Equalizer management                                                       */
/* ---------------------------------------------------------------------------*/
bool EnginePhonon::isEqualizerAvailable() 
{
    QList<Phonon::EffectDescription> effects = Phonon::BackendCapabilities::availableAudioEffects();

    foreach (Phonon::EffectDescription effect, effects) {
      if(effect.name()=="KEqualizer")
        return true;
     }
     return false;
}

void EnginePhonon::addEqualizer()
{
    if(!m_equalizer) {
      Debug::debug() << "[EnginePhonon] -> addEqualizer : no equalizer available !";
      return;
    }

    // Insert effect into path if needed
    if( m_phononPath.effects().indexOf( m_equalizer ) == -1 )
    {
      if( !m_phononPath.effects().isEmpty() )
      {
        m_phononPath.insertEffect( m_equalizer, m_phononPath.effects().first() );
      }
      else
      {
        m_phononPath.insertEffect( m_equalizer );
      }
    }
}


void EnginePhonon::removeEqualizer()
{
    //Debug::debug() << "[EnginePhonon] -> removeEqualizer !";
    if( m_phononPath.effects().indexOf( m_equalizer ) != -1 ) {
      m_phononPath.removeEffect( m_equalizer );
    }
}

void EnginePhonon::applyEqualizer(QList<int> gains)
{
    if(!m_equalizer) {
      Debug::warning() << "[EnginePhonon] -> no equalizer initialized !!";
      return;
    }

    QList<Phonon::EffectParameter> effect_param_list = m_equalizer->parameters();

    QListIterator<int> it_effect_param( gains );
    
    /*Scaled value to set from universal -100 - 100 range to plugin scale */
    double scaled_value; 
    foreach( const Phonon::EffectParameter &effect_param, effect_param_list )
    {
       scaled_value = it_effect_param.hasNext() ? it_effect_param.next() : 0;
       scaled_value *= ( qAbs(effect_param.maximumValue().toDouble() ) +  qAbs( effect_param.minimumValue().toDouble() ) );
       scaled_value /= 200.0;
       //Debug::debug() << "[EnginePhonon] set equalizer value :" << scaled_value;
       m_equalizer->setParameterValue( effect_param, scaled_value );
    }
}

void EnginePhonon::loadEqualizerSettings()
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


#endif // ENABLE_PHONON


#ifdef ENABLE_QTMULTIMEDIA

#include "engine_qtmultimedia.h"
#include "utilities.h"
#include "settings.h"
#include "debug.h"

#include <QFileInfo>
#include <QtPlugin>


EngineQtMultimedia::EngineQtMultimedia() : EngineBase("qtmultimedia") {
  m_type = ENGINE::QTMULTIMEDIA;

  m_player = new QMediaPlayer(this);
  m_output = new QAudioOutput;
  m_player->setAudioOutput(m_output);
  m_output->setVolume(100);

  connect(m_player, SIGNAL(playbackStateChanged(QMediaPlayer::PlaybackState)),this,SLOT(slot_on_player_state_changed(QMediaPlayer::PlaybackState)),Qt::DirectConnection);
  connect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(slot_on_media_status_changed(QMediaPlayer::MediaStatus)));
  connect(m_player, SIGNAL(metaDataChanged()), this, SLOT(slot_on_metadata_change()));
  connect(m_player, SIGNAL(durationChanged(qint64)),this,SLOT(slot_on_duration_change(qint64)));
  //connect(m_player, SIGNAL(sourceChanged(const QUrl)),this,SLOT(slot_on_media_change()));
  connect(m_player, SIGNAL(positionChanged(qint64)),this,SLOT(slot_on_time_change(qint64)));

  connect(m_output, SIGNAL(volumeChanged(float)),this, SIGNAL(volumeChanged()));
  connect(m_output, SIGNAL(mutedChanged(bool)), this, SIGNAL(muteStateChanged()));

  /* ----- initial volume setup ----- */
  int restoredVolume = SETTINGS()->_volumeLevel > this->maxVolume() ? this->maxVolume() : SETTINGS()->_volumeLevel;
  setVolume( restoredVolume );

  m_current_state    = ENGINE::STOPPED;
  m_old_state        = ENGINE::STOPPED;

  m_version = QString(QT_VERSION_STR);
}

EngineQtMultimedia::~EngineQtMultimedia() {
    m_player->stop();
    delete m_player;
}

void EngineQtMultimedia::play() {
    m_player->play();
}

void EngineQtMultimedia::pause() {
    m_player->pause();
}

void EngineQtMultimedia::stop() {
    m_player->stop();
    EngineBase::stop();
}

void EngineQtMultimedia::setMediaItem(MEDIA::TrackPtr track) {
    Debug::debug() << "[EngineQtMultimedia] -> setMediaItem";

    if(m_currentMediaItem)
    {
      MEDIA::registerTrackPlaying(m_currentMediaItem, false);
    }

    m_currentMediaItem = MEDIA::TrackPtr(track);
    //Debug::debug() << "[EngineQtMultimedia] -> setMediaItem mi url:" << mi->data.url;

    if(m_nextMediaItem)
    {
      m_nextMediaItem    = MEDIA::TrackPtr(0);
    }

    /*  BEGIN crash on broken path with vlc backend*/
    if( m_currentMediaItem->isBroken )
    {
      Debug::warning() << "[EngineQtMultimedia] Track path seems to be broken:" << m_currentMediaItem->url;
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
      m_player->setSource(QUrl::fromLocalFile(QFileInfo(track->url).canonicalFilePath()));
    else
      m_player->setSource(QUrl(track->url));

    m_player->play();
}

void EngineQtMultimedia::setNextMediaItem(MEDIA::TrackPtr track) {
    Debug::debug() << "[EngineQtMultimedia] ->setNextMediaItem";
    m_nextMediaItem = MEDIA::TrackPtr(track);

    if( MEDIA::isLocal(track->url) )
      m_player->setSource( QUrl::fromLocalFile(QFileInfo(track->url).canonicalFilePath()) );
    else
      m_player->setSource( QUrl(track->url) );
}

int EngineQtMultimedia::volume() const {
    /* return volume in percent */
    int volume_in_percent = m_output->volume()*100;
    volume_in_percent = qBound<qreal>( 0, volume_in_percent, 100 );

    return volume_in_percent;
}

void EngineQtMultimedia::setVolume(const int& percent) {
      Debug::debug() << "[EngineQtMultimedia] -> set percent : " << percent;
    int vp = qBound<qreal>( 0, percent, 100 );

    const qreal volume =  vp / 100.0;

    if ( m_output->volume() != volume )
    {
      m_output->setVolume( volume );
      emit volumeChanged();
    }
}

bool EngineQtMultimedia::isMuted() const {
  return m_output->isMuted();
}

void EngineQtMultimedia::setMuted(bool mute) {
      bool ismuted = m_output->isMuted();

    if(mute != ismuted) {
      m_output->setMuted( mute );
      emit muteStateChanged();
    }
}

void EngineQtMultimedia::volumeMute() {
  setMuted( !isMuted() );
};

void EngineQtMultimedia::volumeInc() {
      int percent = volume() < 100 ? volume() + 1 : 100;
    setVolume(percent);
};

void EngineQtMultimedia::volumeDec() {
    int percent = volume() > 0 ? volume() -1 : 0;
    setVolume(percent);
};

void EngineQtMultimedia::slot_on_player_state_changed(QMediaPlayer::PlaybackState state) {
    Debug::warning() << "[EngineQtMultimedia] -> slot_on_player_state_change : " << state ;

    switch (state) {
      case QMediaPlayer::StoppedState:  m_current_state = ENGINE::STOPPED; break;

      case QMediaPlayer::PlayingState:  m_current_state = ENGINE::PLAYING; break;

      case QMediaPlayer::PausedState:   m_current_state = ENGINE::PAUSED;  break;

      default:break;
    }

    if(m_old_state != m_current_state)
    {
      if(m_current_state != ENGINE::STOPPED)
      {
        //Debug::debug() << "[EngineQtMultimedia] -> engineStateChanged :" << stateToString(m_current_state);
        emit engineStateChanged();
      }
      else if(!m_currentMediaItem)
      {
        //Debug::debug() << "[EngineQtMultimedia] -> engineStateChanged :" << stateToString(m_current_state);
        emit engineStateChanged();
      }

      m_old_state = m_current_state;
    }

}

void EngineQtMultimedia::slot_on_media_status_changed(QMediaPlayer::MediaStatus status) {
    Debug::warning() << "[EngineQtMultimedia] -> slot_on_media_status_changed : " << status ;

    if (status == QMediaPlayer::BufferedMedia) {
      slot_on_media_change();
    }
    else if (status == QMediaPlayer::EndOfMedia) {
      slot_on_media_about_to_finish();
      slot_on_media_finished();
    }
}

void  EngineQtMultimedia::slot_on_duration_change(qint64 total_time_ms) {
Q_UNUSED(total_time_ms)
    //Debug::debug() << "[EngineQtMultimedia] -> slot_on_duration_change" ;

    /* As Amarok note: don't rely on m_currentTrack here. At least some Phonon backends first emit
       totalTimeChanged(), then metaDataChanged() and only then currentSourceChanged()
      which currently sets correct m_currentTrack. */
    if(m_nextMediaItem) {
       /* totalTimeChanged has been sent before currentSourceChanged */
       return;
    }

    update_total_time();
}

void EngineQtMultimedia::update_total_time() {
  if(m_currentMediaItem && m_currentMediaItem->type() == TYPE_TRACK)
    {
      if( m_currentMediaItem->duration > 0 )
      {
        m_totalTime  = m_currentMediaItem->duration * 1000;
      }
      else
      {
        m_totalTime  = m_player->duration();
      }
    }
    else
    {
      m_totalTime  = m_player->duration();
    }
    Debug::debug() << "[EngineQtMultimedia] -> total_time: " << m_totalTime;
    emit mediaTotalTimeChanged(m_totalTime);
}

void EngineQtMultimedia::slot_on_media_change() {
    Debug::debug() << "[EngineQtMultimedia] -> slot_on_media_change";

    if( m_nextMediaItem )
    {
      Debug::debug() << "[EngineQtMultimedia] -> slot_on_media_change: next media item from queue";
      MEDIA::registerTrackPlaying(m_currentMediaItem, false);
      m_currentMediaItem = MEDIA::TrackPtr(m_nextMediaItem);

      m_nextMediaItem    = MEDIA::TrackPtr(0);
    }

    if(!m_currentMediaItem)
    {
        Debug::error() << "[EngineQtMultimedia] -> no media set";
        // note phonon gestreamer need a sto pin this case
        // don't remember if it's a probleme for phonon vlc
        stop();
        return;
    }

    /* register track change */
    update_total_time();

    MEDIA::registerTrackPlaying(m_currentMediaItem, true);

    emit mediaChanged();
}

void EngineQtMultimedia::slot_on_metadata_change() {
  Debug::debug() << "[EngineQtMultimedia] -> slot_on_metadata_change";
    if(!m_currentMediaItem || m_currentMediaItem->type() != TYPE_STREAM) return;

    const QMediaMetaData &metaData = m_player->metaData();

    if(!metaData.stringValue(QMediaMetaData::Title).isEmpty() && metaData.stringValue(QMediaMetaData::Title) != "Streaming Data" )
      m_currentMediaItem->title = metaData.stringValue(QMediaMetaData::Title);

    if(!metaData.stringValue(QMediaMetaData::AlbumTitle).isEmpty() && metaData.stringValue(QMediaMetaData::AlbumTitle) != "Streaming Data" )
      m_currentMediaItem->album = metaData.stringValue(QMediaMetaData::AlbumTitle);

    if(!metaData.stringValue(QMediaMetaData::AlbumArtist).isEmpty() && metaData.stringValue(QMediaMetaData::AlbumArtist) != "Streaming Data" )
      m_currentMediaItem->artist = metaData.stringValue(QMediaMetaData::AlbumArtist);

    if(metaData.stringValue(QMediaMetaData::Title).contains("-"))
    {
      QStringList list = metaData.stringValue(QMediaMetaData::Title	).split(" - ");
      m_currentMediaItem->artist = list.first();
      m_currentMediaItem->title = list.last();
    }

    // No support in phonon for bitrate/samplerate/format meta data

#ifdef TEST_FLAG
    foreach(QString key, metaData.keys())
      Debug::debug() << "[EngineQtMultimedia] -> on_metadata_change " << key << ":" << metaData[key];
#endif

    emit mediaMetaDataChanged();
}

void EngineQtMultimedia::slot_on_media_about_to_finish() {
    Debug::debug() << "[EngineQtMultimedia] -> slot_on_media_about_to_finish";

    if( m_currentMediaItem && !m_currentMediaItem->isStopAfter )
    {
        /* needed to set next media item */
        if(!m_nextMediaItem)
          emit mediaAboutToFinish();
    }
}

void  EngineQtMultimedia::slot_on_media_finished() {
    Debug::debug() << "[EngineQtMultimedia] -> slot_on_media_finished";
    emit mediaFinished();

    if(m_nextMediaItem)
    {
      Debug::debug() << "[EngineQtMultimedia] -> slot_on_media_finished next mediaitem present !!";
      setMediaItem(m_nextMediaItem);
    }
    else {
      Debug::debug() << "[EngineQtMultimedia] -> slot_on_media_finished QUEUE IS EMPTY";
      emit engineRequestStop();
    }
}

void EngineQtMultimedia::seek(qint64 milliseconds) {
    if( m_player->isSeekable() )
    {
        Debug::debug() << "[EngineQtMultimedia] -> seek to: " << milliseconds;

        m_player->setPosition( milliseconds );
        emit mediaTick( milliseconds );
        m_lastTick = milliseconds;
    }
    else
        Debug::warning() << "[EngineQtMultimedia] -> not seekable media";
}

void EngineQtMultimedia::slot_on_time_change(qint64 ms) {
    m_lastTick = ms;
    emit mediaTick(ms);
}

/*
bool EngineQtMultimedia::isEqualizerAvailable() {
     return false;
}

void EngineQtMultimedia::addEqualizer() {

}

void EngineQtMultimedia::removeEqualizer() {
}

void EngineQtMultimedia::applyEqualizer(QList<int> gains) {

}

void EngineQtMultimedia::loadEqualizerSettings() {

}
*/

#endif // ENABLE_QTMULTIMEDIA

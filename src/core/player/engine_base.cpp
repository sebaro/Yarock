
#include "engine_base.h"
#include "debug.h"

EngineBase::EngineBase() {
    m_name = "no engine";
    m_type = ENGINE::NO_ENGINE;
}

EngineBase::EngineBase(const QString& name) {
    //Debug::debug() << "[PLAYER] create player : " << m_name;
    m_name = name;

    m_isEngineOK       = true;
    m_current_state    = ENGINE::STOPPED;
    m_old_state        = ENGINE::STOPPED;

    m_currentMediaItem = MEDIA::TrackPtr(0);
    m_nextMediaItem    = MEDIA::TrackPtr(0);
}

void EngineBase::stop() {
   if( m_currentMediaItem )
   {
       MEDIA::registerTrackPlaying(m_currentMediaItem, false);

       m_currentMediaItem = MEDIA::TrackPtr(0);
   }

   if( m_nextMediaItem )
   {
       m_nextMediaItem    = MEDIA::TrackPtr(0);
   }

   m_current_state =  ENGINE::STOPPED;
   m_old_state     =  ENGINE::STOPPED;
   emit engineStateChanged();
}

QString EngineBase::stateToString(ENGINE::E_ENGINE_STATE state) {
    switch(state)
    {
      case ENGINE::PLAYING: return QString("PLAYING");break;
      case ENGINE::PAUSED:  return QString("PAUSED"); break;
      case ENGINE::STOPPED: return QString("STOPPED");break;
      case ENGINE::ERROR:   return QString("ERROR");  break;
      default : return QString("unknown state");
    }
}

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

#include "engine_base.h"
#include "debug.h"

EngineBase::EngineBase()
{
    m_name = "no engine";
    m_type = ENGINE::NO_ENGINE;
}

EngineBase::EngineBase(const QString& name)
{
    //Debug::debug() << "[PLAYER] create player : " << m_name;
    m_name = name;
    
    m_isEngineOK       = true;
    m_current_state    = ENGINE::STOPPED;
    m_old_state        = ENGINE::STOPPED;
 
    m_currentMediaItem = MEDIA::TrackPtr(0);
    m_nextMediaItem    = MEDIA::TrackPtr(0);
}

/* ---------------------------------------------------------------------------*/
/* EngineBase::stop                                                           */
/* ---------------------------------------------------------------------------*/ 
void EngineBase::stop() 
{
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

/* ---------------------------------------------------------------------------*/
/* EngineBase::stateToString                                                  */
/* ---------------------------------------------------------------------------*/
QString EngineBase::stateToString(ENGINE::E_ENGINE_STATE state) 
{
    switch(state)
    {
      case ENGINE::PLAYING: return QString("PLAYING");break;
      case ENGINE::PAUSED:  return QString("PAUSED"); break;
      case ENGINE::STOPPED: return QString("STOPPED");break;
      case ENGINE::ERROR:   return QString("ERROR");  break;
      default : return QString("unknown state");
    }
}

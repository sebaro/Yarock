/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2014 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

#include "engine.h"
#include "settings.h"

// vlc engine
#ifdef ENABLE_VLC
#include "engine_vlc.h"
#endif

#ifdef ENABLE_PHONON
#include "engine_phonon.h"
#endif

EngineBase* Engine::CORE_INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class Engine                                                              *
*                                                                              *
********************************************************************************
*/ 
Engine::Engine()
{

#ifdef ENABLE_VLC
    if( SETTINGS()->_engine == ENGINE::VLC ) {
      CORE_INSTANCE     = new EngineVlc();
    }
#endif

#ifdef ENABLE_PHONON
    if( SETTINGS()->_engine == ENGINE::PHONON ) {
      CORE_INSTANCE    = new EnginePhonon();
    }      
#endif
    
    if(CORE_INSTANCE==0) {
      CORE_INSTANCE    = new EngineBase("null");
    }
}
 

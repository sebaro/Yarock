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

#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "engine_base.h"
#include <QString>

namespace ENGINE {
  enum E_ENGINE_TYPE {NO_ENGINE = 0, PHONON, VLC, MPV};
}


/*
********************************************************************************
*                                                                              *
*    Class Engine                                                              *
*                                                                              *
********************************************************************************
*/ 
class Engine
{
    static EngineBase         *CORE_INSTANCE;

public:
    Engine();
    static EngineBase* instance() { return CORE_INSTANCE; }
    
    QString error() { return m_error;}

private:
    QString m_error;
};

#endif // _ENGINE_H_

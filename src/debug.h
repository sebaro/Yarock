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

#ifndef _YAROCK_DEBUG_H_
#define _YAROCK_DEBUG_H_

#include <QtCore/QDebug>

namespace Debug
{

    // from kdebug.h
    enum DebugLevel {
        YDEBUG_INFO  = 0,
        YDEBUG_WARN  = 1,
        YDEBUG_ERROR = 2,
        YDEBUG_FATAL = 3
    };


    bool debugEnabled();
    void setDebugEnabled( bool enable );

    QDebug debugStream( DebugLevel level = YDEBUG_INFO );

    static inline QDebug debug()   { return debugStream( YDEBUG_INFO ); }
    static inline QDebug warning() { return debugStream( YDEBUG_WARN ); }
    static inline QDebug error()   { return debugStream( YDEBUG_ERROR ); }
    static inline QDebug fatal()   { return debugStream( YDEBUG_FATAL ); }


} // namespace Debug

#endif //_YAROCK_DEBUG_H_

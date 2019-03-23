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


#include "debug.h"
#include <QIODevice>


using namespace Debug;

class NoDebugDevice: public QIODevice
{
public:
    NoDebugDevice() { open(WriteOnly); }
    bool isSequential() const { return true; }
    qint64 readData(char *, qint64) { return 0; /* eof */ }
    qint64 readLineData(char *, qint64) { return 0; /* eof */ }
    qint64 writeData(const char *, qint64 len) { return len; }
};

static NoDebugDevice  devnull;
static bool static_debugEnabled = false;

static QString toString( DebugLevel level )
{
    switch( level )
    {
        case YDEBUG_INFO:
            return QString();
        case YDEBUG_WARN:
            return "[WARNING]";
        case YDEBUG_ERROR:
            return "[ERROR__]";
        case YDEBUG_FATAL:
            return "[FATAL__]";
        default:
            return QString();
    }
}


bool Debug::debugEnabled()
{
    return static_debugEnabled;
}

void Debug::setDebugEnabled( bool enable )
{
    static_debugEnabled = enable;
}


QDebug Debug::debugStream( DebugLevel level )
{
    if( !debugEnabled() )
        return QDebug(&devnull);

    QString text = toString(level);

#if QT_VERSION < QT_VERSION_CHECK(5,5,0)
    return QDebug( QtDebugMsg ) << qPrintable( text );
#else
    return QDebug( QtInfoMsg ) << qPrintable( text );
#endif    
}



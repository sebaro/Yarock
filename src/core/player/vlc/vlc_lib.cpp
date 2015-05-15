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
#ifdef ENABLE_VLC


#include <vlc/libvlc.h>
#include <vlc/libvlc_version.h>

#include "vlc_lib.h"
#include "debug.h"

#include <QByteArray>
#include <QVarLengthArray>


VlcLib* VlcLib::INSTANCE = 0;


VlcLib::VlcLib() : m_vlcInstance(0)
{
    INSTANCE         = this;
}

VlcLib::~VlcLib()
{
    if (m_vlcInstance)
      libvlc_release(m_vlcInstance);
}

libvlc_instance_t* VlcLib::core()
{
    return m_vlcInstance;
}

bool VlcLib::init()
{
    //Debug::warning() << "[PLAYER] -> VlcLib::init";
  
    //! init args for vlc libs
    QList<QByteArray> args;

    /* warning from vlc doc : There is absolutely no warranty or 
    promise of forward, backward and cross-platform compatibility with regards 
    to libvlc_new() arguments. We recommend that you do not use them,
     other than when debugging. */
    args << "--no-media-library"
         << "--no-one-instance"
         << "--no-plugins-cache"
         << "--no-stats"
         << "--no-osd"
         << "--no-loop"
         << "--no-xlib"
         << "--no-video-title-show"
         << "--drop-late-frames"
         << "--no-video";

    // Convert arguments to required format
    QVarLengthArray<const char *, 64> vlcArgs(args.size());
    for (int i = 0; i < args.size(); ++i) {
        vlcArgs[i] = args.at(i).constData();
    }
    

    // Create new libvlc instance
    m_vlcInstance = libvlc_new(vlcArgs.size(), vlcArgs.constData());


    // Check if instance is running
    if(m_vlcInstance) 
    {
        Debug::debug() << "[EngineVlc] vlc initialised";
        Debug::debug() << "[EngineVlc] using libvlc version:" << QString(libvlc_get_version());
    } 
    else 
    {
        Debug::error() << "vlc initialization error :" << libvlc_errmsg();
        return false;
    }
    
    return true;
}

void VlcLib::print_error()
{
    // Outputs libvlc error message if there is any
    if(libvlc_errmsg()) {
      Debug::error() << "[libvlc] " << "Error:" << libvlc_errmsg();
      libvlc_clearerr();
    }
}


bool VlcLib::isError()
{
    if(libvlc_errmsg()) 
      return true;
    
    return false;
}


#endif // ENABLE_VLC

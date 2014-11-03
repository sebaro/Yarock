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
        Debug::debug() << "vlc initialised";
        Debug::debug() << "Using libvlc version:" << QString(libvlc_get_version());
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

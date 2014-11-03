#ifdef ENABLE_VLC




#include "vlc_media.h"
#include "vlc_lib.h"
#include "debug.h"

#include <vlc/vlc.h>


VlcMedia::VlcMedia() : QObject()
{
}

VlcMedia::~VlcMedia()
{
    removeCoreConnections();

    if(m_vlcMedia)
      libvlc_media_release(m_vlcMedia);
}

libvlc_media_t* VlcMedia::core()
{
    return m_vlcMedia;
}


void VlcMedia::init(const QString &location, bool isLocal)
{
    m_currentLocation = location;

    // Create a new libvlc media descriptor from location
    if (isLocal)
      m_vlcMedia = libvlc_media_new_path(VlcLib::instance()->core(), m_currentLocation.toLocal8Bit().data());
    else
      m_vlcMedia = libvlc_media_new_location(VlcLib::instance()->core(), m_currentLocation.toLocal8Bit().data());
            
    m_vlcEvents = libvlc_media_event_manager( m_vlcMedia );
    
    createCoreConnections();
}



void VlcMedia::createCoreConnections()
{
    QList<libvlc_event_e> list;
    list << libvlc_MediaMetaChanged
         << libvlc_MediaDurationChanged;

    foreach(const libvlc_event_e &event, list) {
        libvlc_event_attach(m_vlcEvents, event, libvlc_callback, this);
    }
}

void VlcMedia::removeCoreConnections()
{
    QList<libvlc_event_e> list;
    list << libvlc_MediaMetaChanged
         << libvlc_MediaDurationChanged;

    foreach(const libvlc_event_e &event, list) {
        libvlc_event_detach(m_vlcEvents, event, libvlc_callback, this);
    }
}

QString VlcMedia::currentLocation() const
{
    return m_currentLocation;
}

void VlcMedia::setOption(const QString &option)
{
    libvlc_media_add_option(m_vlcMedia, option.toLocal8Bit().data());

    VlcLib::instance()->print_error();
}

void VlcMedia::setOptions(const QStringList &options)
{
    foreach(const QString &option, options) {
        libvlc_media_add_option(m_vlcMedia, option.toLocal8Bit().data());
    }

    VlcLib::instance()->print_error();
}


void VlcMedia::libvlc_callback(const libvlc_event_t *event,void *data)
{
    VlcMedia *vlc_media = (VlcMedia *)data;
    Q_ASSERT(vlc_media);
    
    switch(event->type)
    {
    case libvlc_MediaDurationChanged:
        QMetaObject::invokeMethod(
                    vlc_media, "durationChanged",
                    Qt::QueuedConnection,
                    Q_ARG(qint64, event->u.media_duration_changed.new_duration));
        break;
    case libvlc_MediaMetaChanged:
        QMetaObject::invokeMethod(
                    vlc_media, "metaDataChanged",
                    Qt::QueuedConnection);
        break;
    default:
        Debug::error() << "Unknown event: " << QString(libvlc_event_type_name(event->type));
        break;
    }
}



QString VlcMedia::meta(libvlc_meta_t meta)
{
    return QString(libvlc_media_get_meta(m_vlcMedia, meta));
}

#endif // ENABLE_VLC

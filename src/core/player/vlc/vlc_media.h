
#ifdef ENABLE_VLC

#ifndef _VLC_MEDIA_H_
#define _VLC_MEDIA_H_

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>

#include <vlc/vlc.h>

struct libvlc_event_t;
struct libvlc_event_manager_t;
struct libvlc_media_t;


class VlcMedia : public QObject
{
Q_OBJECT
public:

    VlcMedia();
    ~VlcMedia();

    libvlc_media_t* core();

    void init(const QString &location,bool isLocal);
    
    QString currentLocation() const;

    void setOption(const QString &option);
    void setOptions(const QStringList &options);

    QString meta(libvlc_meta_t meta);
    
signals:
    void durationChanged(qint64 duration);
    void metaDataChanged();

private:
    static void libvlc_callback(const libvlc_event_t *event,
                                void *data);

    void createCoreConnections();
    void removeCoreConnections();

    libvlc_media_t            *m_vlcMedia;
    libvlc_event_manager_t    *m_vlcEvents;
    QString                    m_currentLocation;
};

#endif // _VLC_MEDIA_H_
#endif // ENABLE_VLC

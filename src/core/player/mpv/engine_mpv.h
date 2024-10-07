
#ifdef ENABLE_MPV

#ifndef _ENGINE_MPV_H_
#define _ENGINE_MPV_H_

#include "engine_base.h"
#include "mpv/client.h"

#include <QtCore/QObject>


class EngineMpv : public EngineBase {
    Q_OBJECT
    Q_INTERFACES(EngineBase)
    Q_PLUGIN_METADATA(IID "EngineMpv")

    public:
        EngineMpv();
        ~EngineMpv();

        /* play/pause/stop */
        void play();
        void pause();
        void stop();

        /* media management */
        void setMediaItem(MEDIA::TrackPtr track);
        void setNextMediaItem(MEDIA::TrackPtr track);

        /* audio */
        int maxVolume() const { return 150;};
        int volume() const;
        void setVolume(const int &);
        bool isMuted() const;
        void setMuted( bool mute );

        /* time */
        void seek( qint64 );

    public slots:
        void volumeMute( );
        void volumeInc( );
        void volumeDec( );

    protected:
        bool event(QEvent*);

    private:
        void applyInternalVolume();
        void applyInternalMute();
        void handle_mpv_property_change(mpv_event *);
        void on_metadata_change();
        void on_media_about_to_finish();
        void on_media_change();
        void update_total_time();
        void on_media_finished();
        void on_media_error();

    private:
        mpv_handle      *m_mpv_core;
        int              m_internal_volume;
        bool             m_is_volume_changed;

        bool             m_internal_is_mute;
        bool             m_is_muted_changed;
};

#endif // _ENGINE_MPV_H_
#endif // ENABLE_MPV

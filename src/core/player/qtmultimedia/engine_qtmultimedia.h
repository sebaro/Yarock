
#ifdef ENABLE_QTMULTIMEDIA

#ifndef _ENGINE_QTMULTIMEDIA_H_
#define _ENGINE_QTMULTIMEDIA_H_

#include "core/player/engine_base.h"
#include "core/mediaitem/mediaitem.h"

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMediaMetaData>


class EngineQtMultimedia : public EngineBase {
    Q_OBJECT
    Q_INTERFACES(EngineBase)
    Q_PLUGIN_METADATA(IID "EngineQtMultimedia")

    public:
			EngineQtMultimedia();
        ~EngineQtMultimedia();

        /* play/pause/stop */
        void play();
        void pause();
        void stop();

        /* media management */
        void setMediaItem(MEDIA::TrackPtr track);
        void setNextMediaItem(MEDIA::TrackPtr track);

        /* audio */
        int volume() const;
        void setVolume(const int &);
        bool isMuted() const;
        void setMuted( bool mute );

        /* time */
        void seek( qint64 );

        /* effect */
        //bool isEqualizerAvailable();
        //void addEqualizer();
        //void removeEqualizer();
        //void applyEqualizer(QList<int>);

    public slots:
        void volumeMute( );
        void volumeInc( );
        void volumeDec( );

    private:
        void loadEqualizerSettings();
        void update_total_time();

    private slots:
        void slot_on_player_state_changed(QMediaPlayer::PlaybackState);
        void slot_on_media_status_changed(QMediaPlayer::MediaStatus);
        void slot_on_media_change();
        void slot_on_duration_change(qint64);
        void slot_on_time_change(qint64);
        void slot_on_media_finished();
        void slot_on_media_about_to_finish();
        void slot_on_metadata_change();

    private:
        QMediaPlayer      *m_player;
        QAudioOutput      *m_output;
};

#endif // _ENGINE_QTMULTIMEDIA_H_
#endif // ENABLE_QTMULTIMEDIA

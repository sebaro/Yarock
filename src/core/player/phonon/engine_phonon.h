
#ifdef ENABLE_PHONON

#ifndef _ENGINE_PHONON_H_
#define _ENGINE_PHONON_H_

#include "core/player/engine_base.h"
#include "core/mediaitem/mediaitem.h"

#include <QObject>
#include <QString>
#include <QTimer>
#include <QPointer>
#include <QPixmap>

#include <phonon/mediasource.h>
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#include <phonon/path.h>
#include <phonon/effect.h>
#include <phonon/volumefadereffect.h>


class EnginePhonon : public EngineBase {
    Q_OBJECT
    Q_INTERFACES(EngineBase)
    Q_PLUGIN_METADATA(IID "EnginePhonon")

    public:
        EnginePhonon();
        ~EnginePhonon();

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
        bool isEqualizerAvailable();
        void addEqualizer();
        void removeEqualizer();
        void applyEqualizer(QList<int>);

    public slots:
        void volumeMute( );
        void volumeInc( );
        void volumeDec( );

    private:
        void loadEqualizerSettings();
        void update_total_time();

    private slots:
        void slot_on_media_change();
        void slot_on_duration_change(qint64);
        void slot_on_time_change(qint64);
        void slot_on_media_finished();
        void slot_on_media_about_to_finish();
        void slot_on_metadata_change();

        void slot_on_phonon_state_changed(Phonon::State, Phonon::State);

    private:
        Phonon::MediaObject     *m_mediaObject;
        Phonon::AudioOutput     *m_audioOutput;
        Phonon::Path             m_phononPath;
        Phonon::Effect          *m_equalizer;
        QPointer<Phonon::VolumeFaderEffect> m_preamp;
};

#endif // _ENGINE_PHONON_H_
#endif // ENABLE_PHONON

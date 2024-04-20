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
#ifdef ENABLE_QTMULTIMEDIA

#ifndef _ENGINE_QTMULTIMEDIA_H_
#define _ENGINE_QTMULTIMEDIA_H_

// Qt
#include <QObject>
#include <QMediaPlayer>

#include "core/player/engine_base.h"
#include "core/mediaitem/mediaitem.h"


/*
********************************************************************************
*                                                                              *
*    Class EngineQtMultimedia                                                  *
*                                                                              *
********************************************************************************
*/
class EngineQtMultimedia : public EngineBase
{
Q_OBJECT
Q_INTERFACES(EngineBase)
#if QT_VERSION >= 0x050000
Q_PLUGIN_METADATA(IID "EngineQtMultimedia")
#endif 

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
    void slot_on_player_state_change(QMediaPlayer::State);
    void slot_on_media_status_changed(QMediaPlayer::MediaStatus);
    void slot_on_media_change();
    void slot_on_duration_change(qint64);
    void slot_on_time_change(qint64);
    void slot_on_media_finished();
    void slot_on_media_about_to_finish();
    void slot_on_metadata_change();

private:
    QMediaPlayer      *m_player;

};

#endif // _ENGINE_QTMULTIMEDIA_H_
#endif // ENABLE_QTMULTIMEDIA

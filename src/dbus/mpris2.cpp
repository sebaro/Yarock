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
#include "mpris2.h"
#include "mpris2Player.h"
#include "mpris2Root.h"

#include "core/player/engine.h"
#include "core/player/engine_base.h"
#include "widgets/playertoolbar/audiocontrols.h"
#include "covercache.h"
#include "global_actions.h"
#include "utilities.h"
#include "debug.h"

#include <QApplication>
#include <QImage>
#include <QtDBus/QtDBus>

/*
********************************************************************************
*                                                                              *
*    Mpris2                                                                    *
*                                                                              *
********************************************************************************
*/
Mpris2::Mpris2(QObject* parent)  : QObject(parent)
{
    // DBus connection
    new Mpris2Root( this );
    new Mpris2Player( this );
    
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject( "/org/mpris/MediaPlayer2", this );
    dbus.registerService( "org.mpris.MediaPlayer2.yarock" );

    // Listen to volume changes
    connect(Engine::instance(), SIGNAL( volumeChanged() ), SLOT( slot_onVolumeChanged() ) );
    connect(Engine::instance(), SIGNAL( engineStateChanged() ), SLOT(slot_engineStateChanged()));
    connect(Engine::instance(), SIGNAL( mediaChanged() ), SLOT(slot_engineMediaChanged()));
    connect(Engine::instance(), SIGNAL( mediaTick( qint64 ) ),SLOT( slot_mediaTick( qint64 ) ) );
    
    syncProperties();
}


Mpris2::~Mpris2()
{
    QDBusConnection::sessionBus().unregisterService("org.mpris.MediaPlayer2.yarock");
}


/*
********************************************************************************
*                                                                              *
*    org.mpris.MediaPlayer2                                                    *
*                                                                              *
********************************************************************************
*/
bool Mpris2::canQuit() const
{
    return true;
}


bool Mpris2::canRaise() const
{
    return true;
}

bool Mpris2::hasTrackList() const
{
    return false;
}


QString Mpris2::identity() const
{
    return QString( "yarock" );
}


QString Mpris2::desktopEntry() const
{
    return QString( "yarock" );
}


QStringList Mpris2::supportedUriSchemes() const
{
    static QStringList res = QStringList()
                           << "file"
                           << "http";
    return res;
}


QStringList Mpris2::supportedMimeTypes() const
{
    static QStringList res = QStringList()
                           << "application/ogg"
                           << "application/x-ogg"
                           << "application/x-ogm-audio"
                           << "audio/aac"
                           << "audio/mp4"
                           << "audio/mpeg"
                           << "audio/mpegurl"
                           << "audio/ogg"
                           << "audio/vnd.rn-realaudio"
                           << "audio/vorbis"
                           << "audio/x-flac"
                           << "audio/x-mp3"
                           << "audio/x-mpeg"
                           << "audio/x-mpegurl"
                           << "audio/x-ms-wma"
                           << "audio/x-musepack"
                           << "audio/x-oggflac"
                           << "audio/x-pn-realaudio"
                           << "audio/x-scpls"
                           << "audio/x-speex"
                           << "audio/x-vorbis"
                           << "audio/x-vorbis+ogg"
                           << "audio/x-wav"
                           << "video/x-ms-asf"
                           << "x-content/audio-player";
    return res;
}


void Mpris2::Raise() 
{
//     Debug::debug() << "  [Mpris2] Raise Application";    
    emit RaiseMainWindow();
}


void Mpris2::Quit()
{
    QApplication::quit();
}

/*
********************************************************************************
*                                                                              *
*    org.mpris.MediaPlayer2.Player                                             *
*                                                                              *
********************************************************************************
*/
bool Mpris2::canControl() const
{
    return true;
}


bool Mpris2::canGoNext() const
{
    return true;
}


bool Mpris2::canGoPrevious() const
{
    return true;
}


bool Mpris2::canPause() const
{
    return (playbackStatus() == "Paused" || playbackStatus() == "Stopped");
}


bool Mpris2::canPlay() const
{
    /* to be improved */
    return true;
}


bool Mpris2::canSeek() const
{
    /* to be improved */
    return true;
}


QString Mpris2::loopStatus() const
{
    //! class RepeatControl ==> enum RepeatMode {RepeatOff = 0,RepeatTrack = 1, RepeatAll = 2 };
    int state = RepeatControl::instance()->getState();

    if(state == 0)          /*RepeatOff */
      return QString("None");
    else if(state == 1)     /*RepeatTrack */
      return QString("Track");
    else if (state == 2)    /*RepeatAll */
      return QString("Playlist");
    else
      return QString("None");
}


void Mpris2::setLoopStatus( const QString& value )
{
    //! class RepeatControl ==> enum RepeatMode {RepeatOff = 0,RepeatTrack = 1, RepeatAll = 2 };
    int state = 0;

    if (value == "None")
      state = 0;
    else if (value == "Track")
      state = 1;
    else if (value == "Playlist")
      state = 2;

    RepeatControl::instance()->setState(state);
}


double Mpris2::maximumRate() const
{
    return 1.0;
}

double Mpris2::minimumRate() const
{
    return 1.0;
}

QVariantMap Mpris2::metadata() const
{
    QVariantMap metadataMap;

    /* get media track playing */ 
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();
    if(!track)
      return metadataMap;
    
    if ( track )
    {

        /* Each set of metadata must have a "mpris:trackid" entry at the very least,
         * which contains a string that uniquely identifies this track within the scope of the tracklist.*/
        metadataMap.insert( "mpris:trackid", track->id );

        metadataMap.insert( "xesam:url",    track->url );
        metadataMap.insert( "xesam:album",  track->album );
        metadataMap.insert( "xesam:artist", track->artist );
        metadataMap.insert( "xesam:title",  track->title );
        metadataMap.insert( "xesam:genre",  track->genre );
        metadataMap.insert( "mpris:length", static_cast<qlonglong>(track->duration) * 1000000 );

        const QString coverpath = CoverCache::instance()->coverPath( track );
        if( !coverpath.isEmpty() )
          metadataMap.insert( "mpris:artUrl", QString( QUrl::fromLocalFile( coverpath ).toEncoded() ) );
        
        if (track->rating != -1.0)
          metadataMap.insert("xesam:userRating", static_cast<double>(track->rating));
    }

    return metadataMap;
}



QString Mpris2::playbackStatus() const
{
    switch (Engine::instance()->state()) 
    {
      case ENGINE::PLAYING : return "Playing"; break;
      case ENGINE::PAUSED  : return "Paused";  break;
      default              : return "Stopped"; break;
    }    
}


qlonglong Mpris2::position() const
{
    /* convert in microsecond */
    return Engine::instance()->currentTime() * 1000;
}


double Mpris2::rate() const
{
    float rating = 0.0;
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();
    
    if( track && track->id != -1) 
      rating = track->rating;

    return rating;
}


void Mpris2::setRate( double value )
{
    Q_UNUSED( value );
}


bool Mpris2::shuffle() const
{
    return (ShuffleControl::instance()->getState() == 1) ? true : false;
}


void Mpris2::setShuffle( bool value )
{
    ShuffleControl::instance()->setState(value);
}


double Mpris2::volume() const
{
    return (Engine::instance()->volume() / 100.0);
}


void Mpris2::setVolume( double value )
{
    Engine::instance()->setVolume(value * 100.0);
}


void Mpris2::Next()
{
    Debug::debug() << "  [Mpris2] Next";
    if(true /*CanGoNext() */)
      ACTIONS()->value(ENGINE_PLAY_NEXT)->trigger();
}


void Mpris2::Previous()
{
    Debug::debug() << "  [Mpris2] Previous";
    if(true /*CanGoPrevious()*/)
      ACTIONS()->value(ENGINE_PLAY_PREV)->trigger();

}


void Mpris2::Pause()
{
    Debug::debug() << "  [Mpris2] Pause";
    if(/*CanPause() && */ Engine::instance()->state() != ENGINE::PAUSED)
      ACTIONS()->value(ENGINE_PLAY)->trigger();
}

void Mpris2::Play()
{
    Debug::debug() << "  [Mpris2] Play";
    if(true /*CanPlay()*/)
      ACTIONS()->value(ENGINE_PLAY)->trigger();    
}


void Mpris2::PlayPause()
{
    Debug::debug() << "  [Mpris2] PlayPause";
    if(true /*CanPlay()*/)
      ACTIONS()->value(ENGINE_PLAY)->trigger();   
} 
  



void Mpris2::Seek( qlonglong Offset )
{
    Debug::debug() << "  [Mpris2] Seek offset:" << Offset;
    
    if(true /*CanSeek() */)
    {
      int msec = Engine::instance()->currentTime() + Offset/1000;
      Engine::instance()->seek(msec);
    }
}


void Mpris2::SetPosition( const QDBusObjectPath& TrackId, qlonglong Position )
{
Q_UNUSED(TrackId)
    Debug::debug() << "  [Mpris2]  SetPosition:" << Position;

    /* add check for track_id */
    int msec = Position/1000;
    Engine::instance()->seek( msec );
}

void Mpris2::OpenUri(const QString& uri)
{
Q_UNUSED(uri)
/* TODO */
}


void Mpris2::Stop()
{
    Debug::debug() << "  [Mpris2] stop";
    ACTIONS()->value(ENGINE_STOP)->trigger();
}


void Mpris2::slot_onVolumeChanged(  )
{
    Debug::debug() << "  [Mpris2] slot_onVolumeChanged";
    EmitNotification("Volume");
}

void Mpris2::slot_engineStateChanged()
{
    Debug::debug() << "  [Mpris2] slot_engineStateChanged";
    ENGINE::E_ENGINE_STATE engine_state = Engine::instance()->state();

    if(engine_state != ENGINE::PLAYING && engine_state != ENGINE::PAUSED) {
      // clear metadata
      EmitNotification("Metadata");
    }
    EmitNotification("PlaybackStatus");
}
 
 
void Mpris2::slot_engineMediaChanged()
{
    /* get media track playing */ 
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();
    //Debug::debug() << "  [Mpris2] slot_engineMediaChanged  track " << track;
    
    EmitNotification("Metadata");  
    EmitNotification("Rate");
}

void Mpris2::slot_mediaTick( qint64 ms ) 
{
    qlonglong us = (qlonglong) ( ms*1000 );
    emit Seeked( us ); 
}
    
 
void Mpris2::EmitNotification(const QString& name)
{
//     Debug::debug() << "  [Mpris2] EmitNotification " << name << "Properties :" << property(name.toLatin1());

    QDBusMessage signal = QDBusMessage::createSignal(
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged" );

    signal << "org.mpris.MediaPlayer2.Player";
    
    QVariantMap changedProps;
    changedProps.insert(name, property(name.toLatin1()));
    signal << changedProps;
    signal << QStringList();
    
    QDBusConnection::sessionBus().send(signal);
}


void Mpris2::syncProperties()
{
    QMap<QString, QVariant> props;
  
    props["CanGoNext"]      = canGoNext();
    props["CanGoPrevious"]  = canGoPrevious();
    props["CanPause"]       = canPause();
    props["CanPlay"]        = canPlay();
    props["CanSeek"]        = canSeek();
    props["LoopStatus"]     = loopStatus();
    props["MaximumRate"]    = maximumRate();
    props["MinimumRate"]    = minimumRate();
    props["PlaybackStatus"] = playbackStatus();
    props["Rate"]           = rate();
    props["Shuffle"]        = shuffle();
    props["Volume"]         = volume();
    props["Metadata"]       = metadata();
    
    QVariantMap changedProps;
    foreach(QString name, props.keys())
      changedProps.insert(name, property(name.toLatin1()));

    QDBusMessage signal = QDBusMessage::createSignal(
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged" );

    signal << "org.mpris.MediaPlayer2.Player";
    signal << changedProps;
    signal << QStringList();
    
    QDBusConnection::sessionBus().send(signal); 
}

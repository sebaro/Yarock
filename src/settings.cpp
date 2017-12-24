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

#include "settings.h"
#include "utilities.h"

#include "infosystem/services/service_lyrics.h"
#include "debug.h"

#include <QBuffer>
#include <QKeySequence>

YarockSettings* YarockSettings::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    YarockSettings                                                            *
*                                                                              *
********************************************************************************
*/
YarockSettings::YarockSettings()
{
    INSTANCE    = this;
    s = new QSettings(UTIL::CONFIGFILE,QSettings::IniFormat,0);
}


void YarockSettings::readSettings()
{
    Debug::debug() << "[Settings] readSettings";
    
    // ------ window elements (Startup settings)
    _windowsGeometry     = s->value("Window/geometry").toByteArray();
    _windowsState        = s->value("Window/state").toByteArray();
    _splitterState_1     = s->value("Window/splitter1").toByteArray();
    _splitterState_2     = s->value("Window/splitter2").toByteArray();

    _showPlayQueuePanel  = s->value("Window/showPlaylistPanel", true).toBool();
    _enableSearchPopup   = s->value("Window/searchPopup",       true).toBool();
    _enablePlayOnSearch  = s->value("Window/playOnSearch",      false).toBool();
    
    // ------ session
#ifdef ENABLE_PHONON
    _engine              = s->value("Session/engine",              1).toInt();    // default = phonon
#elif ENABLE_VLC  
    _engine              = s->value("Session/engine",              2).toInt();    // default = vlc
#else    
    _engine              = s->value("Session/engine",              0).toInt();    // default = null engine
#endif
    
    _browserScroll       = s->value("Session/browserScroll",       0).toInt();  
    _viewMode            = s->value("Session/viewMode",            1).toInt();    // default = View About
    _album_view_type     = s->value("Session/album_view_type",     1).toInt();    // default = Extended
    _playlist_view_type  = s->value("Session/playlist_view_type",  0).toInt();    // default = Overview

    _playqueueShowCover  = s->value("Session/playqueueShowCover",   true).toBool();
    _playqueueShowRating = s->value("Session/playqueueShowRating",  true).toBool();
    _playqueueShowNumber = s->value("Session/playqueueShowNumber",  false).toBool();
    _playqueueDuplicate  = s->value("Session/playqueueDuplicate",   true).toBool();
    _playqueueShowFilter = s->value("Session/playqueueShowFilter",  false).toBool();

    _filesystem_path     = s->value("Session/filesystem",     "").toString();
    
    _hideAtStartup       = s->value("Session/hideAtStartup", false).toBool();

    _isbottombarexpanded = s->value("Session/expandBottomBar", true).toBool();
    
    // ------ cover size
    _coverSize           = s->value("Session/coversize", 200).toUInt();
    
    if(_coverSize < 128 || _coverSize > 256)
        _coverSize = 200;
    
    // ------ handle color
    if(s->contains("Session/color")) 
    {
        try 
        { 
            QByteArray bytes = s->value("Session/color").toByteArray();
            
            QBuffer buf(&bytes);            
            buf.open(QIODevice::ReadOnly);

            QDataStream stream(&buf);
            stream >> _baseColor;
        }
        catch (...)
        {
            _baseColor = QColor(0xfca822);
        }
    }
    else
    {
      _baseColor = QColor(0xfca822);      
    }
    updateCheckedColor();

    
    // ------ features activations (Dynamic settings)
    _useTrayIcon         = s->value("Features/systray", false).toBool();
    _useMpris            = s->value("Features/mpris",   true).toBool();
    _useDbusNotification = s->value("Features/dbus",    false).toBool();
    _useLastFmScrobbler  = s->value("Features/lastFm",  false).toBool();
    _useShortcut         = s->value("Features/shortcut", false).toBool();
    _useHistory          = s->value("Features/history", true).toBool();

    // ------ song info 
    _lyrics_providers =  s->value("SongInfo/providers",  ServiceLyrics::defaultProvidersList()).toStringList();
    
    // ------ audio controler (Startup settings)
    _repeatMode          = s->value("AudioControl/repeat",  0).toInt();
    _shuffleMode         = s->value("AudioControl/shuffle", 0).toInt();
    _volumeLevel         = s->value("AudioControl/volume",  75).toInt();
    _replaygain          = s->value("AudioControl/replaygain", 0).toInt();

    // ------ playback option
    _stopOnPlayqueueClear     = s->value("PlaybackOption/stopOnclear", false).toBool();
    _restorePlayqueue         = s->value("PlaybackOption/restorePlayqueue", false).toBool();
    _restartPlayingAtStartup  = s->value("PlaybackOption/restartPlaying", false).toBool();
    _pauseOnScreenSaver       = s->value("PlaybackOption/screenSaverPause", false).toBool();
    _stopOnScreenSaver        = s->value("PlaybackOption/screenSaverStop", false).toBool();

    // ------ playing media
    _url        = s->value("PlayingMedia/url", "").toString();
    _station    = s->value("PlayingMedia/station", "").toString();
    _position   = s->value("PlayingMedia/position", 0).toDouble();

    // ------ Shortcut media key
    _shortcutsKey["play"]        = s->value("Shortcuts/play",QKeySequence(Qt::Key_MediaPlay).toString()).toString();
    _shortcutsKey["stop"]        = s->value("Shortcuts/stop",QKeySequence(Qt::Key_MediaStop).toString()).toString();
    _shortcutsKey["prev_track"]  = s->value("Shortcuts/prev_track","Meta+Left").toString();
    _shortcutsKey["next_track"]  = s->value("Shortcuts/next_track","Meta+Right").toString();
    _shortcutsKey["inc_volume"]  = s->value("Shortcuts/inc_volume","Ctrl+Up").toString();
    _shortcutsKey["dec_volume"]  = s->value("Shortcuts/dec_volume","Ctrl+Down").toString();
    _shortcutsKey["mute_volume"] = s->value("Shortcuts/mute_volume","Meta+M").toString();
    _shortcutsKey["jump_to_track"] = s->value("Shortcuts/jump_to_track","Meta+J").toString();
    _shortcutsKey["clear_playqueue"] = s->value("Shortcuts/clear_playqueue","Ctrl+K").toString();

    // ------ Equalizer settings
    const int count = s->beginReadArray("Equalizer/presets");
    for (int i=0 ; i<count ; ++i)
    {
      s->setArrayIndex(i);
      _presetEq[ s->value("name").toString() ] = s->value("params").value<Equalizer::EqPreset>();
    }
    s->endArray();

    _currentPreset = s->value("Equalizer/selected_preset", "Custom").toString();
    _enableEq      = s->value("Equalizer/enabled", false).toBool();
}

void YarockSettings::writeSettings()
{
    Debug::debug() << "[Settings] writeSettings";
  
    // window elements (Startup settings)
    s->beginGroup("Window");
    s->setValue("geometry",           _windowsGeometry);
    s->setValue("state",              _windowsState);
    s->setValue("splitter1",          _splitterState_1);
    s->setValue("splitter2",          _splitterState_2);

    s->setValue("showPlaylistPanel",  _showPlayQueuePanel);
    s->setValue("searchPopup",        _enableSearchPopup);
    s->setValue("playOnSearch",       _enablePlayOnSearch);
    
    s->endGroup();

    // session
    s->beginGroup("Session");
    s->setValue("engine",             _engine);
    s->setValue("browserScroll",      _browserScroll);
    s->setValue("viewMode",           _viewMode);
    s->setValue("album_view_type",    _album_view_type);
    s->setValue("playlist_view_type", _playlist_view_type);
    s->setValue("playqueueShowCover", _playqueueShowCover);
    s->setValue("playqueueShowRating",_playqueueShowRating);
    s->setValue("playqueueShowNumber",_playqueueShowNumber);
    s->setValue("playqueueDuplicate", _playqueueDuplicate);
    s->setValue("playqueueShowFilter",_playqueueShowFilter);
    s->setValue("hideAtStartup",      _hideAtStartup);
    s->setValue("filesystem",         _filesystem_path);
    s->setValue("coversize",          _coverSize);
    s->setValue("expandBottomBar",    _isbottombarexpanded);
     
      /* handle color */
      QByteArray byteArray;      
      QBuffer buffer(&byteArray);
      buffer.open(QIODevice::WriteOnly);
    
      QDataStream stream(&buffer);
      stream << _baseColor;
      buffer.close();

      s->setValue("color",             byteArray);

    s->endGroup();

    // features activations (Dynamic settings)
    s->beginGroup("Features");
    s->setValue("systray",           _useTrayIcon);
    s->setValue("mpris",             _useMpris);
    s->setValue("dbus",              _useDbusNotification);
    s->setValue("lastFm",            _useLastFmScrobbler);
    s->setValue("shortcut",          _useShortcut);
    s->setValue("history",           _useHistory);
    s->endGroup();

    // song info 
    s->beginGroup("SongInfo");
    s->setValue("providers",          _lyrics_providers);
    s->endGroup();
    
    // audio controler (Startup settings)
    s->beginGroup("AudioControl");
    s->setValue("repeat",            _repeatMode);
    s->setValue("shuffle",           _shuffleMode);
    s->setValue("volume",            _volumeLevel);
    s->setValue("replaygain",        _replaygain);
    s->endGroup();

    // playback option
    s->beginGroup("PlaybackOption");
    s->setValue("stopOnclear",       _stopOnPlayqueueClear);
    s->setValue("restorePlayqueue",  _restorePlayqueue);
    s->setValue("restartPlaying",    _restartPlayingAtStartup);
    s->setValue("screenSaverPause",  _pauseOnScreenSaver);
    s->setValue("screenSaverStop",   _stopOnScreenSaver);
    s->endGroup();
    
    // ------ playing media
    s->beginGroup("PlayingMedia");
    s->setValue("url",        _url);
    s->setValue("station",    _station);
    s->setValue("position",   _position);
    s->endGroup();

    // Shurtcut media key
    s->beginGroup("Shortcuts");
    s->setValue("play",             _shortcutsKey["play"]);
    s->setValue("stop",             _shortcutsKey["stop"]);
    s->setValue("prev_track",       _shortcutsKey["prev_track"]);
    s->setValue("next_track",       _shortcutsKey["next_track"]);
    s->setValue("inc_volume",       _shortcutsKey["inc_volume"]);
    s->setValue("dec_volume",       _shortcutsKey["dec_volume"]);
    s->setValue("mute_volume",      _shortcutsKey["mute_volume"]);
    s->setValue("jump_to_track",    _shortcutsKey["jump_to_track"]);
    s->setValue("clear_playqueue",  _shortcutsKey["clear_playqueue"]);
    s->endGroup();

    // Equalizer settings
    s->beginGroup("Equalizer");
    s->beginWriteArray("presets", _presetEq.count());
    int i = 0;
    foreach (const QString& name, _presetEq.keys()) {
      s->setArrayIndex(i++);
      s->setValue("name", name);
      s->setValue("params", QVariant::fromValue(_presetEq[name]));
    }
    s->endArray();
    s->setValue("selected_preset", _currentPreset);
    s->setValue("enabled", _enableEq);
    s->endGroup();

    // final sync to write setting to file
    s->sync ();
}

void YarockSettings::updateCheckedColor()
{
    QRgb rgb = _baseColor.rgb();
    
    if( qBlue(rgb) > qRed(rgb) && qBlue(rgb) > qGreen(rgb) )
        _checkedColor = QColor(0x000099);
    else if ( qGreen(rgb) > qRed(rgb) && qGreen(rgb) > qBlue(rgb) )
        _checkedColor = QColor(0x009900);
    else if ( qRed(rgb) > qBlue(rgb) && qRed(rgb) > qGreen(rgb) )
        _checkedColor = QColor(0x990000);
    else
        _checkedColor = QColor(0x990000);
}



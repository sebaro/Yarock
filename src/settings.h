/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2016 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

#ifndef _YAROCK_SETTINGS_H_
#define _YAROCK_SETTINGS_H_

//! local
#include "widgets/equalizer/equalizer_preset.h"

//! qt
#include <QSettings>
#include <QString>
#include <QByteArray>
#include <QColor>

namespace SETTING {
enum ReplayGainMode
{
  ReplayGainOff     = 0,
  TrackReplayGain   = 1, // All tracks should be equally loud.  Also known as Radio mode
  AlbumReplayGain   = 2  // All albums should be equally loud.  Also known as Audiophile mode
};

}
/*
********************************************************************************
*                                                                              *
*    YarockSettings                                                            *
*                                                                              *
********************************************************************************
*/

#define SETTINGS() (YarockSettings::instance())

class YarockSettings : public QObject
{
Q_OBJECT
Q_DISABLE_COPY(YarockSettings)

    static YarockSettings* INSTANCE;

  public:
    static YarockSettings* instance() { return INSTANCE; }

    YarockSettings();

    void writeSettings();
    void readSettings();

    void updateCheckedColor();
    
  public:
    // features activations (Dynamic settings)
    bool                 _useTrayIcon;
    bool                 _useMpris;
    bool                 _useDbusNotification;
    bool                 _useLastFmScrobbler;
    bool                 _useShortcut;

    // show/hide widget (Startup settings)
    bool                 _showPlayQueuePanel;
    bool                 _enableSearchPopup;
    bool                 _enablePlayOnSearch;

    // interface option
    QColor               _baseColor;
    QColor               _checkedColor;
    
    //  session elements (Startup settings)
    int                  _engine;

    QByteArray           _windowsGeometry;
    QByteArray           _windowsState;
    QByteArray           _splitterState_1;
    QByteArray           _splitterState_2;

    int                  _browserScroll;
    int                  _viewMode;            // 1 to 16
    int                  _album_view_type;     // 0 : grid, 1: extended
    int                  _playlist_view_type;  // 0 : overview, 1: by_tracks
    bool                 _playqueueShowCover; 
    bool                 _playqueueShowRating;
    bool                 _playqueueShowNumber;
    bool                 _playqueueDuplicate;  // false = remove duplicate
    bool                 _playqueueShowFilter;
    bool                 _hideAtStartup;
    QString              _filesystem_path;
    

    // song info 
    QStringList          _lyrics_providers;

    // audio controler (Startup settings)
    int                  _repeatMode;
    int                  _shuffleMode;
    int                  _volumeLevel;
    int                  _replaygain; // 0 to 2

    // playback option
    bool                 _stopOnPlayqueueClear;
    bool                 _restorePlayqueue;
    bool                 _restartPlayingAtStartup;
    bool                 _pauseOnScreenSaver;
    bool                 _stopOnScreenSaver;
    QString              _playingUrl;
    qint64               _playingPosition;

    // _shortcuts
    QMap<QString, QString> _shortcutsKey;  //shortcutName, KeySequence

    // equalizer
    QMap<QString, Equalizer::EqPreset> _presetEq;
    bool                 _enableEq;
    QString              _currentPreset;

  private:
    QSettings           *s;
};


#endif // _YAROCK_SETTINGS_H_

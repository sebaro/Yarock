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
// based on Clementine Player, thanks for the good work


#ifndef _COMMANDLINEOPTIONS_H_
#define _COMMANDLINEOPTIONS_H_

#include <QList>
#include <QUrl>
#include <QDataStream>

/*
********************************************************************************
*                                                                              *
*    Class CommandlineOptions                                                  *
*                                                                              *
********************************************************************************
*/
class CommandlineOptions
{
  friend QDataStream& operator<<(QDataStream& s, const CommandlineOptions& a);
  friend QDataStream& operator>>(QDataStream& s, CommandlineOptions& a);

  public:
    CommandlineOptions(int argc = 0, char** argv = NULL);

    static const char* kHelpText;

    // Value have to be unchanged between 2 different yarock version
    enum E_PLAYER_ACTION {
       Player_None       = 0,
       Player_Play       = 1,
       Player_PlayPause  = 2,
       Player_Pause      = 3,
       Player_Stop       = 4,
       Player_Previous   = 5,
       Player_Next       = 6
    };

    enum E_PLAYLIST_ACTION {
       Playlist_Default  = 0,
       Playlist_Append   = 1,
       Playlist_Load     = 2
    };


    bool Parse();
    bool isEmpty() const;

    E_PLAYER_ACTION player_action() const { return _player_action; }
    E_PLAYLIST_ACTION playlist_action() const { return _playlist_action; }
    int set_volume() const { return _set_volume; }
    int volume_modifier() const { return _volume_modifier; }
    int seek_to() const { return _seek_to; }
    int seek_by() const { return _seek_by; }
    int play_track_at() const { return _play_track_at; }
    QList<QUrl> urls() const { return _urls; }
    QString language() const { return _language; }

    bool debug() const {return _debug;}

    QByteArray Serialize() const;
    void Load(const QByteArray& serialized);

  private:
    // These are "invalid" characters to pass to getopt_long for options that
    // shouldn't have a short (single character) option.
    enum E_LONG_OPTIONS {
      VolumeUp      = 256,
      VolumeDown,
      SeekTo,
      SeekBy,
    };

    QString tr(const char* source_text);
    void RemoveArg(const QString& starts_with, int count);

    int                  _argc;
    char**               _argv;

    E_PLAYER_ACTION      _player_action;
    E_PLAYLIST_ACTION    _playlist_action;
    int                  _set_volume;
    int                  _volume_modifier;
    int                  _seek_to;
    int                  _seek_by;
    int                  _play_track_at;
    QString              _language;
    bool                 _debug;

    QList<QUrl>          _urls;
};


/*******************************************************************************
    QDataStream operator
*******************************************************************************/
QDataStream& operator<<(QDataStream& s, const CommandlineOptions& a);
QDataStream& operator>>(QDataStream& s, CommandlineOptions& a);

#endif // _COMMANDLINEOPTIONS_H_

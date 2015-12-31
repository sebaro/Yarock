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

// based on Clementine Player, thanks for the good work

#include "commandlineoptions.h"
// #include "debug.h"


#include <cstdlib>
#include <getopt.h>
#include <iostream>

#include <QBuffer>
#include <QFileInfo>

const char* CommandlineOptions::kHelpText =
    "%1: yarock [%2] [%3]\n"
    "\n"
    "%4:\n"
    "  -p, --play                %5\n"
    "  -t, --play-pause          %6\n"
    "  -u, --pause               %7\n"
    "  -s, --stop                %8\n"
    "  -r, --previous            %9\n"
    "  -f, --next                %10\n"
    "  -v, --volume <value>      %11\n"
    "  --volume-up               %12\n"
    "  --volume-down             %13\n"
    "  --seek-to <seconds>       %14\n"
    "  --seek-by <seconds>       %15\n"
    "\n"
    "%16:\n"
    "  -a, --append              %17\n"
    "  -l, --load                %18\n"
    "  -k, --play-track <n>      %19\n"
    "\n"
    "%20:\n"
    "  -d, --debug               %21\n";

/*
********************************************************************************
*                                                                              *
*    Class CommandlineOptions                                                  *
*                                                                              *
********************************************************************************
*/
CommandlineOptions::CommandlineOptions(int argc, char** argv)
{
    //! init class value
    _argc              = argc;
    _argv              = argv;
    _player_action     = Player_None;
    _playlist_action   = Playlist_Default;
    _set_volume        = -1;
    _volume_modifier   =  0;
    _seek_to           = -1;
    _seek_by           =  0;
    _play_track_at     = -1;
    _debug             = false;

    //! Remove the -session option that KDE passes
    RemoveArg("-session", 2);
}


void CommandlineOptions::RemoveArg(const QString& starts_with, int count)
{
  for (int i = 0; i < _argc; ++i) {
    QString opt(_argv[i]);
    if (opt.startsWith(starts_with)) {
      for (int j = i; j < _argc - count + 1; ++j) {
        _argv[j] = _argv[j+count];
      }
      _argc -= count;
      break;
    }
  }
}


bool CommandlineOptions::Parse()
{
    //qDebug("CommandlineOptions::Parse");

    static const struct option kOptions[] = {
      {"help",        no_argument,       0, 'h'},

      {"play",        no_argument,       0, 'p'},
      {"play-pause",  no_argument,       0, 't'},
      {"pause",       no_argument,       0, 'u'},
      {"stop",        no_argument,       0, 's'},
      {"previous",    no_argument,       0, 'r'},
      {"next",        no_argument,       0, 'f'},
      {"volume",      required_argument, 0, 'v'},
      {"volume-up",   no_argument,       0, VolumeUp},
      {"volume-down", no_argument,       0, VolumeDown},
      {"seek-to",     required_argument, 0, SeekTo},
      {"seek-by",     required_argument, 0, SeekBy},

      {"append",      no_argument,       0, 'a'},
      {"load",        no_argument,       0, 'l'},
      {"play-track",  required_argument, 0, 'k'},

      {"debug",       no_argument,       0, 'd'},

      {0, 0, 0, 0}
    };

    // Parse the arguments
    bool ok = false;
    forever
    {
      int c = getopt_long(_argc, _argv, "hptusrfv:alk:d:", kOptions, NULL);

      // End of the options
      if (c == -1) break;

      switch (c) {
        case 'h':
        {
          QString translated_help_text = QString(kHelpText).arg(
            tr("Usage"), tr("options"), tr("URL(s)"), tr("Player options"),
            tr("Start playback"),
            tr("Play if stopped, pause if playing"),
            tr("Pause playback"),
            tr("Stop playback"),
            tr("Skip backwards in playlist")).arg(
            tr("Skip forwards in playlist"),
            tr("Set the volume to <value> percent"),
            tr("Increase the volume by 4%"),
            tr("Decrease the volume by 4%"),
            tr("Seek to an absolute position"),
            tr("Seek by a relative amount"),
            tr("Playlist options"),
            tr("Append files/URLs to the playlist"),
            tr("Loads files/URLs, replacing current playlist")).arg(
            tr("Play the <n>th track in the playlist"),
            tr("Other options"),
            tr("Print debug information"));


          std::cout << translated_help_text.toLocal8Bit().constData();
          return false;
        }

        case 'p': _player_action   = Player_Play;      break;
        case 't': _player_action   = Player_PlayPause; break;
        case 'u': _player_action   = Player_Pause;     break;
        case 's': _player_action   = Player_Stop;      break;
        case 'r': _player_action   = Player_Previous;  break;
        case 'f': _player_action   = Player_Next;      break;

        case 'a': _playlist_action = Playlist_Append;   break;
        case 'l': _playlist_action = Playlist_Load;     break;
        case 'g': _language        = QString(optarg);  break;
        case VolumeUp:   _volume_modifier = +4;        break;
        case VolumeDown: _volume_modifier = -4;        break;

        case 'v':
          _set_volume = QString(optarg).toInt(&ok);
          if (!ok) _set_volume = -1;
          break;

        case SeekTo:
          _seek_to = QString(optarg).toInt(&ok);
          if (!ok) _seek_to = -1;
          break;

        case SeekBy:
          _seek_by = QString(optarg).toInt(&ok);
          if (!ok) _seek_by = 0;
          break;

        case 'k':
          _play_track_at = QString(optarg).toInt(&ok);
          if (!ok) _play_track_at = -1;
          break;

        case 'd':  _debug = true; break;

        case '?':
        default:
          return false;
      }
    }

  // Get any filenames or URLs following the arguments
  for (int i=optind ; i<_argc ; ++i) {
    QString value = QFile::decodeName(_argv[i]);
    if (value.contains("://"))
      _urls << value;
    else
      _urls << QUrl::fromLocalFile(QFileInfo(value).canonicalFilePath());
  }

  return true;
}


bool CommandlineOptions::isEmpty() const
{
  return _player_action   == Player_None &&
         _playlist_action == Playlist_Default &&
         _set_volume      == -1 &&
         _volume_modifier ==  0 &&
         _seek_to         == -1 &&
         _seek_by         ==  0 &&
         _play_track_at   == -1 &&
         _urls.isEmpty();
}

QByteArray CommandlineOptions::Serialize() const
{
    QBuffer buf;
    buf.open(QIODevice::WriteOnly);

    QDataStream s(&buf);
    s << *this;
    buf.close();

    return buf.data();
}


void CommandlineOptions::Load(const QByteArray &serialized)
{
    QByteArray copy(serialized);
    QBuffer buf(&copy);
    buf.open(QIODevice::ReadOnly);

    QDataStream s(&buf);
    s >> *this;
}


QString CommandlineOptions::tr(const char *source_text)
{
  return QObject::tr(source_text);
}


/*******************************************************************************
    QDataStream operator
*******************************************************************************/
QDataStream& operator<<(QDataStream& s, const CommandlineOptions& a)
{
    s << qint32(a._player_action)
      << qint32(a._playlist_action)
      << a._set_volume
      << a._volume_modifier
      << a._seek_to
      << a._seek_by
      << a._play_track_at
      << a._urls;

    return s;
}

QDataStream& operator>>(QDataStream& s, CommandlineOptions& a)
{
    quint32 player_action   = 0;
    quint32 playlist_action = 0;

    s >> player_action
      >> playlist_action
      >> a._set_volume
      >> a._volume_modifier
      >> a._seek_to
      >> a._seek_by
      >> a._play_track_at
      >> a._urls;

    a._player_action   = CommandlineOptions::E_PLAYER_ACTION(player_action);
    a._playlist_action = CommandlineOptions::E_PLAYLIST_ACTION(playlist_action);

    return s;
}

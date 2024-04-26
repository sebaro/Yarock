
About
----------------------------------------------------------------------------------------------
Yarock is a modern looking music player, packed with features, that doesn’t depend on any specific desktop environment.
Yarock is designed to provide an easy and pretty music browser based on cover art. Yarock is easy to build with a minimal set of dependancies, and offers the choose of differents audio back-end.

![](https://gitlab.com/sebaro/Yarock/raw/main/screenshot.png)


Features
----------------------------------------------------------------------------------------------
  * Browse artist photos, album covers
  * Music collection database (SQLite 3)
  * Easy search and filter music collection
  * Manage favorites item (album, artist)
  * Play music directly from collection or playqueue
  * Simple playqueue
  * Smart playlist generator
  * Support mp3,Ogg Vorbis,flac music files (depending on audio engine)
  * Support load/save playlist file (m3u, pls, xspf)
  * Play radio stream (tunein, dirble, radionomy,...)
  * Mp3Gain tag support for volume normalization
  * Contextual info & cover art download (lastfm, echonest, musicbrainz)
  * Supports scrobbling to Last.fm
  * Command line interface, Mpris interface
  * Clean and simple user interface
  * No GNOME or KDE dependancies


Dependencies
----------------------------------------------------------------------------------------------

Build:

 * c++-17 compiler
 * cmake >= 3.16
 * make

GUI:

 * qt6
 * taglib
 * htmlcxx

Engines:

 * mpv (libmpv)
 * vlv (libvlc)
 * phonon
 * qtmultimedia


Compiling from source
----------------------------------------------------------------------------------------------

#### Building from command line

    mkdir build && cd build
    cmake ..
    make

#### Install

    make install (as root)

#### Audio engine

You can build yarock with alternate audio engine (vlc, phonon or mpv).

If you want enable/disable audio engine use the following cmake option :

    mkdir build && cd build
    cmake .. -DENABLE_VLC=ON -DENABLE_MPV=ON -DENABLE_PHONON=ON -DENABLE_QTMULTIMEDIA=ON
    make
    make install (as root)


Tips & Tricks
----------------------------------------------------------------------------------------------

#### Change systray icon

You can use your on icon for yarock in the systray by using the file

    /home/your name/.config/yarock/systray_icon.png


Original version (Qt4/Qt5)
----------------------------------------------------------------------------------------------

  * Website : http://seb-apps.github.io/yarock/
  * Bugreport: https://bugs.launchpad.net/yarock
  * Translate : https://www.transifex.net/projects/p/yarock-translation/
  * Donate: http://qt-apps.org/content/donate.php?content=129372

Copyright (C) 2010-2019 Sébastien Amardeilh sebastien.amardeilh+yarock@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.


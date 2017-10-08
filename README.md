
About
----------------------------------------------------------------------------------------------
Yarock is a modern looking music player, packed with features, that doesn’t depend on any specific desktop environment. 
Yarock is designed to provide an easy and pretty music browser based on cover art. Yarock is easy to build with a minimal set of dependancies, and offers the choose of differents audio back-end.

  * Website : http://seb-apps.github.io/yarock/
  * Bugreport: https://bugs.launchpad.net/yarock
  * Translate : https://www.transifex.net/projects/p/yarock-translation/
  * Donate: http://qt-apps.org/content/donate.php?content=129372


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

Qt4 dependencies:

 * qt4-devel
 * qjson-devel
 * taglib-devel, taglib-extras-devel
 * htmlcxx, htmlcxx-devel
 * phonon-devel [vlc-devel, libmpv-devel]

Qt5 dependencies:

 * qt5-qtbase-devel
 * qt5-qtx11extras-devel
 * qt5-linguist
 * phonon-qt5-devel
 * phonon-devel, [vlc-devel, libmpv-devel]

Links

 * CMake        http://www.cmake.org/
 * Qt           http://qt-project.org
 * QJson        http://qjson.sourceforge.net/
 * TagLib       http://developer.kde.org/~wheeler/taglib.html
 * htmlcxx      http://htmlcxx.sourceforge.net/
 * Phonon       http://phonon.kde.org/
 * Libvlc       http://www.videolan.org/vlc/libvlc.html
 * Libmpv       http://mpv.io/


Dependencies installation
----------------------------------------------------------------------------------------------

On a **Fedora 23** fresh system, Install following packages using  `sudo dnf install <PACKAGE>`

 * cmake, gcc-c++
 * qt4-devel
 * qjson-devel
 * taglib-devel, taglib-extras-devel
 * htmlcxx, htmlcxx-devel
 * phonon-devel [vlc-devel, libmpv-devel]
    
For Qt5 build:

 * qt5-qtbase-devel
 * qt5-qtx11extras-devel
 * qt5-linguist
 * phonon-qt5-devel

On a **Ubuntu 15.10** fresh system, Install following packages using  `sudo apt-get install <PACKAGE>`

 * build-essential
 * qt4-dev-tools 
 * libphonon-dev [libvlc-dev, mpv ]
 * libtag1-dev
 * libqt4-sql-sqlite 
 * libqjson-dev 
 * libhtmlcxx-dev

For Qt5 build:
 * cmake
 * qtbase5-dev
 * qtbase5-private-dev
 * qtbase5-dev-tools
 * qttools5-dev-tools
 * qttools5-dev
 * libqt5x11extras5-dev
 * libtag1-dev
 * libhtmlcxx-dev
 * libphonon4qt5-dev [libvlc-dev,libmpv-dev]


Compiling from source
----------------------------------------------------------------------------------------------

#### Building from command line

    mkdir build && cd build
    cmake ..
    make

#### Install

    make install (as root)


#### Phonon

By default, Yarock player use the phonon capabilities system provided by Qt. 

You have to install a phonon backend and check your Phonon setup. 

See Phonon backends : VLC, gstreamer, Xine, Mplayer... 

Under some linux distro (tested on Mageia) you need to change link to phonon plugin with 

      ln -s /usr/lib/kde4/plugins/phonon_backend /usr/lib/qt/plugins
      (OR) ln -s /usr/lib64/kde4/plugins/phonon_backend /usr/lib64/qt4/plugins
     
     
#### Alternate Audio engine

You can build yarock with alternate audio engine (vlc,phonon or mpv). Vlc is the default audio engine
since the 1.1.5 versio.

If you want enable/disable audio engine use the following cmake option :

      mkdir build && cd build
      cmake ..  -DENABLE_VLC=ON -DENABLE_MPV=ON -DENABLE_PHONON=ON
      make
      make install (as root)




Tips & Tricks
----------------------------------------------------------------------------------------------

#### Change systray icon

you can use your on icon for yarock in the systray by using the file 

    /home/your name/.config/yarock/systray_icon.png 


Legal Stuff
----------------------------------------------------------------------------------------------

Copyright (C) 2010-2016 Sébastien Amardeilh sebastien.amardeilh+yarock@gmail.com

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


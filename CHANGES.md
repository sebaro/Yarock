Yarock changes

#### 1.5.0  - PUBLIC RELEASE - april 2024
---------------------------------------------------------------------------------
  - New    : Ported to Qt6

#### 1.4.0  - PUBLIC RELEASE - march 2019
---------------------------------------------------------------------------------
  - BugFix : fix Appstream metadata legacy location and format #1813638
  - BugFix : fix widget bottom toolbar save/restore state
  - BugFix : fix debian Lintian spelling errors #1815382
  - BugFix : fix internal build rules for phonon engine #1745854
  - BugFix : improve mpris metatada for web stream
  - BugFix : fix icon installation under some environment #1747308

#### 1.3.1  - PUBLIC RELEASE - february 2018
---------------------------------------------------------------------------------
  - BugFix : Fix Qt4 build
  - BugFix : Fix player tool bar save/restore
  - BugFix : Fix playqueue track rating minor issue

#### 1.3.0  - PUBLIC RELEASE - december 2017
---------------------------------------------------------------------------------
  - New    : New favorite stream editor with extended option
  - New    : Improve favorites streams management
  - New    : Improve stream search (TuneIn, Dirble)
  - New    : Add bitrate info for radio stream view (TuneIn, Dirble)
  - New    : Add option to enable/disable playing history
  - New    : Better icon color consistency for light or dark system theme
  - New    : Save/restore bottom bar state (expanded/collapsed)
  - BugFix : Fix favorite stream loss
  - BugFix : Fix playcount update if history not enabled
  - BugFix : Fix auto rating calculation
  - BugFix : Fix cover size management for album edition
  - BugFix : Fix phonon engine crash condition on stopping player (with gstreamer backend)
  - BugFix : Fix mpv engine player crash
  - Changes: Improve playing item visibility
  - Changes: Restore previous playing cover for radio stream
  - Changes: Minor code update for deprecated Qt5
  - Changes: Minor ui changes & improvments


#### 1.2.0  - PUBLIC RELEASE - october 2017
---------------------------------------------------------------------------------
  - New    : Configurable custom cover size [#1627841]
  - New    : Redesign player tool bar with expanded tool bar mode
  - New    : Remove Echonest web service no longer available
  - New    : Add Spotify web service for artist image
  - Changes: Internet connection; prevent downloading artist image if no used [#1624536]
  - Changes: Change & fix many lyrics provider services
  - Changes: Many minors ui changes and improvments


#### 1.1.6  - PUBLIC RELEASE - october 2016
---------------------------------------------------------------------------------
  - New    : Add Opus audio file support [#1539942]
  - New    : Enable maximum volume level to 150 % (VLC/MPV engine)
  - New    : Add drag and drop from playqueue to playlist to add tracks to playlist
  - New    : Add option to enable/disable shorcuts [#1543410]
  - BugFix : Fix closing app with active systray icon [#1627840]
  - BugFix : Allow media keys working when current song info panel is displayed [#1624532]
  - BugFix : Fix wrong player stop notification [#1539912]
  - BugFix : Translations fix


#### 1.1.5  - PUBLIC RELEASE - december 2015
---------------------------------------------------------------------------------
  - New    : Default audio engine is VLC (phonon/MPV still available)
  - New    : Default Qt library is Qt5 (Qt4 still available)
  - New    : new advanced now playing widget
  - New    : new favorites web radio management
  - New    : new redesign webradio view layout
  - Change : many GUI changes and improvments
  - Option to reach collections from the left side pane [#1489281]
  - BugFix : missing icons and wrong context menu under gnome desktop
  - BugFix : fix album cover during complete database reconstruction [#1493948]
  - BugFix : segfaults on Fedora 23 when rating a track [#1529087]
  - BugFix : fix closing equalizer windows [#1492119]
  - BugFix : fix crash at startup when first playing track [#1527876]
  - BugFix : fix minor seek slider popup display issue
  - BugFix : fix global shortcut management with Qt5
  - BugFix : building issue [#1491736]
  - BugFix : building support taglib-1.10 (and newer) [#1522829]
  - BugFix : building support LIB_SUFFIX [#1522832]

#### 1.1.4  - PUBLIC RELEASE - august 2015
---------------------------------------------------------------------------------
  - New : improve genre tags browsing with new genre tag cloud
  - New : allow playing tracks from playlist editor
  - New : UI change with new now playing information widget
  - New : add track numbers in the playqueue [#1489222]
  - New : add user's volume restoration at startup [#1489224]
  - New : add symobolic link support for user collection [#1486708]
  - Change : simplified playlist editor
  - Change : activate mpris control by default
  - BugFix : fix ordering tracks when dragging album to playqueue [#1489222]
  - BugFix : yarock crashes when trying to load a radio station [#1486524]
  - BugFix : dysfunctional playlist system [#1489228]
  - BugFix : fix mpris command to raise application [#1487730]
  - BugFix : fix metadata update for phonon engine
  - BugFix : add scalable icon for better desktop integration (ubuntu unity)
  - BugFix : improve stream loader

#### 1.1.3  - PUBLIC RELEASE - august 2015
---------------------------------------------------------------------------------
  - New : new radionomy radio service (replace shoutcast)
  - New : add radio cover for all radio views
  - New : add symbolic link support for file system view
  - New : add MP4 audio files tag reading
  - New : add ape files import into playqueue
  - New : add browsing history support for stream link in radio views
  - BugFix : fix network redirection management
  - BugFix : fix volume slider for phonon engine
  - BugFix : fix "lyrics.com" radio provider
  - Bugfix : fix image catching for stream image
  - BugFix : improve management of no sqlite driver error
  - BugFix : improve management of audio engine initialization failure
  - BugFix : fix playqueue ordering tracks (from filsystem without track number)
  - BugFix : fix playqueue stop after playing track
  - BugFix : fix now playing widget update
  - BugFix : fix qt5 build on some system ( bad include of qt5X11Extras library)
  - BugFix : fix qt5 folder icon for filesystem view
  - Change : remove confusing parameter for multi-collection option
  - Change : improve status widgets
  - Change : improve translation lookup path with cmake user definition
  - Change : many widgets/ui minors update
  - Change : update translations
  - Build info : new depandencie to htmlcxx library

#### 1.1.2  - PUBLIC RELEASE - may 2015
---------------------------------------------------------------------------------
  - New    : add alternate mpv audio engine (mplayer based)
  - New    : add plugin system for audio back-ends, in order to load at run time desire audio engine (vlc, mpv, phonon)
  - New    : add "search and play" option for quick playing tracks according to your search result
  - New    : add add equalizer support for vlc audio engine (vlclib > 2.2)
  - New    : add popup on player seek bar
  - BugFix : vlc audio engine -> fix volume management
  - BugFix : vlc audio engine -> fix backwards seeking
  - BugFix : fix minor issue when dragging tracks to the bottom of playqueue
  - BugFix : fix crash when playing stream added to user playlist
  - Change : dirble radio use api v2
  - Change : improve media key support
  - Change : improve Qt5 building support
  - Change : improve Qt5 third party source building
  - Change : various user interface fix and changes
  - Change : filesystem view, add sorting of files when dragging directory to playqueue
  - Change : no collection update if directory is not available (remote collection)

#### 1.0.1  - PUBLIC RELEASE - mars 2015
---------------------------------------------------------------------------------
  - BugFix : database issue
  - BugFix : fix song lenght update (#1393027)
  - BugFix : Jumbled Text in Settings Area (#1394508)
  - BugFix : minor ui presentation for filesystem view
  - BugFix : image for stream management
  - Change : add ascending order for filesystem view
  - Change : file dialog open user music folder by default if exists
  - Change : file dialog in settings -> open saved directory
  - Change : easiest first time user startup
  - Change : add saving position in browser view
  - Change : update and add translation

#### 1.0.0  - PUBLIC RELEASE - november 2014
---------------------------------------------------------------------------------
  - New    : new clean and elegant design, new icon set, new app icon
  - New    : add support for artists images (new view, download artists images)
  - New    : save rating to file
  - New    : add advanced library search
  - New    : add album cover for playqueue widget
  - New    : add stars rating for playqueue widget
  - New    : add multiple tracks edition
  - Change : rewrite now playing widget
  - Change : improve browser view multiple selection with SHIFT key
  - Change : improve artist/album/track tags edition
  - Change : support more scalable ui items
  - Change : file dialog various improvements
  - BugFix : improve file system view performance
  - BugFix : fix "open with" yarock from KDE menu
  - BugFix : fix playqueue restoration at startup
  - BugFix : fix translation installation path
  - BugFix : rating mp3 tag reading
  - BugFix : minor fix for genre sorting view

#### 0.9.67 - PUBLIC RELEASE - may 2014
---------------------------------------------------------------------------------
  - Change : improve database incremental update
  - Change : improve playqueue content save/restore performance
  - Change : improve playlist editor
  - Change : new dirble api implementation
  - BugFix : phonon vlc crash on stream network error (with phonon media queue empty)
  - BugFix : fix crash on app closing
  - BugFix : fix playing update statu

#### 0.9.66 - PUBLIC RELEASE - april 2014
---------------------------------------------------------------------------------
  - New : add contextual playing information for stream
  - New : improve contextual playing information
      - add musicbrainz support
      - rewrite echonest provider (remove echonest library dependancy)
      - change discogs provider (new policy for cover downloading)
      - add similars artist images fetching
  - New : internet radio services:
      - remove Icecast directory (no conveniant api to sort & search...)
      - add Dirble radio service
      - add stream search capability for radio services (tunin,shoutcast,dirble)
  - New : improve contextual menu for artist/album/track
  - New : add settings option to restore or not playqueue at startup
  - New : add settings option to chose/sort lyrics provider for contextual lyrics
  - New : improve navigation between artist/album/track
  - BugFix : minor fix for filesystem view
  - BugFix : fix text antialiasing issue
  - BugFix : crash with vlc network access error on stream loading

#### 0.9.65 - PUBLIC RELEASE - December 2013
---------------------------------------------------------------------------------
  - New : add file system browser view
  - New : add tunein stream image download support
  - New : add option to remove or not duplicate tracks in playqueue
  - New : improve discography part in context view
  - New : add loading status when building database
  - BugFix : fix last fm now playing method
  - BugFix : fix crash on browser favorite view
  - BugFix : fix crash on menu  bar clicking
  - BugFix : fix mpris2 support
  - BugFix : fix smart playlist edition (losing configuration)
  - BugFix : fix little freeze when entering view by genre
  - BugFix : fix browser item right click in case of single item selection
  - BugFix : fix file dialog issue
  - BugFix : fix regression on tracks move inside playqueue
  - Change : improve settings view
  - Change : lot of code rewrite/refactoring
  - Building : change to facilitate fedora packaging (thanks to fedora community help)
     => CMakeLists : add option to use system lib for src3party
     => CMakeLists : change default installation path

#### 0.9.64 - PUBLIC RELEASE - November 2013
---------------------------------------------------------------------------------
  - New : add support for multiset album (grouping or not)
  - New : add support to drag from playqueue to file browser
          (in order to copie music file to folder)
  - New : add standalone vlc audio engine (without phonon layer)
  - New : add option to choose color scheme
  - BugFix : fix multithreading issue when populating playqueue
  - BugFix : fix cover searcher threadmanager
  - BugFix : fix crash on media item context menu access
  - BugFix : fix lyricsmania extraction rule
  - BugFix : fix echonest artist image download
  - BugFix : fix context view item layout update
  - BugFix : fix seek bar and time label widget spacing
  - Change : reorganise code structure
  - Change : rewrite file dialogs
  - Change : replace navigator bar by a menu icon bar
  - Change : lot of minor ui improvement

#### 0.9.63 - PUBLIC RELEASE - June 2013
---------------------------------------------------------------------------------
  - New : add Tunein radio service
  - New : add cover art reading from ogg/flac file
  - Change : code refactoring for radio stream services
  - BugFix : playlist editor crash
  - BugFix : sorting on track duration
  - BugFix : playqueue move item
  - BugFix : fix playqueue keyboard enter action

#### 0.9.62 - PUBLIC RELEASE - May 2013
---------------------------------------------------------------------------------
  - New : add playqueue filter and sort toolbar
  - New : add support for multiple item selection
  - New : add support for mpris2 interface
  - New : add completer for music explorer search field
  - New : add keyboard shortcut to jump to current playing track
  - New : add option to disable volume notification
  - Change : change build system to cmake
  - Change : change application name from YaRock to "yarock" without upper case
        - Note: you can recover your old install configuration dir by copying
            $   mv <home folder>/.config/YaRock to <home folder>/.config/yarock)
  - Change : minor rewrite code for dbus notification
  - Change : single click on shortcut widget in settings page to trigger shortcut dialogs
  - BugFix : restore drag and drop track from history view
  - BugFix : crash on playqueue restoration
  - BugFix : random playing from playqueue

#### 0.9.61 - PUBLIC RELEASE - Mars 2013
---------------------------------------------------------------------------------
  - New : improve system color sheme support (bug ##928789)
  - New : add rating edition in browser (bug #1102794)
  - New : add different rating color in case of auto rating vs user rating
  - BugFix : CPU too high when playing  (bug #1119970)
  - BugFix : item context menu (bug #1102793)
  - BugFix : crash on file path broken with vlc backend (ok with gstreamer)
  - BugFix : automatic rating for artist wrong computation
  - BugFix : fix playing button action when no track is selected (play first track in playqueue)
  - BugFix : fix file include for building on mandriva
  - Change : add Ctr+F shortcut to focus on search line edit widget
  - Change : improve statusWidget design
  - Change : improve playqueue view update
  - Change : minor settings pages UI update
  - Change : change playcount representation
  - Change : rewrite core data management
  - Change : rewrite pixmap caching management
  - Change : delete provided custom font (use system font)
  - Change : improve history with pretty category name
  - Change : remove boost library dependency

#### 0.0.60 - PUBLIC RELEASE - January 2013
---------------------------------------------------------------------------------
  - New : add jump to now playing artis/album/track
  - New : add rating edition in now playing widget
  - New : add filtering support for dashboard view
  - New : add context view information (artist discography)
  - New : add new settings dialogs page instead of dialog window
  - New : add new dialog to add internet stream
  - BugFix : fix artist database edition
  - BugFix : add a limit to "never played tracks" smart playlist to avoid UI freeze
  - BugFix : fix discogs album cover issue (api key + other minor bug)
  - BugFix : fix playlist parser absolute path detection
  - BugFix : fix smart playlist default sorting order
  - BugFix : fix player playing transition
  - Change : improve remote playlist download
  - Change : rewrite history management and history view
  - Change : rewrite context view management and widgets
  - Change : rewrite playlist editor (use threadmanager)
  - Change : rewrite track info providers (lastfm, discogs, echonest)
  - Change : rewrite web link widget for context view
  - Change : update libechonest library
  - Change : update lyrics info provider data
  - Change : update icons and minor ui changes
  - Change : update translations

#### 0.0.59 - PUBLIC RELEASE - October 2012
---------------------------------------------------------------------------------
  - 0.0.59 - New : add parsing of disk number tags (bug:#1079803)
  - 0.0.59 - BugFix : fix building issue for standard C++ function (std::random_shuffle)
  - 0.0.59 - Bugfix : fix playqueue information for extended mode when file access failed
  - 0.0.59 - Change : Code refactoring for database scanner

#### 0.0.58 - PUBLIC RELEASE - August 2012
---------------------------------------------------------------------------------
  - 0.0.58 - New : add option du disable menu on mouse over (Launchpad bug #1041611)
  - 0.0.58 - New : add a basic stream list editor (for custom user stream file)
  - 0.0.58 - New : add ctrl/click on item to enqueue in playqueue
  - 0.0.58 - BugFix : wrong context view update
  - 0.0.58 - BugFix : wrong .pro file for translation isntallation
  - 0.0.58 - BugFix : wrong playlist view update when adding playlist with playlist editor
  - 0.0.58 - BugFix : favorite radio stream view (bug in 0.0.57)
  - 0.0.58 - Change : minor user interface update
  - 0.0.58 - Change : minor improvment for playlist editor
  - 0.0.58 - Change : update translations
  - 0.0.58 - Change : force detection of header "EXTM3U" for M3u file
  - 0.0.58 - Change : update update right click menu for stream item

#### 0.0.57 - PUBLIC RELEASE - August 2012
---------------------------------------------------------------------------------
  - 0.0.57 - New : enhanced right click menu for artist/album items
  - 0.0.57 - New : improve artiste name display (artist view)
  - 0.0.57 - New : add messages in view in case of empty result in radio streammodel
  - 0.0.57 - New : add support for custom sysray icon
  - 0.0.57 - New : add option to start with hidden windows if systray is activated
  - 0.0.57 - BugFix : wrong album metada update
  - 0.0.57 - BugFix : fix insensitive sorting for artist view
  - 0.0.57 - BugFix : track queuing with phonon-vlc
  - 0.0.57 - BugFix : artist rating display
  - 0.0.57 - Bugfix : lack in dbus now playing notification under some system (mandriva + phonon-gstreamer)
  - 0.0.57 - Bugfix wrong playing item update (in case of symlink)
  - 0.0.57 - Bugfix volume menu location (on some distro like mandriva)
  - 0.0.57 - Bugfix crash when enabling equalizer
  - 0.0.57 - Change: Rewrite equalizer settings management
  - 0.0.57 - Change sorting for album by year view
  - 0.0.57 - Change : update qxt provided library to 0.6.2
  - 0.0.57 - Change : rewrite contextual right click menu
  - 0.0.57 - Change : fix icecast "Unspecified name" entries
  - 0.0.57 - Change : minor graphic update and layout improvments
  - 0.0.57 - Change : code clean up & refactoring (global actions)

#### 0.0.56 - PUBLIC RELEASE - May 2012
---------------------------------------------------------------------------------
  - 0.0.56 - New : add option to stop after specific track in playqueue
  - 0.0.56 - New : add option to resume playback at startup
  - 0.0.56 - New : add option to stop or not playing on playqueue clear action
  - 0.0.56 - New : add tag reading for "ALBUM-ARTIST" metadata in case of flac/ogg file (launchpad Bug #969987)
  - 0.0.56 - BugFix : lyrics fetching broken for release 0.0.55
  - 0.0.56 - BugFix : collection browser alignement is not correct (launchpad Bug #965146)
  - 0.0.56 - BugFix : fix crash at player stopped event on MAGEAI linux (launchpad Bug #969990)
  - 0.0.56 - BugFix : fix unstranslated string (rewrite SmartPlaylist part) (launchpad Bug #969988)

#### 0.0.55 - PUBLIC RELEASE - March 2012
---------------------------------------------------------------------------------
  - 0.0.55 - New : add track/album/artist rating support
  - 0.0.55 - New : smart playlist generator
  - 0.0.55 - New : add playlist editor
  - 0.0.55 - New : add advance media search engine
  - 0.0.55 - New : add cover search from discogs
  - 0.0.55 - Change : delete file browser view
  - 0.0.55 - Change : rewrite/change diashboard view
  - 0.0.55 - Change : improve playlist file database handling
  - 0.0.55 - Change : improve history database handling
  - 0.0.55 - BugFix : track time tracking invalid (lauchpad bug : #887505)
  - 0.0.55 - BugFix : add artist name to albums views  (lauchpad bug : #927557)
  - 0.0.55 - BugFix : (partially ) add option to enqueue media item into playqueue (lauchpad bug : #927747)
  - 0.0.55 - BugFix : bugfix color sheme for dark theme (lauchpad bug : #928789)
  - 0.0.55 - BugFix : improve loading collection time (lauchpad bug : #927563)
  - 0.0.55 - BugFix : crash when database is rebuilding and next mediaitem is requested (data model cleared)
  - 0.0.55 - BugFix : album update info not working
  - 0.0.55 - Minor Change : default font, gui minor changes

#### 0.0.54 - PUBLIC RELEASE - January 2012
---------------------------------------------------------------------------------
  - 0.0.54 - New : support replaygain tag (volume normalization)
  - 0.0.54 - New : new layout for playlist
  - 0.0.54 - New : add hide/show now playing widget option
  - 0.0.54 - New : add basic key support (delete/play) for playlist view
  - 0.0.54 - New : add multiple selection in playlist views
  - 0.0.54 - New : add delete multiple selected files from playlist
  - 0.0.54 - New : add internal track move in playlist
  - 0.0.54 - Update Czech translation (thanks to pavel)
  - 0.0.54 - BugFix : crash when adding manual url to playlist
  - 0.0.54 - BugFix : crask on right click on the playlist
  - 0.0.54 - BugFix : total time track not updated all the time
  - 0.0.54 - GUI : change browser color scheme
  - 0.0.54 - GUI : status widget rework and replace statusbar
  - 0.0.54 - GUI : current playing track put in tool bar (instead of old status bar)
  - 0.0.54 - Internal code rewrite (engineplayer, merge with inforesolver code)

#### 0.0.53 - PUBLIC RELEASE - December 2011
---------------------------------------------------------------------------------
  - 0.0.53 - New : add remote control through command line
  - 0.0.53 - New : add remote control through mpris1 interface
  - 0.0.53 - New : add option to hide status bar and new status widget instead
  - 0.0.53 - New : add option to perform a simple database update
  - 0.0.53 - New : option active(de-activate) notification/mpris/systray without restarting
  - 0.0.53 - New : add m4a/aac format support (depending on phonon backend to be played)
  - 0.0.53 - New : add volume Up/Down/Mute media key support
  - 0.0.53 - Changes : lastfm authentication rework
  - 0.0.53 - Changes : improve status message for database loading or scanning
  - 0.0.53 - Changes : delete useless Menu Bar
  - 0.0.53 - Changes : notification through  dbus
  - 0.0.53 - Changes : improve looking under different theme
  - 0.0.53 - BugFix : shurtcut media key update
  - 0.0.53 - BugFix : volume menu location
  - 0.0.53 - BugFix : compact widget with bad resize
  - 0.0.53 - Improve internal settings management system
  - 0.0.53 - Internal Code rewriting (engine player, user dialog)
  - 0.0.53 - Internal Code rewriting (threadmanager, search cover thread)
  - 0.0.52 - PUBLIC RELEASE - October 2011
  - 0.0.52 - New : save and restore settings for Repeat & Shuffle mode
  - 0.0.52 - BugFix : wrong database user modification
  - 0.0.52 - BugFix : scrobbler lasf fm wrong init
  - 0.0.52 - BugFix : missing icons & delete unused font

#### 0.0.51 - PUBLIC RELEASE - July 2011
---------------------------------------------------------------------------------
  - 0.0.51 - New : add dashboard view with most played chart
  - 0.0.51 - New : add support for album & artists playcount (database change !)
  - 0.0.51 - New : add dynamic layout for context view
  - 0.0.51 - New : add information for track view
  - 0.0.51 - Changes : improve Database version detection & rebuild
  - 0.0.51 - BugFix : crash on user Database change when browser is populating
  - 0.0.51 - BugFix : fix smart playlist creation
  - 0.0.51 - BugFix : fix first start widget size
  - 0.0.50 - New : add now playing desktop notification
  - 0.0.50 - New : dynamic layout resize for the collection browser
  - 0.0.50 - New : add option to remove duplicate stream
  - 0.0.50 - New : add tooltip for stream with url
  - 0.0.50 - New : improve failed stream detection
  - 0.0.50 - New : add search support for prev/next history navigation
  - 0.0.50 - New : add menu entry for file browsing depending on mounted devices (etc/mtab)
  - 0.0.50 - New : add Romanian translation (thanks to Sebastian)
  - 0.0.50 - GUI : add custom font (Segoe UI light) for Library, update typography
  - 0.0.50 - Changes : file browser view is replaced by a single icon view page instead of treeview
  - 0.0.50 - Changes : improve phonon backend behaviour management and failure deletection (moslty tested with gstreamer and vlc)
  - 0.0.50 - Changes : add a fast navigator bar instead of the usual breadcrumb trail
  - 0.0.50 - Changes : delete Up/down navigation arrow from ToolBar (now up/down navigation is done by clicking on scrollbar area)
  - 0.0.50 - BugFix : workaround for file access issue
  - 0.0.50 - BugFix : seekslider not working properly dependings on backends
  - 0.0.50 - BugFix : empty title for stream read from remote pls playlist
  - 0.0.50 - BugFix : workaround for failed stream detection stream
  - 0.0.50 - BugFix : crash on navigation prev/next and navigator bar interaction

#### 0.0.49 - PUBLIC RELEASE - May 2011
---------------------------------------------------------------------------------
  - 0.0.49 - New : smart predefined playlists
  - 0.0.49 - Changes statusbar message to display database update progress percent
  - 0.0.49 - Internal Code rewriting (database builder merge)
  - 0.0.48 - New (Hot) lastFM scrobbling support, experimental :)
  - 0.0.48 - New : playback option (repeat, shuffle)
  - 0.0.48 - New : basic keyboard shortcut support
  - 0.0.48 - New : option to display album in a simple grid
  - 0.0.48 - Changes : simplify statusbar message
  - 0.0.48 - Changes : update Czech translation, thanks to Pavel ;)
  - 0.0.48 - Changes : minor UI update for better lookin with ubuntu/gnome default theme
  - 0.0.48 - BugFix : button in context view without text or icon

#### 0.0.47 - PUBLIC RELEASE - April 2011
---------------------------------------------------------------------------------
  - 0.0.47 - Add track playcount support into database
  - 0.0.46 - New : use lib echonest to provide artist bio, image & similar
  - 0.0.46 - New : add providers for lyrics download
  - 0.0.46 - New : lyrics can be saved to & retrieve from local file
  - 0.0.46 - New : add button to context view with a link to last Fm artist page
  - 0.0.46 - GUI : minor change for statusbar messages
  - 0.0.45 - New history view (last played track & stream)
  - 0.0.45 - New add a view to sort album by date
  - 0.0.45 - Changes : disable playqueue track internal move :(
  - 0.0.45 - Changes : improve algorithm to sort album by genre
  - 0.0.45 - Changes : improve Drag & Drop from collection to playqueue
  - 0.0.45 - Changes : code refactoring for internal data management
  - 0.0.45 - Changes : code refactoring & optimisation (use QPixmapCache)
  - 0.0.45 - Changes : graphicviewwidget.cpp (class inhérit from QGraphicsView )
  - 0.0.45 - Changes : fix selection into menu view (selection follow the good page)
  - 0.0.45 - Changes : display history view with one category for one day
  - 0.0.45 - Changes : minor code rewrite for contexte view
  - 0.0.45 - BugFix  : lot of minor bug fixes & minor regression
  - 0.0.45 - BugFix  : major fix shoutcast file download
  - 0.0.45 - BugFix  : fix up/down navigation for radio browser
  - 0.0.44 - New multiple database support
  - 0.0.44 - New add InfoResolver class to centralize now playing info & metadata
  - 0.0.44 - GUI add right clic menu on main menu
  - 0.0.44 - GUI change time tracking slider
  - 0.0.44 - Change internals Playlist support (no file anymore, only saved into Db)
  - 0.0.44 - BugFix wrong database update on user rename album action
  - 0.0.44 - BugFix view update on playlist status item chang
  - 0.0.43 - New add visual icon for broken media item or stream
  - 0.0.43 - New audio equalizer
  - 0.0.43 - New simple application minimal mode widget
  - 0.0.43 - New copy of icecast & shoutcast file during first run of application
  - 0.0.43 - New filer browser not loaded at startup (only on demand)
  - 0.0.43 - GUI simplify & clean GUI, icon, hidden menuBar
  - 0.0.43 - GUI change playlist image into browser
  - 0.0.43 - GUI add splitter collapser
  - 0.0.43 - GUI add visual item selection
  - 0.0.43 - GUI change icon set, rewrite menu
  - 0.0.43 - GUI change custom splitter handle
  - 0.0.43 - BugFix breadcrumb apppearance under GNOME desktop
  - 0.0.43 - BugFix When stream list is reloaded, activated item is false due to error defore reloading
  - 0.0.43 - BugFix Single application use correction
  - 0.0.43 - BugFix delete a media from playlist crash

#### 0.0.42 - PUBLIC RELEASE - February 2011
---------------------------------------------------------------------------------
  - 0.0.42 - New Context View (artist, album lyrics info)
  - 0.0.42 - New BreadCrumb with navigation capabilities
  - 0.0.42 - Change clean up code
  - 0.0.42 - Change Code refactoring (Menu, GlobalAction view switching)
  - 0.0.42 - Change Grouping view collectionwidget/radiowidget/contextwidget
  - 0.0.42 - Change icon for stream and empty cover change
  - 0.0.42 - BugFix multiple access to database crash (when playlist is beieg populated)
  - 0.0.42 - BugFix breadcrumb restore initialization
  - 0.0.42 - BugFix drag drop from fileBrowser to playlist (local file)
  - 0.0.42 - BugFix graphic status for playing item not update

#### 0.0.41 - PUBLIC RELEASE - december 2010
---------------------------------------------------------------------------------
  - 0.0.41 - New ToolBar with Prev/Next browsing, read only BreadCrumb
  - 0.0.41 - Change (MAJOR) music collection redesign (less memory usage, better performance)
  - 0.0.41 - New ShoutCast radio browser
  - 0.0.40 - BugFix streammodel missing active stream initialization
  - 0.0.40 - New favorite radio item (stored into xspf file)
  - 0.0.40 - New add support for remote playlist to be played diectly from radio browser
  - 0.0.39 - New Menu with tree structure and radio file
  - 0.0.39 - New add radio setting for adding/removing specific radio file
  - 0.0.39 - New radio browser for user radio file
  - 0.0.39 - New IceCast radio browser
  - 0.0.39 - Change core graphic painting method improvement
  - 0.0.39 - Change core thread management improvement
  - 0.0.38 - New internal Database v8 (internal & user playlist file)
  - 0.0.37 - New remote Url and remote playlist support
  - 0.0.37 - New use of QtSingle application module
  - 0.0.37 - New playlist detail view (with up/down and search fonction)
  - 0.0.37 - BugFix minor playlist view line offset correction
  - 0.0.37 - Changes playlist populator (for remote playlist support)
  - 0.0.36 - New Translation support + translations French, Czech, Spanish
  - 0.0.36 - BugFix file filtering into dialog when saving playlist

#### 0.0.35 - PUBLIC RELEASE - november 2010
---------------------------------------------------------------------------------
  - 0.0.35 - Basic Playlist File Browser
  - 0.0.35 - Code refactoring
  - 0.0.34 - New Setting option to activate/de-activate systray
  - 0.0.34 - User Settings rewritten
  - 0.0.34 - New Menu panel on Home button
  - 0.0.34 - Simple systray
  - 0.0.33 - Change Hard coded lower case for artist/album deleted (db is now case sensitive)
  - 0.0.33 - Change Now playing cover art display simplification
  - 0.0.33 - BugFix Now playing database search for title playing
  - 0.0.33 - Change default image for empty cover art pre-sized to useful value
  - 0.0.33 - New support load/save playlist from/to file (m3u, pls, xspf)
  - 0.0.32 - New automatic fetch for all the collection
  - 0.0.32 - Change Status bar Progress action
  - 0.0.32 - BugFix Status bar database search for title playing
  - 0.0.32 - New option to get cover from file directory
  - 0.0.31 - New main home icon (with collection, filer, ...)
  - 0.0.31 - Code refactoring
  - 0.0.30 - Code refactoring
  - 0.0.30 - Change, cover fetch improvment
  - 0.0.29 - BugFix Go Up navigation
  - 0.0.29 - BugFix play track from collection

#### 0.0.28 : PUBLIC RELEASE - october 2010
---------------------------------------------------------------------------------
  - 0.0.27 : BugFix Genre database modification
  - 0.0.26 : New up/down navigation into collection
  - 0.0.25 : BugFix collection favorite view
  - 0.0.24 : New collection favorite view
  - 0.0.23 : BugFix Genre View
  - 0.0.22 : New genre view
  - 0.0.21 : New search cover from last FM
  - 0.0.21 : New statubar with info (playlist, collection, now playing)
  - 0.0.21 : Change playlist and now playing grouped
  - 0.0.21 : Change search into collection (new filtering method)
  - 0.0.21 : code clean up, unused code

#### 0.0.20 : PUBLIC RELEASE - september 2010
---------------------------------------------------------------------------------
  - 0.0.20 : Bug fix Major playing track from track view
  - 0.0.19 : New architecture design between database and collection viewer (model added)
  - 0.0.18 : New simple file browser view
  - 0.0.18 : Bug fix severals minor bug
  - 0.0.17 : Change minor changes/improvements
  - 0.0.17 : Bug fix wrong database modification fix

#### 0.0.16 : PUBLIC RELEASE - august 2010
---------------------------------------------------------------------------------
  - 0.0.16 : New search into collection feature
  - 0.0.16 : Bug Fix severals minor bug fix

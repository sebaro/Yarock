/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2015 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

#include "mainwindow.h"

// player engine
#include "core/player/engine.h"

// widget
#include "playqueue/playlistview.h"
#include "playqueue/playlistwidget.h"
#include "widgets/main/centralwidget.h"
#include "widgets/minimalwidget.h"
#include "widgets/statuswidget.h"
#include "widgets/equalizer/equalizer_dialog.h"

#include "widgets/editors/editor_playlist.h"


// data model
#include "models/local/local_track_model.h"
#include "models/local/local_playlist_model.h"
#include "models/stream/stream_model.h"
#include "models/local/histo_model.h"

#include "playqueue/playqueue_model.h"
#include "playqueue/task_manager.h"


// views
#include "views/browser_view.h"
#include "views/local/local_scene.h"
#include "views/stream/stream_scene.h"
#include "views/context/context_scene.h"
#include "views/settings/settings_scene.h"

// core
#include "infosystem/info_system.h"
#include "infosystem/tasks/tagsearch.h"

#include "core/history/histomanager.h"
#include "core/database/database.h"
#include "playqueue/virtual_playqueue.h"

#include "online/lastfm.h"

#include "threadmanager.h"
#include "widgets/audiocontrols.h"
#include "commandlineoptions.h"
#include "global_shortcuts.h"
#include "settings.h"

#include "networkaccess.h"
#include "covercache.h"

#include "global_actions.h"
#include "constants.h"
#include "utilities.h"
#include "debug.h"

// Dbus & remote
#include "dbus/dbusnotification.h"
#include "dbus/mpris_manager.h"


// dialog
#include "dialogs/dboperationdialog.h"
#include "dialogs/addstreamdialog.h"
#include "dialogs/filedialog.h"
#include "dialogs/first_time_dialog.h"


MainWindow* MainWindow::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class MainWindow                                                          *
*                                                                              *
********************************************************************************
*/
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    INSTANCE       = this;
    is_first_start = true;

    //Debug::debug() << "line :"<< __LINE__ <<  " MainWindow -> start";
    setObjectName(QString::fromUtf8("yarock player"));
    setWindowTitle(QString::fromUtf8("yarock player"));

    QApplication::setWindowIcon( QIcon(":/icon/yarock_64x64.png") );

    // Size Policy
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
    setSizePolicy(sizePolicy);

    // Menu Policy
    this->setContextMenuPolicy (Qt::NoContextMenu);

    
    //! ###############   Settings   ########################################
    new YarockSettings();
    SETTINGS()->readSettings();

    //! ###############   Actions    ########################################
    new GlobalActions();
    createActions();

    //! ############### init Player #########################################
    new Engine();
    _player = Engine::instance();

    //! Info system
    InfoSystem::instance();

    //! ############### init collection database ############################
    Debug::debug() << "Mainwindow -> init database";
    new Database();

    //! ############### global instance #####################################
    new RatingPainter();
    new CoverCache();
    
    //! ############### init playqueue part  ################################
    m_playqueue = new Playqueue();
        
    _playlistView    = new PlaylistView(this, m_playqueue);
    _playlistwidget  = new PlaylistWidget(this,_playlistView, m_playqueue);

    //! ############### init data model  ####################################
    m_localTrackModel     = new LocalTrackModel(this);
    m_localPlaylistModel  = new LocalPlaylistModel(this);
    m_histoModel          = new HistoModel(this);
    m_streamModel         = new StreamModel(this);

    //! ############### Thread management ###################################
    m_thread_manager = new ThreadManager();
    
    //! ############### End Gui Stuff     ###################################
    _centralWidget = new CentralWidget(this,_playlistwidget);
    this->setCentralWidget(_centralWidget);

    //! ############### init views  #########################################
    //Debug::debug() << "MainWindow -> creation BrowserView";
    m_virtual_queue       = new VirtualPlayqueue(this);
    m_browserView         = new BrowserView(this);
    _centralWidget->setBrowser(m_browserView); 
    
    //! ############### Status Widget     ###################################
    _statusWidget = new StatusWidget(_centralWidget);
    _statusWidget->hide();
    
    //! Shortcuts & Signals
    //Debug::debug() << "MainWindow -> creation global shortcut";
    m_globalShortcuts = new GlobalShortcuts (this);

    //! ###############     Connection    ###################################
    //Debug::debug() << "MainWindow -> connectSlots";
    connectSlots();

    //! ############### Scrobbler ###########################################
    // le scrobbler doit être initialisé avant le fisrtStartDialog
    //Debug::debug() << "MainWindow -> Scrobbler";
    LastFmService::instance()->init();

    //! ############### start database ######################################
    //Debug::debug() << "MainWindow -> start timer for database update";
    /* use Timer to let Maindow show before all database stuff is done */
    QTimer::singleShot(4, this, SLOT(slot_database_start()));

    //! ############### Systray Icon ########################################
    m_canClose = false;
    m_trayIcon = 0;
    reloadSystraySettings();

    //! ############### Minimal window ######################################
    m_minimalwidget = NULL;

    //! ############### History manager #####################################
    //Debug::debug() << "MainWindow -> m_histoManager";
    m_histoManager = new HistoManager();

    //! ############### DBUS & MPRIS ########################################
    //Debug::debug() << "MainWindow -> Dbus & Mpris";
    m_dbus_notifier   = new DbusNotification(this);
    m_mpris_manager   = new MprisManager(this);

    //! ############### Restore Playing   ###################################
    if(SETTINGS()->_restartPlayingAtStartup)
      restorePlayingTrack();

    //! ############### Restore windows geometry ############################
    Debug::debug() << "MainWindow -> restore geometry";
    if( !SETTINGS()->_windowsGeometry.isEmpty() )
      restoreGeometry(SETTINGS()->_windowsGeometry);
    else
      resize(1200, 800);

    QTimer::singleShot(50, _centralWidget, SLOT(restoreState()));
    
    /* use time to have good Browser width before populating the scene */
    QTimer::singleShot(100, m_browserView, SLOT(restore_view()));
    
    //! ############### Hide window       ###################################
    if(SETTINGS()->_hideAtStartup)
      this->showMinimized();    
    
    
// TEST    
//     Debug::debug() << "####### START TEST ##############";
//     QTimer::singleShot(4000, this, SLOT(slot_start_test()));
// END TEST
}


void MainWindow::slot_start_test()
{
//     INFO::InfoRequestData request;
//     INFO::InfoStringHash hash;
//     hash["artist"] = "Clapton";
//     hash["album"]  = "Slowhand";
//     hash["title"]  = "";
//     
//     request = INFO::InfoRequestData(INFO::InfoAlbumSongs, hash);
//     InfoSystem::instance()->getInfo( request );
}


//! --------- ~MainWindow ------------------------------------------------------
MainWindow::~MainWindow()
{
    Debug::debug() << "Mainwindow -> EXIT ";
    /* ---- Save playqueue content to database ---- */
    if(SETTINGS()->_restorePlayqueue)
      m_playqueue->manager()->savePlayqueueSession();

    /* ---- Save current track playing ---- */
    if(SETTINGS()->_restartPlayingAtStartup)
      savePlayingTrack();

    /* ---- Save setting ---- */
    _player->stop();
    
    /* ---- Save setting ---- */
    _centralWidget->saveState();
    m_browserView->save_view();
    SETTINGS()->_windowsGeometry = this->saveGeometry();
    SETTINGS()->_windowsState    = this->saveState();
    SETTINGS()->_playqueueShowCover  = ACTIONS()->value(PLAYQUEUE_OPTION_SHOW_COVER)->isChecked();
    SETTINGS()->_playqueueShowRating = ACTIONS()->value(PLAYQUEUE_OPTION_SHOW_RATING)->isChecked();
    SETTINGS()->_playqueueDuplicate  = !ACTIONS()->value(PLAYQUEUE_REMOVE_DUPLICATE)->isChecked();

    SETTINGS()->writeSettings();
    
    Database::instance()->settings_save();

    /* ---- delete object ---- */
    Debug::debug() << "Mainwindow -> EXIT delete object";
    QPixmapCache::clear();
    delete LastFmService::instance();
    delete m_dbus_notifier;
    delete m_mpris_manager;
    delete m_localTrackModel;
    delete m_localPlaylistModel;
    delete _player;

    /* ---- stop main task ---- */
    m_thread_manager->stopThread();    
    delete m_thread_manager;

    /* ---- stop playqueue task ---- */
    /* !!! wait for playqueue task to be done */
    delete m_playqueue->manager();

    /* ---- database close ---- */
    Database::instance()->close();
    Debug::debug() << "Mainwindow -> EXIT Bye Bye";
}

//! --------- reloadSystraySettings --------------------------------------------
void MainWindow::reloadSystraySettings()
{
    // systray de-activated
    if(!SETTINGS()->_useTrayIcon)
    {
      if(m_trayIcon != 0) {
          disconnect(m_trayIcon, 0,this, 0);
          delete m_trayIcon;
          m_trayIcon = 0;
      }
    }
    // systray activated
    else
    {
      if(!m_trayIcon) {
        QMenu *trayIconMenu = new QMenu(this);
        //QAction *focus_action = new QAction(tr("set focus"),this);
        trayIconMenu->addAction(ACTIONS()->value(ENGINE_PLAY_PREV));
        trayIconMenu->addAction(ACTIONS()->value(ENGINE_PLAY));
        trayIconMenu->addAction(ACTIONS()->value(ENGINE_STOP));
        trayIconMenu->addAction(ACTIONS()->value(ENGINE_PLAY_NEXT));
        trayIconMenu->addSeparator();
        trayIconMenu->addAction(ACTIONS()->value(PLAYING_TRACK_LOVE));
        trayIconMenu->addSeparator();
        trayIconMenu->addAction(ACTIONS()->value(APP_QUIT));

        m_trayIcon = new QSystemTrayIcon(this);

        m_trayIcon->setIcon( QApplication::windowIcon() );
        m_trayIcon->setToolTip("Yarock");
        m_trayIcon->setContextMenu(trayIconMenu);
        m_trayIcon->show();

        connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(slot_systray_clicked(QSystemTrayIcon::ActivationReason)));
        connect(m_trayIcon, SIGNAL(messageClicked()), this, SLOT(setFocus()));
      }
    }
}


//! --------- createActions ----------------------------------------------------
void MainWindow::createActions()
{
    /* yarock global actions */
    ACTIONS()->insert(APP_QUIT, new QAction(QIcon::fromTheme("application-exit"),tr("&Quit"), this));
    ACTIONS()->insert(APP_SHOW_YAROCK_ABOUT, new QAction(QIcon(":/images/about-48x48.png"),tr("About"), this));

    ACTIONS()->insert(APP_SHOW_SETTINGS, new QAction(QIcon(":/images/settings-48x48.png"), tr("settings"), this));
    
    /* playlist action */
    ACTIONS()->insert(PLAYQUEUE_ADD_FILE, new QAction(QIcon(":/images/track-48x48.png"),tr("&Add media to playlist"), this));
    ACTIONS()->insert(PLAYQUEUE_ADD_DIR, new QAction(QIcon(":/images/folder-48x48.png"),tr("&Add directory to playlist"), this));
    ACTIONS()->insert(PLAYQUEUE_ADD_URL, new QAction(QIcon(":/images/media-url-48x48.png"),tr("&Add Url..."), this));
    ACTIONS()->insert(PLAYQUEUE_CLEAR, new QAction(QIcon::fromTheme("edit-clear-list"), tr("&Clear playlist"), this));    
    ACTIONS()->insert(PLAYQUEUE_SAVE, new QAction(QIcon(":/images/save-32x32.png"), tr("&Save playlist to file"), this));
    ACTIONS()->insert(PLAYQUEUE_AUTOSAVE, new QAction(QIcon(":/images/save-32x32.png"), tr("&Auto save playlist to yarock database"), this));
    ACTIONS()->insert(PLAYQUEUE_REMOVE_ITEM,new QAction(QIcon::fromTheme("edit-delete"),tr("&Remove media from playlist"), this));
    ACTIONS()->insert(PLAYQUEUE_STOP_AFTER,new QAction(QIcon(),tr("Stop after this track"), this));
    ACTIONS()->value(PLAYQUEUE_STOP_AFTER)->setCheckable(true);
    ACTIONS()->insert(PLAYQUEUE_SORT,new QAction(QIcon(),"", this));
    ACTIONS()->insert(PLAYQUEUE_REMOVE_DUPLICATE,new QAction(QIcon(),tr("Remove duplicate"), this));
    
    ACTIONS()->value(PLAYQUEUE_REMOVE_DUPLICATE)->setCheckable(true);
    ACTIONS()->value(PLAYQUEUE_REMOVE_DUPLICATE)->setChecked( !SETTINGS()->_playqueueDuplicate );

    ACTIONS()->insert(PLAYQUEUE_TRACK_LOVE, new QAction(QIcon(":/images/lastfm.png"), tr("Send LastFm love"), this));
    
    /* Option d'affichage de la playlist */
      ACTIONS()->insert(PLAYQUEUE_OPTION_SHOW_COVER, new QAction(tr("Show cover"), this));
      ACTIONS()->value(PLAYQUEUE_OPTION_SHOW_COVER)->setCheckable(true);
      ACTIONS()->value(PLAYQUEUE_OPTION_SHOW_COVER)->setChecked( SETTINGS()->_playqueueShowCover );

      ACTIONS()->insert(PLAYQUEUE_OPTION_SHOW_RATING, new QAction(tr("Show rating"), this));
      ACTIONS()->value(PLAYQUEUE_OPTION_SHOW_RATING)->setCheckable(true);
      ACTIONS()->value(PLAYQUEUE_OPTION_SHOW_RATING)->setChecked( SETTINGS()->_playqueueShowRating );
    
    /* player action*/
    ACTIONS()->insert(ENGINE_PLAY, new QAction(QIcon(":/images/media-play.png"), tr("Play or Pause media"), this));
    ACTIONS()->insert(ENGINE_STOP, new QAction(QIcon(":/images/media-stop.png"), tr("Stop playing media"), this));
    ACTIONS()->insert(ENGINE_PLAY_NEXT, new QAction(QIcon(":/images/media-next.png"), tr("Play next media"), this));
    ACTIONS()->insert(ENGINE_PLAY_PREV, new QAction(QIcon(":/images/media-prev.png"), tr("Play previous media"), this));
    ACTIONS()->insert(ENGINE_VOL_MUTE, new QAction(QIcon(":/images/volume-icon.png"),"", this));
    ACTIONS()->insert(ENGINE_AUDIO_EQ, new QAction(QIcon(":/images/equalizer-48x48-1.png"),tr("Audio equalizer"), this));

    /* database action */
    ACTIONS()->insert(DIALOG_DB_OPERATION, new QAction(QIcon(":/images/rebuild.png"),tr("Database operation"), this));

    /* Show/hide MenuBar / PlayQueue / Menu */
    ACTIONS()->insert(APP_SHOW_PLAYQUEUE, new QAction(QIcon(),tr("Show playqueue panel"), this));
    ACTIONS()->insert(APP_SHOW_MENU, new QAction(QIcon(),tr("Show menu panel"), this));
    ACTIONS()->insert(APP_SHOW_NOW_PLAYING, new QAction(QIcon(),tr("Show now playing"), this));
    ACTIONS()->value(APP_SHOW_PLAYQUEUE)->setCheckable(true);
    ACTIONS()->value(APP_SHOW_MENU)->setCheckable(true);
    ACTIONS()->value(APP_SHOW_NOW_PLAYING)->setCheckable(true);

    /* Screen mode Switch actions */
    ACTIONS()->insert(APP_MODE_COMPACT, new QAction(QIcon(":/images/screen-minimalmode.png"), tr("Switch to minimal mode"), this));
    ACTIONS()->insert(APP_MODE_NORMAL, new QAction(QIcon(":/images/screen-normalmode.png"), tr("Switch to normal mode"), this));

    /* jump to track  */
    ACTIONS()->insert(BROWSER_JUMP_TO_ARTIST, new QAction(QIcon(":/images/jump_to_32x32.png"),tr("Jump to artist"), this));
    ACTIONS()->insert(BROWSER_JUMP_TO_ALBUM,  new QAction(QIcon(":/images/jump_to_32x32.png"),tr("Jump to album"), this));
    ACTIONS()->insert(BROWSER_JUMP_TO_TRACK,  new QAction(QIcon(":/images/jump_to_32x32.png"),tr("Jump to track"), this));
    ACTIONS()->insert(BROWSER_JUMP_TO_MEDIA,  new QAction(QIcon(":/images/jump_to_32x32.png"),QString(), this));
    
    ACTIONS()->insert(PLAYQUEUE_JUMP_TO_TRACK, new QAction(QIcon(":/images/jump_to_32x32.png"),tr("Jump to track"), this));
    
    set_enable_jump_to(false);

    ACTIONS()->insert(APP_ENABLE_SEARCH_POPUP, new QAction(QIcon(),tr("Enable search popup"), this));
    ACTIONS()->value(APP_ENABLE_SEARCH_POPUP)->setCheckable(true);

    /* restore Actions states              */
    (ACTIONS()->value(APP_SHOW_PLAYQUEUE))->setChecked(SETTINGS()->_showPlayQueuePanel);
    (ACTIONS()->value(APP_SHOW_MENU))->setChecked(SETTINGS()->_showMenuPanel);
    (ACTIONS()->value(APP_SHOW_NOW_PLAYING))->setChecked(SETTINGS()->_showNowPlaying);
    (ACTIONS()->value(APP_ENABLE_SEARCH_POPUP))->setChecked(SETTINGS()->_enableSearchPopup);
}


//! --------- connectSlots -----------------------------------------------------
void MainWindow::connectSlots()
{
    //! Global Yarock Actions
    QObject::connect(ACTIONS()->value(APP_QUIT), SIGNAL(triggered()), SLOT(slot_on_yarock_quit()));
    QObject::connect(ACTIONS()->value(APP_SHOW_YAROCK_ABOUT), SIGNAL(triggered()), SLOT(slot_on_aboutYarock()));
    QObject::connect(ACTIONS()->value(APP_SHOW_SETTINGS), SIGNAL(triggered()), SLOT(slot_on_show_settings()));

    //! Playback option
    QObject::connect(ACTIONS()->value(PLAYQUEUE_STOP_AFTER), SIGNAL(triggered()), SLOT(slot_stop_after_media_triggered()));

    //! Connection Actions player
    QObject::connect(_player, SIGNAL(mediaChanged()), this, SLOT(slot_player_on_track_change()));
    QObject::connect(_player, SIGNAL(mediaAboutToFinish()), this, SLOT(slot_player_enqueue_next()));
    QObject::connect(_player, SIGNAL(engineStateChanged()), this, SLOT(slot_player_on_state_change()));
    QObject::connect(_player, SIGNAL(engineRequestStop()), this, SLOT(stopPlayer()));

    QObject::connect(ACTIONS()->value(ENGINE_PLAY), SIGNAL(triggered()), SLOT(playOrPause()));
    QObject::connect(ACTIONS()->value(ENGINE_PLAY_NEXT), SIGNAL(triggered()), SLOT(playNext()));
    QObject::connect(ACTIONS()->value(ENGINE_PLAY_PREV), SIGNAL(triggered()), SLOT(playPrev()));
    QObject::connect(ACTIONS()->value(ENGINE_STOP), SIGNAL(triggered()), SLOT(stopPlayer()));
    QObject::connect(ACTIONS()->value(ENGINE_AUDIO_EQ), SIGNAL(triggered()), SLOT(slot_eq_openDialog()));

    //! Connection Actions playlist
    QObject::connect(ACTIONS()->value(PLAYQUEUE_CLEAR), SIGNAL(triggered()), SLOT(slot_playqueue_clear()));
    QObject::connect(ACTIONS()->value(PLAYQUEUE_SAVE), SIGNAL(triggered()), SLOT(slot_playqueue_save()));
    QObject::connect(ACTIONS()->value(PLAYQUEUE_AUTOSAVE), SIGNAL(triggered()), SLOT(slot_playqueue_save_auto()));
    QObject::connect(ACTIONS()->value(PLAYQUEUE_ADD_FILE), SIGNAL(triggered()), SLOT(slot_playqueue_add()));
    QObject::connect(ACTIONS()->value(PLAYQUEUE_ADD_DIR), SIGNAL(triggered()), SLOT(slot_playqueue_add()));
    QObject::connect(ACTIONS()->value(PLAYQUEUE_ADD_URL), SIGNAL(triggered()), SLOT(slot_playqueue_add()));
    QObject::connect(_playlistView, SIGNAL(signal_playlist_itemDoubleClicked()), this, SLOT(slot_play_from_playqueue()));

    QObject::connect(ACTIONS()->value(PLAYQUEUE_SORT), SIGNAL(triggered()), this,  SLOT(slot_playqueue_sort()));

    //! Connection Actions Database
    QObject::connect(ACTIONS()->value(DIALOG_DB_OPERATION), SIGNAL(triggered()), SLOT(slot_database_dialog()));

    //! Connection signaux COLLECTION
    QObject::connect(m_virtual_queue, SIGNAL(signal_collection_playTrack()), this, SLOT(slot_play_from_collection()));

    //! Connection signaux USER SETTINGS
    QObject::connect(m_browserView, SIGNAL(settings_saved()), SLOT(slot_on_settings_saved()));

    //! ThreadManager connection
    QObject::connect(MainLeftWidget::instance(), SIGNAL(dbNameChanged()), this, SLOT(slot_database_start()));;
    QObject::connect(m_playqueue->manager(), SIGNAL(playlistSaved()), m_thread_manager, SLOT(populateLocalPlaylistModel()));
    
    //! Screen mode connection
    QObject::connect(ACTIONS()->value(APP_MODE_COMPACT), SIGNAL(triggered()), SLOT(slot_widget_mode_switch()));;
    QObject::connect(ACTIONS()->value(APP_MODE_NORMAL), SIGNAL(triggered()), SLOT(slot_widget_mode_switch()));
}


//! --------- Quit Actions -----------------------------------------------------
void MainWindow::slot_on_yarock_quit()
{
    m_canClose = true;
    this->close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(!m_trayIcon) {
      this->close();
    }
    else {
      // only hide app is tray is running
      if (m_trayIcon->isVisible() && (m_canClose == false)) {
        this->hide();
        event->ignore();
      }
    }
}

//! -------------- Systray methods ---------------------------------------------
void MainWindow::slot_systray_clicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        bool is_visible = !this->isVisible();
        this->setVisible(is_visible);
        if (is_visible) {
          activateWindow();
          showNormal();
        }
    }
}

/*******************************************************************************
    Global actions
*******************************************************************************/
void MainWindow::slot_on_aboutYarock()
{
    m_browserView->active_view(VIEW::ViewAbout,QString(),QVariant());
}

void MainWindow::slot_on_show_settings()
{
    m_browserView->active_view(VIEW::ViewSettings,QString(),QVariant());
}


/*******************************************************************************
    File management
*******************************************************************************/
void MainWindow::slot_playqueue_add()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action) return;

    /*--------------------------------------------------*/
    /* add file to playqueue                            */
    /* -------------------------------------------------*/
    if( action == ACTIONS()->value(PLAYQUEUE_ADD_FILE) )
    {
      FileDialog fd(this, FileDialog::AddFiles, tr("Add music files or playlist"));

      if(fd.exec() == QDialog::Accepted) {
        QStringList files  = fd.addFiles();
        m_playqueue->manager()->playlistAddFiles(files);
      }      
    }
    /*--------------------------------------------------*/
    /* add dir to playqueue                             */
    /* -------------------------------------------------*/
    else if ( action == ACTIONS()->value(PLAYQUEUE_ADD_DIR) )
    {
      FileDialog fd(this, FileDialog::AddDirs, tr("Add music directories"));

      if(fd.exec() == QDialog::Accepted) {
        QStringList dirs  = fd.addDirectories();
        m_playqueue->manager()->playlistAddFiles(dirs);
      }    
    }
    /*--------------------------------------------------*/
    /* add url to playqueue                             */
    /* -------------------------------------------------*/
    else if ( action == ACTIONS()->value(PLAYQUEUE_ADD_URL) )
    {
      AddStreamDialog stream_dialog(this,false);

      if(stream_dialog.exec() == QDialog::Accepted)
      {
        const QString url   = stream_dialog.url();

        if(!QUrl(url).isEmpty() && QUrl(url).isValid()) {
          const QString name  = stream_dialog.name();

          MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());
          media->setType(TYPE_STREAM);
          media->id          = -1;
          media->url         = url;
          media->name        = !name.isEmpty() ? name : url ;
          media->isFavorite  = false;
          media->isPlaying   = false;
          media->isBroken    = false;
          media->isPlayed    = false;
          media->isStopAfter = false;

          m_playqueue->addMediaItem(media);
          media.reset();
        }
        else {
          _statusWidget->startShortMessage("invalid url can not be added !!", STATUS::TYPE_WARNING, 5000);
        }
      }
    }
}


void MainWindow::slot_playqueue_save()
{
    FileDialog fd(this, FileDialog::SaveFile, tr("Save playlist to file"));

    if(fd.exec() == QDialog::Accepted) {
      QString file  = fd.saveFile();
      if (!file.isEmpty())
        m_playqueue->manager()->playlistSaveToFile(file);
    }  
}

void MainWindow::slot_playqueue_save_auto()
{
    DialogInput input(this, tr("Playlist name"), tr("Save playlist"));
    input.setFixedSize(480,140);
    

    if(input.exec() == QDialog::Accepted) {
      Debug::debug() << "MainWindow::slot_playqueue_save_auto : " << input.editValue();
      m_playqueue->manager()->playlistSaveToDb(input.editValue());
    }
}


/*******************************************************************************
    Playlist
*******************************************************************************/
void MainWindow::slot_playqueue_clear()
{
    m_playqueue->clear();

    // on arrete la lecture seulement si il n'y a pas de lecture a partir
    // de la collection
    if((_playRequestFrom == FromPlayQueue) && SETTINGS()->_stopOnPlayqueueClear) 
    {
      m_playqueue->updatePlayingItem(MEDIA::TrackPtr(0));
      m_virtual_queue->updatePlayingItem(MEDIA::TrackPtr(0));
      this->stopPlayer();
    }
}


void MainWindow::slot_playqueue_sort()
{  
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action) return;
    
    QVariant sort_query = action->data();

    m_playqueue->slot_sort(sort_query);
}


/*******************************************************************************
    Player management
*******************************************************************************/
void MainWindow::slot_player_on_state_change()
{
   Debug::debug() << Q_FUNC_INFO;
   
   ENGINE::E_ENGINE_STATE state = _player->state();

   switch (state) {
     /**************** STOPPED **********************/
     case ENGINE::STOPPED:
        set_enable_jump_to(false);
        (ACTIONS()->value(ENGINE_STOP))->setEnabled(false);
        (ACTIONS()->value(ENGINE_PLAY))->setIcon(QIcon(":/images/media-play.png"));
     break;
     /**************** PLAYING ***********************/
     case ENGINE::PLAYING:
        (ACTIONS()->value(ENGINE_STOP))->setEnabled(true);
        (ACTIONS()->value(ENGINE_PLAY))->setIcon(QIcon(":/images/media-pause.png"));
     break;
     /**************** PAUSE *************************/
     case ENGINE::PAUSED:
        (ACTIONS()->value(ENGINE_PLAY))->setIcon(QIcon(":/images/media-play.png"));
     break;
     /**************** ERROR *************************/
     case ENGINE::ERROR:
     {
         Debug::warning() << "ENGINE error";

         _statusWidget->startShortMessage(tr("Playing error"), STATUS::TYPE_ERROR, 5000);
         MEDIA::TrackPtr track = _player->playingTrack();
         if(track)
         {
           MEDIA::registerTrackBroken(track, true);

           if(_playRequestFrom == FromCollection)
             m_virtual_queue->updatePlayingItem(track);
           else
             m_playqueue->updatePlayingItem(track);
         }
         stopPlayer();
      }
      break;
      default:break;
    }
}


void MainWindow::slot_player_on_track_change()
{
    Debug::debug() << "MainWindow -> slot_player_on_track_change ";
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();

    if(!track) {
      Debug::error() << "MainWindow -> slot_player_on_track_change track ERROR";
      return;
    }
    
    /*  update playing track */
    MEDIA::registerTrackPlaying(track, true);
    m_playqueue->updatePlayingItem(track);
    m_virtual_queue->updatePlayingItem(track);
    
    /*  update jump to action */
    if ( (track->type() == TYPE_TRACK) && (track->id != -1) )
      set_enable_jump_to(true);
    else
      set_enable_jump_to(false);
}

//! --------- slot_play_from_playqueue -----------------------------------------
// a file is activated from the playlist
void MainWindow::slot_play_from_playqueue()
{
    //Debug::debug() << "Mainwindow -> slot_play_from_playqueue";
    _playRequestFrom = FromPlayQueue;

    const MEDIA::TrackPtr track = m_playqueue->requestedTrack();

    if(track)
      _player->setMediaItem( track );
}

//! --------- slot_play_from_collection ----------------------------------------
// a MediaItem is activated from collection browser
void MainWindow::slot_play_from_collection()
{
    //Debug::debug() << "Mainwindow -> slot_play_from_collection";
  
    _playRequestFrom = FromCollection;

    const MEDIA::TrackPtr track = m_virtual_queue->requestedTrack();

    if(track)
      _player->setMediaItem( track );
}



//! --------- playOrPause ------------------------------------------------------
void MainWindow::playOrPause()
{
    if (_player->state() == ENGINE::PLAYING)
    {
      _player->pause();
    }
    else if (_player->state() == ENGINE::PAUSED)
    {
      _player->play();
    }
    else
    {
        if(m_browserView->playSelectedItems())
          return;

        MEDIA::TrackPtr media;
        if( _playlistView->isTrackSelected() )
          media = _playlistView->firstSelectedTrack();
        else
          media = m_playqueue->trackAt(0);

        if(media)
          _player->setMediaItem( media );
    }
}

//! --------- stopPlayer -------------------------------------------------------
void MainWindow::stopPlayer()
{
     Debug::debug() << "Mainwindow -> stopPlayer";

    _player->stop();

    m_virtual_queue->updatePlayingItem(MEDIA::TrackPtr(0));
    m_playqueue->updatePlayingItem(MEDIA::TrackPtr(0));
}

//! --------- slot_player_enqueue_next -----------------------------------------
void MainWindow::slot_player_enqueue_next()
{
    Debug::debug() << "Mainwindow -> slot_player_enqueue_next";
    MEDIA::TrackPtr media;

    if(_playRequestFrom == FromCollection)
      media = m_virtual_queue->nextTrack();
    else
      media = m_playqueue->nextTrack();

    if(media)
      _player->setNextMediaItem(media);
    else 
    {
      if (_playRequestFrom == FromPlayQueue)
        emit playlistFinished();
    }
}


//! --------- playNext ---------------------------------------------------------
void MainWindow::playNext()
{
    Debug::debug() << "Mainwindow -> playNext requested ";
    MEDIA::TrackPtr media;

    //! We need to known if we are playing from collection or playlist
    if(_playRequestFrom == FromCollection)
      media = m_virtual_queue->nextTrack();
    else
      media = m_playqueue->nextTrack();

    if(media)
      _player->setMediaItem(media);
    else
      stopPlayer();
}

//! --------- playPrev ---------------------------------------------------------
void MainWindow::playPrev()
{
    MEDIA::TrackPtr media;

    //! We need to known if we are playing from collection or playlist
    if(_playRequestFrom == FromCollection)
      media = m_virtual_queue->prevTrack();
    else
      media = m_playqueue->prevTrack();

    if(media)
      _player->setMediaItem(media);
    else
      stopPlayer();
}

//! --------- savePlayingTrack -------------------------------------------------
void MainWindow::savePlayingTrack()
{
    Debug::debug() << "Mainwindow -> savePlayingTrack";
    if (_player->state() == ENGINE::PLAYING)
    {
      SETTINGS()->_playingUrl = _player->playingTrack()->url;
      
      if( MEDIA::isLocal(SETTINGS()->_playingUrl) )
         SETTINGS()->_playingPosition = _player->currentTime();
      else
         SETTINGS()->_playingPosition = 0;
    }
}

//! --------- restorePlayingTrack ----------------------------------------------
void MainWindow::restorePlayingTrack()
{
    Debug::debug() << "Mainwindow -> restorePlayingTrack";
    const QString url = SETTINGS()->_playingUrl;
    if(url.isEmpty())
      return;

    if( MEDIA::isLocal(url) )
    {
        MEDIA::TrackPtr media = MEDIA::FromDataBase(url);
        if(!media)
          media = MEDIA::FromLocalFile( url );

        _player->setMediaItem(media);

        qint64 position = SETTINGS()->_playingPosition;
        Debug::debug() << "Mainwindow -> restorePlayingTrack TYPE_TRACK position = " << position;
        _player->seek( position );
    }
    else
    {
        MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());
        media->setType(TYPE_STREAM);
        media->id        = -1;
        media->url       = url;
        media->name      = url;
        media->title     = QString();
        media->artist    = QString();
        media->album     = QString();
        media->categorie = QString();
        media->isFavorite= false;
        media->isPlaying = false;
        media->isBroken  = false;
        media->isPlayed  = false;

        _player->setMediaItem(media);
    }
}

/*******************************************************************************
    Playback Option
*******************************************************************************/
void MainWindow::slot_stop_after_media_triggered()
{
    Debug::debug() << "MainWindow::slot_stop_after_media_triggered";
    MEDIA::TrackPtr requested_media = qvariant_cast<MEDIA::TrackPtr>( (ACTIONS()->value(PLAYQUEUE_STOP_AFTER))->data() );

    if(!requested_media)
      return;

    m_playqueue->setStopAfterTrack(requested_media);

    ACTIONS()->value(PLAYQUEUE_STOP_AFTER)->setChecked( requested_media->isStopAfter );
}


/*******************************************************************************
    User settings methods
*******************************************************************************/
void MainWindow::slot_on_settings_saved()
{
    Debug::debug() << "MainWindow --> slot_on_settings_saved";

    SETTINGS::Results r = m_browserView->settingsResults();

    Debug::debug() << "MainWindow --> isSystrayChanged "    << r.isSystrayChanged;
    Debug::debug() << "MainWindow --> isDbusChanged "       << r.isDbusChanged;
    Debug::debug() << "MainWindow --> isMprisChanged "      << r.isMprisChanged;
    Debug::debug() << "MainWindow --> isShorcutChanged "    << r.isShorcutChanged;
    Debug::debug() << "MainWindow --> isScrobblerChanged "  << r.isScrobblerChanged;
    Debug::debug() << "MainWindow --> isLibraryChanged "    << r.isLibraryChanged;
    Debug::debug() << "MainWindow --> isViewChanged "       << r.isViewChanged;

    if(r.isSystrayChanged)    { reloadSystraySettings();}
    if(r.isDbusChanged)       { m_dbus_notifier->reloadSettings(); }
    if(r.isMprisChanged)      { m_mpris_manager->reloadSettings(); }
    if(r.isShorcutChanged)    { m_globalShortcuts->reloadSettings();}
    if(r.isScrobblerChanged)  { LastFmService::instance()->init();}
    if(r.isLibraryChanged)
    {
      
      /* NOTE : hack pour eviter un crash de l'appli (car on ferme la connection a la 
       base de donnée puis on relance le thread database builder). Stop des thread ajouté
       avant la fermeture de la db */
      if(m_thread_manager->isDbRunning()) {
        Debug::warning() << "Database builder already running, request stop all thread!!";
        m_thread_manager->stopThread();
      }

      Database::instance()->close();

      if(!Database::instance()->exist())
        createDatabase();

      rebuildDatabase();
    }
    else if (r.isViewChanged) 
    {
        m_thread_manager->populateLocalTrackModel();
    }

    _statusWidget->startShortMessage(tr("settings saved"),STATUS::TYPE_INFO, 2500);
}



/*******************************************************************************
    Database methods
*******************************************************************************/
void MainWindow::createDatabase ()
{
    Debug::debug() << "Mainwindow -> createDatabase";
    Database::instance()->close();
    Database::instance()->remove();
    Database::instance()->create();
}

void MainWindow::rebuildDatabase()
{
    Debug::debug() << "Mainwindow -> rebuildDatabase";

    QStringList listDir = QStringList() << Database::instance()->param()._paths;

    // Database Building
    if (!listDir.isEmpty()) {
      m_thread_manager->databaseBuild(listDir);
    }
}


void MainWindow::slot_database_start()
{
    Debug::debug() << "MainWindow -->  slot_database_start";
    Database::instance()->close();

    //! check new database entry
    if( !Database::instance()->exist() )
    {
        /*-----------------------------------------------------------*/
        /* First start dialog                                        */
        /* ----------------------------------------------------------*/
        Debug::debug() << "MainWindow --> slot_database_start : first start dialog";
        /* create database */
        createDatabase();

        FirstTimeDialog dialog(this);
        if(dialog.exec() == QDialog::Accepted) 
        {
           Debug::debug() << "MainWindow --> slot_database_start : first start dialog ACCEPTED";
           rebuildDatabase();
        }
        else
        {
           m_browserView->active_view(VIEW::ViewSettings,QString(), QVariant());
        }
    }
    else if (!Database::instance()->versionOK() )
    {
        /*-----------------------------------------------------------*/
        /* Database revision change                                  */
        /* ----------------------------------------------------------*/
        Debug::debug() << "MainWindow --> slot_database_start : database revision change";
        const QString str = tr("<p>Database need to be rebuilt</p>");

        DialogMessage dlg(this, tr("Database revision update"));
        dlg.setMessage(str);
        dlg.resize(445, 120);
        dlg.exec();      

        /* delete all user data (si version depart < 19 car changement de taille cover) */
        Q_FOREACH(QFileInfo info, QDir(UTIL::CONFIGDIR + "/albums/").entryInfoList(QDir::NoDotAndDotDot | QDir::System |
                   QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            QFile::remove(info.absoluteFilePath());
        }

        Q_FOREACH(QFileInfo info, QDir(UTIL::CONFIGDIR + "/artists/").entryInfoList(QDir::NoDotAndDotDot | QDir::System |
                   QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            QFile::remove(info.absoluteFilePath());
        }

        /* create database */
        createDatabase();
        rebuildDatabase();
    }
    else if ( Database::instance()->param()._option_auto_rebuild)
    {
        /*-----------------------------------------------------------*/
        /* Start existing database with auto rebuild at startup      */
        /* ----------------------------------------------------------*/
        rebuildDatabase();
    }
    else
    {
        /*-----------------------------------------------------------*/
        /* Start existing database by populating models              */
        /* ----------------------------------------------------------*/
        m_thread_manager->populateLocalTrackModel();
    }
    
    connect(m_thread_manager, SIGNAL(modelPopulationFinished(E_MODEL_TYPE)), this, SLOT(slot_restore_playqueue()), Qt::UniqueConnection);
    
    /* process command line */
    commandlineOptionsHandle();
}

void MainWindow::slot_database_dialog()
{
    /* case 1 : remove database and rescan all collection directories */
    /* case 2 : scan directory changes and update database */
    DbOperationDialog dialog(this);

    if( dialog.exec() == QDialog::Accepted)
    {
      Debug::debug() << "OPERATION = " << dialog.operation();
      switch( dialog.operation() )
      {
        case DbOperationDialog::OPE_REBUILD:
          createDatabase();
        case DbOperationDialog::OPE_RESCAN:
          rebuildDatabase();
          break;
        case DbOperationDialog::OPE_COVER:
          m_thread_manager->startTagSearch(TagSearch::ALBUM_COVER_FULL);
          break;
        case DbOperationDialog::OPE_ARTIST_IMAGE:
          m_thread_manager->startTagSearch(TagSearch::ARTIST_IMAGE_FULL);
          break;
        case DbOperationDialog::OPE_GENRE_TAG:
          m_thread_manager->startTagSearch(TagSearch::ALBUM_GENRE_FULL);
          break;

        default:break;
      }
    }
}


void MainWindow::slot_restore_playqueue()
{
    if( is_first_start) 
    {
      Debug::debug() << "[MainWindow] restore last playqueue content";
      if( SETTINGS()->_restorePlayqueue)
        m_playqueue->manager()->restorePlayqueueSession();

      is_first_start = false;
    }  
}

/*******************************************************************************
    Jump to
*******************************************************************************/
void MainWindow::set_enable_jump_to(bool b)
{
    ACTIONS()->value(BROWSER_JUMP_TO_ARTIST)->setEnabled(b);
    ACTIONS()->value(BROWSER_JUMP_TO_ALBUM)->setEnabled(b);
    ACTIONS()->value(BROWSER_JUMP_TO_TRACK)->setEnabled(b);
    ACTIONS()->value(PLAYQUEUE_JUMP_TO_TRACK)->setEnabled(b);
    
}


/*******************************************************************************
    Compact mode widget management
*******************************************************************************/
void MainWindow::slot_widget_mode_switch()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action) return;

    /*--------------------------------------------------*/
    /* switch to minimal widget                         */
    /* -------------------------------------------------*/
    if( action == ACTIONS()->value(APP_MODE_COMPACT) )
    {
      if(m_minimalwidget == NULL)
        m_minimalwidget = new MinimalWidget(this);

      this->hide();
      m_minimalwidget->move(this->pos());
      m_minimalwidget->show();
    }
    /*--------------------------------------------------*/
    /* switch to full window widget                     */
    /* -------------------------------------------------*/
    else if( action == ACTIONS()->value(APP_MODE_NORMAL) )
    {
      if(m_minimalwidget != NULL)
        m_minimalwidget->hide();

      this->show();
    }
}
/*******************************************************************************
    AudioEqualizer Dialog
*******************************************************************************/
void MainWindow::slot_eq_openDialog()
{
    if(_player->isEqualizerAvailable()) 
    {
      Equalizer_Dialog *eqDialog = new Equalizer_Dialog(this);
      
      connect(eqDialog, SIGNAL(enabledChanged(bool)), SLOT(slot_eq_enableChange(bool)));
      connect(eqDialog, SIGNAL(newEqualizerValue(int, QList<int>)), SLOT(slot_eq_paramChange(int, QList<int>)));
      eqDialog->exec();
      eqDialog->deleteLater();
    }
    else 
    {
        QString str = tr("<b>Equalizer is not available</b>"
        "<p>Equalizer wasn't found, probably you are using a backend that doesn't support it.</p>"
        "<p>Change to another backend (gstreamer is supported) if you want to have equalizer</p>");

        DialogMessage dlg(this, QString(tr("Equalizer information")));
        dlg.setMessage(str);
        dlg.resize(445, 120);
        dlg.exec();      
    }
}

void MainWindow::slot_eq_enableChange(bool eqActivated)
{
    Debug::debug() << "MainWindow --> slot_eq_enableChange bool" << eqActivated;
    if(eqActivated) 
    {
        ACTIONS()->value(ENGINE_AUDIO_EQ)->setIcon(QIcon(":/images/equalizer-48x48-2.png"));
        _player->addEqualizer();
    }
    else
    {
        ACTIONS()->value(ENGINE_AUDIO_EQ)->setIcon(QIcon(":/images/equalizer-48x48-1.png"));
        _player->removeEqualizer();
    }
}


void MainWindow::slot_eq_paramChange(int preamp, QList<int> listGain)
{
    Debug::debug() << "MainWindow --> slot_eq_paramChange";

    QList<int> gains;
    gains << preamp << listGain;

    _player->applyEqualizer(gains);
}

/*******************************************************************************
    Command Line options
*******************************************************************************/
void MainWindow::set_command_line(const CommandlineOptions& options)
{
    m_options = options;
}

void MainWindow::slot_commandline_received(const QByteArray& serialized_options)
{
    if(m_minimalwidget != NULL)
       m_minimalwidget->hide();     
     
    this->show();     
    this->activateWindow();
    this->showNormal();
     
    CommandlineOptions options;
    options.Load(serialized_options);

    set_command_line(options);
    commandlineOptionsHandle();
}

void MainWindow::commandlineOptionsHandle(/*const CommandlineOptions &options*/)
{
    Debug::debug() << "MainWindow --> commandlineOptionsHandle";
  
    if (m_options.isEmpty()) {
      return;
    }

    switch (m_options.player_action())
    {
      case CommandlineOptions::Player_Play:        this->playOrPause();break;
      case CommandlineOptions::Player_PlayPause:   this->playOrPause();break;
      case CommandlineOptions::Player_Pause:       this->playOrPause();break;
      case CommandlineOptions::Player_Stop:        this->stopPlayer();break;
      case CommandlineOptions::Player_Previous:    this->playPrev();break;
      case CommandlineOptions::Player_Next:        this->playNext();break;
      case CommandlineOptions::Player_None: /* do nothing */      break;
    }

    switch (m_options.playlist_action())
    {
      case CommandlineOptions::Playlist_Load:
        this->slot_playqueue_clear(); // no break !
      case CommandlineOptions::Playlist_Append:
        m_playqueue->manager()->playlistAddUrls(m_options.urls());
        break;
      
       case CommandlineOptions::Playlist_Default:
        /* new case to handle Open with yarock simple case (enqueue and play the new tracks) */
        m_playqueue_index = m_playqueue->size() -1;
        m_playqueue->manager()->playlistAddUrls(m_options.urls());
        connect(m_playqueue->manager(), SIGNAL(playlistPopulated()), this, SLOT(slot_play_after_playqueue_loaded()), Qt::UniqueConnection);
        break;
    }    
    
    if (m_options.set_volume() != -1)
      _player->setVolume(m_options.set_volume()); //! shall be in percent !

    if (m_options.volume_modifier() != 0)
      _player->setVolume(_player->volume() + m_options.volume_modifier());

    if (m_options.seek_to() != -1)
      _player->seek( m_options.seek_to() );
    else if (m_options.seek_by() != 0)
      _player->seek( _player->currentTime() +  m_options.seek_by() );

    if (m_options.play_track_at() != -1) {
      MEDIA::TrackPtr media = m_playqueue->trackAt(m_options.play_track_at());      
      if(media) {
        _playRequestFrom = FromPlayQueue;
        m_virtual_queue->updatePlayingItem(MEDIA::TrackPtr(0));
        _player->setMediaItem(media);
      }
    }
}

/* specific case to handle open with yarock + url   */
/* => enqueue knew tracks + play first enqueued one */
void MainWindow::slot_play_after_playqueue_loaded()
{
     this->stopPlayer();
     m_playqueue->setRequestedTrackAt(m_playqueue_index + 1);
     this->slot_play_from_playqueue();
}


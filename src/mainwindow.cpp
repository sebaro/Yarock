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

#include "mainwindow.h"

// player engine
#include "core/player/engine.h"

// widget
#include "playqueue/playlistview.h"
#include "playqueue/playlistwidget.h"
#include "widgets/main/centralwidget.h"
#include "widgets/main/menumodel.h"
#include "widgets/minimalwidget.h"
#include "widgets/statusmanager.h"
#include "widgets/equalizer/equalizer_dialog.h"

#include "widgets/editors/editor_playlist.h"
#include "widgets/iconloader.h"

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
#include "commandlineoptions.h"
#include "shortcuts_manager.h"
#include "settings.h"

#include "networkaccess.h"
#include "covercache.h"

#include "iconmanager.h"
#include "global_actions.h"
#include "systray.h"
#include "config.h"
#include "utilities.h"
#include "debug.h"

// Dbus & remote
#include "dbus/dbusnotification.h"
#include "dbus/mpris_manager.h"

// dialog
#include "dialogs/database_operation.h"
#include "dialogs/database_add.h"


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

    Debug::debug() << "line :"<< __LINE__ <<  " MainWindow -> start";
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

    //! ###############   Fonts #############################################
    IconManager* im = new IconManager(this);
    im->initFontAwesome();
    
    
    //! ###############   Settings   ########################################
    new YarockSettings();
    SETTINGS()->readSettings();

    //! ###############   Actions    ########################################
    new GlobalActions();
    createActions();

    //! ###############  Player #############################################
    Engine* engine = new Engine();
    
    _player = Engine::instance();

    //! Info system
    InfoSystem::instance();

    //! ############### init collection database ############################
    Debug::debug() << "[Mainwindow] init database";
    new Database();

    //! ############### global instance #####################################
    new RatingPainter();
    new CoverCache();
    
    //! ############### init playqueue part  ################################
    Debug::debug() << "[Mainwindow] creation Playqueue part";
    PlaylistWidget* playlistWidget  = new PlaylistWidget(this);
    m_playqueue      = playlistWidget->model();

    //! ############### init data model  ####################################
    Debug::debug() << "[Mainwindow] init data model";
    m_localTrackModel     = new LocalTrackModel(this);
    m_localPlaylistModel  = new LocalPlaylistModel(this);
    m_histoModel          = new HistoModel(this);
    m_streamModel         = new StreamModel(this);

    //! ############### Thread management ###################################
    m_thread_manager = new ThreadManager();
    
    //! ############### End Gui Stuff     ###################################
    m_centralWidget = new CentralWidget(this);
    this->setCentralWidget(m_centralWidget);

    QObject::connect(MainRightWidget::instance(), SIGNAL(playqueueAdded(QWidget*)), SLOT(slot_playqueue_added(QWidget*)));
    QObject::connect(MainRightWidget::instance(), SIGNAL(playqueueRemoved(QWidget*)), SLOT(slot_playqueue_removed(QWidget*)));
    MainRightWidget::instance()->addWidget( playlistWidget );
    
    //! ############### init views  #########################################
    //Debug::debug() << "[Mainwindow] creation BrowserView";
    m_virtual_queue       = new VirtualPlayqueue(this);
    m_browserView         = new BrowserView(this);
    m_centralWidget->setBrowser(m_browserView); 
    
    //! ############### Status Widget     ###################################
    m_statusManager = new StatusManager(m_centralWidget);
    
    //! Shortcuts & Signals
    //Debug::debug() << "[Mainwindow] creation global shortcut";
    m_shortcutsManager = new ShortcutsManager (this);

    //! ###############     Connection    ###################################
    //Debug::debug() << "[Mainwindow] connect signals & slots";
    connectSlots();

    //! ############### Scrobbler ###########################################
    // le scrobbler doit etre initialise avant le fisrtStartDialog
    //Debug::debug() << "[Mainwindow] init lastFm scrobbler";
    LastFmService::instance()->init();

    //! ############### start database ######################################
    //Debug::debug() << "MainWindow -> start timer for database update";
    /* use Timer to let Maindow show before all database stuff is done */
    QTimer::singleShot(4, this, SLOT(slot_database_start()));

    //! ############### Systray Icon ########################################
    m_canClose = false;
    m_systray = new SysTray(this);

    //! ############### Minimal window ######################################
    m_minimalwidget = NULL;

    //! ############### History manager #####################################
    //Debug::debug() << "[Mainwindow] m_histoManager";
    m_histoManager = new HistoManager();

    //! ############### DBUS & MPRIS ########################################
    //Debug::debug() << "[Mainwindow] Dbus & Mpris";
    m_dbus_notifier   = new DbusNotification(this);
    m_mpris_manager   = new MprisManager(this);

    //! ############### Restore Equalizer ###################################
    if( SETTINGS()->_enableEq && _player->isEqualizerAvailable()) 
    {
        ACTIONS()->value(ENGINE_AUDIO_EQ)->setIcon(IconManager::instance()->icon("equalizer","selected"));
    }
        
    //! ############### Restore windows geometry ############################
    Debug::debug() << "[Mainwindow] restore geometry";
    if( !SETTINGS()->_windowsGeometry.isEmpty() )
      restoreGeometry(SETTINGS()->_windowsGeometry);
    else
      resize(1200, 800);

    QTimer::singleShot(50, m_centralWidget, SLOT(restoreState()));
    
    /* use time to have good Browser width before populating the scene */
    QTimer::singleShot(100, m_browserView, SLOT(restore_view()));
    
    //! ############### Hide window       ###################################
    if(SETTINGS()->_hideAtStartup)
      this->showMinimized();    
    
    Debug::debug() << "[Mainwindow] check engine startup";
    
    /* Check startup */
    if ( !engine->error().isEmpty() )
      m_statusManager->startMessage( engine->error(), STATUS::ERROR_CLOSE );

    slot_player_on_state_change();

    m_playingModel = 0;
    
// #ifdef TEST_FLAG    
//     QTimer::singleShot(4000, this, SLOT(slot_start_test()));
// #endif    
}

#ifdef TEST_FLAG

void MainWindow::slot_start_test()
{
    //Debug::debug() << "####### START TEST #######";
}
#endif


//! --------- ~MainWindow ------------------------------------------------------
MainWindow::~MainWindow()
{
    /* ---- Save playqueue content to database ---- */
    if(SETTINGS()->_restorePlayqueue)
      m_playqueue->manager()->savePlayqueueSession();

    /* ---- Save current track playing ---- */
    if(SETTINGS()->_restartPlayingAtStartup)
      savePlayingTrack();

    /* ---- Stop player ---- */
    SETTINGS()->_volumeLevel = _player->volume();
    _player->stop();
    
    /* ---- Save widget option setting ---- */
    m_centralWidget->saveState();
    m_browserView->save_view();
    SETTINGS()->_windowsGeometry = this->saveGeometry();
    SETTINGS()->_windowsState    = this->saveState();
    SETTINGS()->_playqueueShowCover  = ACTIONS()->value(PLAYQUEUE_OPTION_SHOW_COVER)->isChecked();
    SETTINGS()->_playqueueShowRating = ACTIONS()->value(PLAYQUEUE_OPTION_SHOW_RATING)->isChecked();
    SETTINGS()->_playqueueShowNumber = ACTIONS()->value(PLAYQUEUE_OPTION_SHOW_NUMBER)->isChecked();
    SETTINGS()->_playqueueDuplicate  = !ACTIONS()->value(PLAYQUEUE_REMOVE_DUPLICATE)->isChecked();

    SETTINGS()->writeSettings();
    
    Database::instance()->settings_save();

    /* ---- delete object ---- */
    Debug::debug() << "[Mainwindow] EXIT delete object";
    QPixmapCache::clear();
    delete LastFmService::instance();
    delete m_dbus_notifier;
    delete m_mpris_manager;
    delete m_localTrackModel;
    delete m_localPlaylistModel;
    delete _player;

    /* ---- stop main task ---- */
    Debug::debug() << "[Mainwindow] Stop Thread Manager";
    m_thread_manager->stopThread();    
    delete m_thread_manager;
    Debug::debug() << "[Mainwindow] Stop Thread Manager OK";
    

    
    /* ---- stop playqueue task ---- */
    /* !!! wait for playqueue task to be done */
    delete m_playqueue->manager();

    /* ---- database close ---- */
    Database::instance()->close();
    Debug::debug() << "[Mainwindow] EXIT Bye Bye";
}



//! --------- createActions ----------------------------------------------------
void MainWindow::createActions()
{
    Debug::debug() << "[Mainwindow] createActions";
    
    /* yarock global actions */
    ACTIONS()->insert(APP_QUIT, new QAction(IconLoader::Load("application-exit"),tr("&Quit"), this));
    ACTIONS()->insert(APP_SHOW_YAROCK_ABOUT, new QAction(QIcon(":/images/about-48x48.png"),tr("About"), this));

    ACTIONS()->insert(APP_SHOW_SETTINGS, new QAction(IconManager::instance()->icon("setting"), tr("settings"), this));
    
    ACTIONS()->insert(PLAYING_TRACK_EDIT,new QAction(QIcon(":/images/edit-48x48.png"), tr("Edit"), this));
    ACTIONS()->insert(PLAYING_TRACK_LOVE, new QAction(QIcon(":/images/lastfm.png"), tr("Send LastFm love"), this));
    
    ACTIONS()->insert(NEW_PLAYLIST, new QAction(QIcon(":/images/add_32x32.png"),tr("new playlist"), this));
    ACTIONS()->insert(NEW_SMART_PLAYLIST, new QAction(QIcon(":/images/add_32x32.png"),tr("new smart playlist"), this));
   
    ACTIONS()->insert(TAG_CLICKED, new QAction(QIcon(),"TAG_CLICKED", this));
    
    /* player action*/
    ACTIONS()->insert(ENGINE_PLAY, new QAction( IconManager::instance()->icon( "media-play" ), tr("Play or Pause media"), this));
    ACTIONS()->insert(ENGINE_STOP, new QAction( IconManager::instance()->icon( "media-stop" ), tr("Stop playing media"), this));
    ACTIONS()->insert(ENGINE_PLAY_NEXT, new QAction( IconManager::instance()->icon( "media-next" ), tr("Play next media"), this));
    ACTIONS()->insert(ENGINE_PLAY_PREV, new QAction( IconManager::instance()->icon( "media-prev" ), tr("Play previous media"), this));
    ACTIONS()->insert(ENGINE_VOL_MUTE, new QAction(QIcon(":/images/volume-icon.png"),"", this));
    ACTIONS()->insert(ENGINE_VOL_INC, new QAction(QIcon(":/images/volume-icon.png"),"", this));
    ACTIONS()->insert(ENGINE_VOL_DEC, new QAction(QIcon(":/images/volume-icon.png"),"", this));
    ACTIONS()->insert(ENGINE_AUDIO_EQ, new QAction(IconManager::instance()->icon("equalizer"),tr("Audio equalizer"), this));

    /* database action */
    ACTIONS()->insert(DATABASE_OPERATION, new QAction(QIcon(":/images/rebuild.png"),tr("Database operation"), this));
    ACTIONS()->insert(DATABASE_ADD, new QAction(QIcon(":/images/add_32x32.png"),tr("Database add"), this));

    /* Show/hide playqueue panel */
    ACTIONS()->insert(APP_SHOW_PLAYQUEUE, new QAction(QIcon(),tr("Show playqueue panel"), this));
    ACTIONS()->value(APP_SHOW_PLAYQUEUE)->setCheckable(true);

    /* Screen mode Switch actions */
    ACTIONS()->insert(APP_MODE_COMPACT, new QAction(QIcon(":/images/screen-minimalmode.png"), tr("Switch to minimal mode"), this));
    ACTIONS()->insert(APP_MODE_NORMAL, new QAction(QIcon(":/images/screen-normalmode.png"), tr("Switch to normal mode"), this));

    /* jump to track  */
    ACTIONS()->insert(BROWSER_JUMP_TO_ARTIST, new QAction( IconManager::instance()->icon( "goto" ),tr("Jump to artist"), this));
    ACTIONS()->insert(BROWSER_JUMP_TO_ALBUM,  new QAction( IconManager::instance()->icon( "goto" ),tr("Jump to album"), this));
    ACTIONS()->insert(BROWSER_JUMP_TO_TRACK,  new QAction( IconManager::instance()->icon( "goto" ),tr("Jump to track"), this));
    ACTIONS()->insert(BROWSER_JUMP_TO_MEDIA,  new QAction( IconManager::instance()->icon( "goto" ),QString(), this));
    
    ACTIONS()->insert(APP_ENABLE_SEARCH_POPUP, new QAction(QIcon(),tr("Enable search popup"), this));
    ACTIONS()->value(APP_ENABLE_SEARCH_POPUP)->setCheckable(true);

    ACTIONS()->insert(APP_PLAY_ON_SEARCH, new QAction(QIcon(),tr("Enable play on search"), this));
    ACTIONS()->value(APP_PLAY_ON_SEARCH)->setCheckable(true);
    
    /* restore Actions states              */
    (ACTIONS()->value(APP_SHOW_PLAYQUEUE))->setChecked(SETTINGS()->_showPlayQueuePanel);
    (ACTIONS()->value(APP_ENABLE_SEARCH_POPUP))->setChecked(SETTINGS()->_enableSearchPopup);
    (ACTIONS()->value(APP_PLAY_ON_SEARCH))->setChecked(SETTINGS()->_enablePlayOnSearch);
}


//! --------- connectSlots -----------------------------------------------------
void MainWindow::connectSlots()
{
    //! Global Yarock Actions
    QObject::connect(ACTIONS()->value(APP_QUIT), SIGNAL(triggered()), SLOT(slot_on_yarock_quit()));
    QObject::connect(ACTIONS()->value(APP_SHOW_YAROCK_ABOUT), SIGNAL(triggered()), SLOT(slot_on_aboutYarock()));
    QObject::connect(ACTIONS()->value(APP_SHOW_SETTINGS), SIGNAL(triggered()), SLOT(slot_on_show_settings()));

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

    
    QObject::connect(ACTIONS()->value(ENGINE_VOL_MUTE), SIGNAL(triggered()),Engine::instance(), SLOT(volumeMute()));
    QObject::connect(ACTIONS()->value(ENGINE_VOL_INC), SIGNAL(triggered()), Engine::instance(), SLOT(volumeInc()));
    QObject::connect(ACTIONS()->value(ENGINE_VOL_DEC), SIGNAL(triggered()), Engine::instance(), SLOT(volumeDec()));    
    
    //! Connection Actions Database
    QObject::connect(ACTIONS()->value(DATABASE_OPERATION), SIGNAL(triggered()), SLOT(slot_database_ope_dialog()));
    QObject::connect(ACTIONS()->value(DATABASE_ADD), SIGNAL(triggered()), SLOT(slot_database_add_dialog()));

    //! Connection signaux COLLECTION
    QObject::connect(m_virtual_queue, SIGNAL(signal_collection_playTrack()), this, SLOT(slot_play_from_collection()));

    //! Connection signaux USER SETTINGS
    QObject::connect(m_browserView, SIGNAL(settings_saved()), SLOT(slot_on_settings_saved()));

    //! ThreadManager connection
    QObject::connect(MenuModel::instance(), SIGNAL(dbNameChanged()), this, SLOT(slot_database_start()));
    QObject::connect(ThreadManager::instance(), SIGNAL(dbBuildFinished()), this, SLOT(slot_dbBuilder_finished()));

    //! Screen mode connection
    QObject::connect(ACTIONS()->value(APP_MODE_COMPACT), SIGNAL(triggered()), SLOT(slot_widget_mode_switch()));
    QObject::connect(ACTIONS()->value(APP_MODE_NORMAL), SIGNAL(triggered()), SLOT(slot_widget_mode_switch()));
    
    // main application widget actions
    QObject::connect(ACTIONS()->value(NEW_PLAYLIST), SIGNAL(triggered()), MainRightWidget::instance(), SLOT(slot_create_new_playlist_editor()));
    QObject::connect(ACTIONS()->value(NEW_SMART_PLAYLIST), SIGNAL(triggered()), MainRightWidget::instance(), SLOT(slot_create_new_smart_editor()));
}


//! --------- Quit Actions -----------------------------------------------------
void MainWindow::slot_on_yarock_quit()
{
    Debug::debug() << "[Mainwindow] slot_on_yarock_quit";
    m_canClose = true;
    
    if( m_systray->isSysTrayOn() == true) 
    {
        m_systray->close();
    }
    
    
    this->close();
}

// void MainWindow::closeEvent(QCloseEvent *event)
// {
//     if( m_systray->isSysTrayOn() == false) 
//     {
//       this->close();
//     }
//     else /* only hide app is tray is running  */
//     {
//       if (m_systray->isVisible() && (m_canClose == false)) 
//       {
//           this->hide();
//           event->ignore();
//       }
//     }
// }


void MainWindow::resizeEvent(QResizeEvent* event)
{
   if( m_statusManager )
     m_statusManager->onResize();
   
   
    QMainWindow::resizeEvent(event);
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
    Playqueue Management
*******************************************************************************/
void MainWindow::slot_playqueue_added(QWidget* w)
{
    Debug::debug() << "[Mainwindow] slot_playqueue_added";
    
    if ( PlaylistWidgetBase *widget = dynamic_cast<PlaylistWidgetBase*>(w) )
    {
      connect(widget->view(), SIGNAL(signal_playlist_itemDoubleClicked()), this, SLOT(slot_play_from_playqueue()));
      connect(widget->model()->manager(), SIGNAL(playlistSaved()), m_thread_manager, SLOT(populateLocalPlaylistModel()));
      connect(widget->model(), SIGNAL(modelCleared()), this, SLOT(slot_playqueue_cleared()));
    }
}

void MainWindow::slot_playqueue_removed(QWidget* w)
{
    Debug::debug() << "[Mainwindow] slot_playqueue_removed";
    
    if ( PlaylistWidgetBase *widget = dynamic_cast<PlaylistWidgetBase*>(w) )
    {
      if ( widget->model() == m_playingModel )
      {
          _player->stop();
          m_playingModel  = 0;
      }
      disconnect(w, 0,this, 0);
      w->deleteLater();
    }
}

void MainWindow::slot_playqueue_cleared()
{
    Debug::debug() << "[Mainwindow] slot_playqueue_cleared";
    if ( PlayqueueModel *model = dynamic_cast<PlayqueueModel*>(sender()) )
    {        
       if ( (m_playqueue == model) && (m_playqueue == m_playingModel) )
       {
           if((_playRequestFrom == FromPlayQueue) && SETTINGS()->_stopOnPlayqueueClear) 
           {
               m_playqueue->updatePlayingItem(MEDIA::TrackPtr(0));
               m_virtual_queue->updatePlayingItem(MEDIA::TrackPtr(0));
               this->stopPlayer();
           }
       }
    }
}


/*******************************************************************************
    Player management
*******************************************************************************/
void MainWindow::slot_player_on_state_change()
{
   //Debug::debug() << "[Mainwindow] slot_player_on_state_change";
   
   ENGINE::E_ENGINE_STATE state = _player->state();

   switch (state) {
     /**************** STOPPED **********************/
     case ENGINE::STOPPED:
        set_enable_jump_to(false);
        (ACTIONS()->value(ENGINE_STOP))->setEnabled(false);
        (ACTIONS()->value(ENGINE_PLAY))->setIcon(IconManager::instance()->icon( "media-play" ));
     break;
     /**************** PLAYING ***********************/
     case ENGINE::PLAYING:
        (ACTIONS()->value(ENGINE_STOP))->setEnabled(true);
        (ACTIONS()->value(ENGINE_PLAY))->setIcon(IconManager::instance()->icon( "media-pause" ));
     break;
     /**************** PAUSE *************************/
     case ENGINE::PAUSED:
        (ACTIONS()->value(ENGINE_PLAY))->setIcon(IconManager::instance()->icon( "media-play" ));
     break;
     /**************** ERROR *************************/
     case ENGINE::ERROR:
     {
         Debug::warning() << "ENGINE error";

         m_statusManager->startMessage(tr("Playing error"), STATUS::ERROR, 5000);
         MEDIA::TrackPtr track = _player->playingTrack();
         if(track)
         {
           MEDIA::registerTrackBroken(track, true);

           if(_playRequestFrom == FromCollection)
             m_virtual_queue->updatePlayingItem(track);
           else
             playingQueue()->updatePlayingItem(track);
         }
         stopPlayer();
      }
      break;
      default:break;
    }
}


void MainWindow::slot_player_on_track_change()
{
    Debug::debug() << "[Mainwindow] slot_player_on_track_change ";
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();

    if(!track) {
      Debug::error() << "[Mainwindow] slot_player_on_track_change track ERROR";
      return;
    }
    
    /*  update playing track */
    playingQueue()->updatePlayingItem(track);
    m_virtual_queue->updatePlayingItem(track);

    /*  update jump to action */
    if ( (track->type() == TYPE_TRACK) && (track->id != -1) )
      set_enable_jump_to(true);
    else
      set_enable_jump_to(false);
    
    Debug::debug() << "[Mainwindow] slot_player_on_track_change DONE";
}

//! --------- slot_play_from_playqueue -----------------------------------------
// a file is activated from the playlist
void MainWindow::slot_play_from_playqueue()
{
    Debug::debug() << "[Mainwindow] slot_play_from_playqueue";
    
    PlaylistView* view = qobject_cast<PlaylistView*>(sender());
    
    PlaylistWidgetBase* widget = dynamic_cast<PlaylistWidgetBase*>(view->parent());
    
    m_playingModel = widget->model();
    
    _playRequestFrom = FromPlayQueue;

    const MEDIA::TrackPtr track = playingQueue()->requestedTrack();

    if(track)
      _player->setMediaItem( track );
}

//! --------- slot_play_from_collection ----------------------------------------
// a MediaItem is activated from collection browser
void MainWindow::slot_play_from_collection()
{
    //Debug::debug() << "[Mainwindow] slot_play_from_collection";
  
    _playRequestFrom = FromCollection;
    m_playingModel = 0;

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
        MEDIA::TrackPtr media;
        /* check first browser selection */
        if(m_browserView->playSelectedItems())
        {
          return;
        }
        /* check if one playlist is opened with selection */
        else if( PlaylistWidgetBase* p = MainRightWidget::instance()->activePlayqueue() )
        {
            if( p->view()->isTrackSelected() )
              media = p->view()->firstSelectedTrack();
        }
        /* check if one playlist is opened with selection */
        else
        {
            media = m_playqueue->trackAt(0);
        }

        if(media)
          _player->setMediaItem( media );
    }
}

//! --------- stopPlayer -------------------------------------------------------
void MainWindow::stopPlayer()
{
     Debug::debug() << "[Mainwindow] stopPlayer";

    _player->stop();

    m_virtual_queue->updatePlayingItem(MEDIA::TrackPtr(0));
    playingQueue()->updatePlayingItem(MEDIA::TrackPtr(0));
}

//! --------- slot_player_enqueue_next -----------------------------------------
void MainWindow::slot_player_enqueue_next()
{
    Debug::debug() << "[Mainwindow] slot_player_enqueue_next";
    MEDIA::TrackPtr media;

    if(_playRequestFrom == FromCollection)
      media = m_virtual_queue->nextTrack();
    else
      media = playingQueue()->nextTrack();

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
    Debug::debug() << "[Mainwindow] playNext requested ";
    MEDIA::TrackPtr media;

    /* check if player has already the next track */
    media = _player->nextTrack();

    /* if not check from collection or playqueue */
    if( !media )
    {
      if(_playRequestFrom == FromCollection)
        media = m_virtual_queue->nextTrack();
      else
        media = playingQueue()->nextTrack();
    }

    if(media)
      _player->setMediaItem(media);
    else
      stopPlayer();
}

//! --------- playPrev ---------------------------------------------------------
void MainWindow::playPrev()
{
    Debug::debug() << "[Mainwindow] playPrev requested ";
    
    MEDIA::TrackPtr media;

    //! We need to known if we are playing from collection or playlist
    if(_playRequestFrom == FromCollection)
      media = m_virtual_queue->prevTrack();
    else
      media = playingQueue()->prevTrack();

    if(media)
      _player->setMediaItem(media);
    else
      stopPlayer();
}

/*******************************************************************************
    Save / Restore playing track
*******************************************************************************/
void MainWindow::savePlayingTrack()
{
    //Debug::debug() << "[Mainwindow] savePlayingTrack";
    if (_player->state() == ENGINE::PLAYING)
    {
         SETTINGS()->_url      = _player->playingTrack()->url;
      
         if( MEDIA::isLocal(SETTINGS()->_url) )
         {
            SETTINGS()->_position = _player->currentTime();
            SETTINGS()->_station  = "";
         }
         else
         {
            SETTINGS()->_position = 0;
            SETTINGS()->_station  = _player->playingTrack()->extra["station"].toString();
         }
    }
}

void MainWindow::restorePlayingTrack()
{
    Debug::debug() << "[Mainwindow] restorePlayingTrack";
    const QString url = SETTINGS()->_url;
    if(url.isEmpty())
      return;

    if( MEDIA::isLocal(url) )
    {
        MEDIA::TrackPtr media = MEDIA::FromDataBase(url);
        if(!media)
          media = MEDIA::FromLocalFile( url );

        _player->setMediaItem(media);

        qint64 position = SETTINGS()->_position;
        //Debug::debug() << "[Mainwindow] restorePlayingTrack TYPE_TRACK position = " << position;
        _player->seek( position );
    }
    else
    {
        MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());
        media->setType(TYPE_STREAM);
        media->id        = -1;
        media->url       = url;
        media->extra["station"] = SETTINGS()->_station;

        _player->setMediaItem(media);
    }
}

/*******************************************************************************
    User settings methods
*******************************************************************************/
void MainWindow::slot_on_settings_saved()
{
    Debug::debug() << "[MainWindow] slot_on_settings_saved";

    SETTINGS::Results r = m_browserView->settingsResults();

    Debug::debug() << "[MainWindow] isSystrayChanged   "  << r.isSystrayChanged;
    Debug::debug() << "[MainWindow] isDbusChanged      "  << r.isDbusChanged;
    Debug::debug() << "[MainWindow] isMprisChanged     "  << r.isMprisChanged;
    Debug::debug() << "[MainWindow] isHistoryChanged   "  << r.isHistoryChanged;
    Debug::debug() << "[MainWindow] isShorcutChanged   "  << r.isShorcutChanged;
    Debug::debug() << "[MainWindow] isScrobblerChanged "  << r.isScrobblerChanged;
    Debug::debug() << "[MainWindow] isEngineChanged    "  << r.isEngineChanged;
    Debug::debug() << "[MainWindow] isLibraryChanged   "  << r.isLibraryChanged;
    Debug::debug() << "[MainWindow] isCoverSizeChanged "  << r.isCoverSizeChanged;
    Debug::debug() << "[MainWindow] isViewChanged      "  << r.isViewChanged;

    if(r.isSystrayChanged)    { m_systray->reloadSettings();}
    if(r.isDbusChanged)       { m_dbus_notifier->reloadSettings(); }
    if(r.isMprisChanged)      { m_mpris_manager->reloadSettings(); }
    if(r.isHistoryChanged)    { m_histoManager->reloadSettings(); }
    if(r.isShorcutChanged)    { m_shortcutsManager->reloadSettings();}
    if(r.isScrobblerChanged)  { LastFmService::instance()->init();}
    if(r.isLibraryChanged || r.isCoverSizeChanged)
    {
        if( r.isCoverSizeChanged )
        {
            Debug::warning() << "Database builder -> removing all existing artist and albums images !";

            /* delete all covers  */
            Q_FOREACH(QFileInfo info, QDir(UTIL::CONFIGDIR + "/albums/").entryInfoList(QDir::NoDotAndDotDot | QDir::System |
                    QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
                QFile::remove(info.absoluteFilePath());
            }

            Q_FOREACH(QFileInfo info, QDir(UTIL::CONFIGDIR + "/artists/").entryInfoList(QDir::NoDotAndDotDot | QDir::System |
                    QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
                QFile::remove(info.absoluteFilePath());
            }

            Q_FOREACH(QFileInfo info, QDir(UTIL::CONFIGDIR + "/radio/").entryInfoList(QDir::NoDotAndDotDot | QDir::System |
                    QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
                QFile::remove(info.absoluteFilePath());
            }
       }
       
      
      /* NOTE : hack pour eviter un crash de l'appli (car on ferme la connection a la 
       base de donnee puis on relance le thread database builder). Stop des thread ajoute
       avant la fermeture de la db */
      if(m_thread_manager->isDbRunning()) {
        Debug::warning() << "Database builder already running, request stop all thread!!";
        m_thread_manager->stopThread();
      }

      Database::instance()->close();

      if(!Database::instance()->exist())
        createDatabase();
      
      if(r.isCoverSizeChanged)
        createDatabase();

      // rebuild all only if cover size has been changed, otherwise it's an database update
      rebuildDatabase( r.isCoverSizeChanged );
    }
    else if (r.isViewChanged) 
    {
        m_thread_manager->populateLocalTrackModel();
    }
}



/*******************************************************************************
    Database methods
*******************************************************************************/
void MainWindow::createDatabase ()
{
    Debug::debug() << "[MainWindow] createDatabase";
    Database::instance()->close();
    Database::instance()->remove();
    Database::instance()->create();
}

void MainWindow::rebuildDatabase(bool doRebuild)
{
    Debug::debug() << "[MainWindow] rebuildDatabase doRebuild:" << doRebuild;

    QStringList listDir = QStringList() << Database::instance()->param()._paths;
    Debug::debug() << "[MainWindow] rebuildDatabase listDir" << listDir;

    // Database Building
    if (!listDir.isEmpty())
      m_thread_manager->databaseBuild(listDir,doRebuild);
}


void MainWindow::slot_database_start()
{
    Debug::debug() << "[MainWindow] slot_database_start";
    Database::instance()->close();

    //! check new database entry
    if( !Database::instance()->exist() )
    {
        /*-----------------------------------------------------------*/
        /* First start dialog                                        */
        /* ----------------------------------------------------------*/
        Debug::debug() << "[MainWindow] slot_database_start : first start dialog";

        DatabaseAddDialog dialog(this);
        dialog.setTitle( tr("Setup your music collection directory") );
        dialog.setFirstStart();
        
        if(dialog.exec() == QDialog::Accepted) 
        {
           /* create database */
           createDatabase();
        
           Debug::debug() << "[MainWindow] slot_database_start : first start dialog ACCEPTED";
           rebuildDatabase( true );
        }
        else
        {
           /* create default database */
           createDatabase();
           
           m_browserView->active_view(VIEW::ViewSettings,QString(), QVariant());
        }
    }
    else if (!Database::instance()->versionOK() )
    {
        /*-----------------------------------------------------------*/
        /* Database revision change                                  */
        /* ----------------------------------------------------------*/
        Debug::debug() << "[MainWindow] slot_database_start : database revision change";
        const QString str = tr("<p>Database need to be rebuilt</p>");

        DialogMessage dlg(this, tr("Database revision update"));
        dlg.setMessage(str);
        dlg.resize(445, 120);
        dlg.exec();      

        if( Database::instance()->version() <= 20 )
        {
            /* delete all user data (if version is <= 20 because cover size change) */
            Q_FOREACH(QFileInfo info, QDir(UTIL::CONFIGDIR + "/albums/").entryInfoList(QDir::NoDotAndDotDot | QDir::System |
                    QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
                QFile::remove(info.absoluteFilePath());
            }

            Q_FOREACH(QFileInfo info, QDir(UTIL::CONFIGDIR + "/artists/").entryInfoList(QDir::NoDotAndDotDot | QDir::System |
                    QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
                QFile::remove(info.absoluteFilePath());
            }
        }
        
        /* create database */
        createDatabase();
        rebuildDatabase( true );
    }
    else if ( Database::instance()->param()._option_auto_rebuild)
    {
        /*-----------------------------------------------------------*/
        /* Start existing database with auto update at startup       */
        /* ----------------------------------------------------------*/
        rebuildDatabase( false );
    }
    else
    {
        /*-----------------------------------------------------------*/
        /* Start existing database by populating models              */
        /* ----------------------------------------------------------*/
        m_thread_manager->populateLocalTrackModel();
        
        
        /* Restore playing track */
        if(SETTINGS()->_restartPlayingAtStartup)
            restorePlayingTrack();
    }
    
    QObject::connect(m_thread_manager, SIGNAL(modelPopulationFinished(E_MODEL_TYPE)), this, SLOT(slot_restore_playqueue()), Qt::UniqueConnection);
    
    /* process command line */
    commandlineOptionsHandle();
}


void MainWindow::slot_dbBuilder_finished()
{
    /* after db is built => repopulate local track model */
    m_thread_manager->populateLocalTrackModel();
    
    if(Database::instance()->param()._option_download_cover)
    {
        if(Database::instance()->param()._option_artist_image)
            m_thread_manager->startTagSearch(TagSearch::ARTIST_ALBUM_FULL);
        else
            m_thread_manager->startTagSearch(TagSearch::ALBUM_COVER_FULL);
    }
}


void MainWindow::slot_database_add_dialog()
{
    DatabaseAddDialog dialog(this);
    if( dialog.exec() == QDialog::Accepted)
    {
        /* create database */
        createDatabase();
        rebuildDatabase( false );
    }
}


void MainWindow::slot_database_ope_dialog()
{
    /* case 1 : remove database and rescan all collection directories */
    /* case 2 : scan directory changes and update database */
    DatabaseOperationDialog dialog(this);

    if( dialog.exec() == QDialog::Accepted)
    {
      Debug::debug() << "OPERATION = " << dialog.operation();
      switch( dialog.operation() )
      {
        case DatabaseOperationDialog::OPE_REBUILD:
          createDatabase();
          rebuildDatabase( true );
          break;
        case DatabaseOperationDialog::OPE_RESCAN:
          rebuildDatabase( false );
          break;
        case DatabaseOperationDialog::OPE_COVER:
          m_thread_manager->startTagSearch(TagSearch::ALBUM_COVER_FULL);
          break;
        case DatabaseOperationDialog::OPE_ARTIST_IMAGE:
          m_thread_manager->startTagSearch(TagSearch::ARTIST_IMAGE_FULL);
          break;
        case DatabaseOperationDialog::OPE_GENRE_TAG:
          m_thread_manager->startTagSearch(TagSearch::ALBUM_GENRE_FULL);
          break;

        default:break;
      }
    }
}


void MainWindow::slot_restore_playqueue()
{
    if( is_first_start )
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
    
    foreach(PlaylistWidgetBase* widget, MainRightWidget::instance()->playqueueList())
    {
        widget->menuActions()->value(PLAYQUEUE_JUMP_TO_TRACK)->setEnabled(b);  
    }
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
        m_statusManager->startMessage(
            QString(tr("No equalizer available with this configuration")), STATUS::ERROR, 2500
        );
    }
}

void MainWindow::slot_eq_enableChange(bool eqActivated)
{
    Debug::debug() << "[MainWindow] slot_eq_enableChange bool" << eqActivated;
    if(eqActivated) 
    {
        ACTIONS()->value(ENGINE_AUDIO_EQ)->setIcon(IconManager::instance()->icon("equalizer","selected"));

        _player->addEqualizer();
    }
    else
    {
        ACTIONS()->value(ENGINE_AUDIO_EQ)->setIcon(IconManager::instance()->icon("equalizer","normal"));        

        _player->removeEqualizer();
    }
}


void MainWindow::slot_eq_paramChange(int preamp, QList<int> listGain)
{
    Debug::debug() << "[MainWindow] slot_eq_paramChange";

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

void MainWindow::commandlineOptionsHandle()
{
    Debug::debug() << "[MainWindow] commandlineOptionsHandle";
    if (m_options.isEmpty()) {
      return;
    }

    this->showNormal();
    this->raise();     
    this->activateWindow();


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
        m_playqueue->clear(); // no break !
      case CommandlineOptions::Playlist_Append:
        if( !m_options.urls().isEmpty() )
          m_playqueue->manager()->playlistAddUrls(m_options.urls());
        break;
      
      case CommandlineOptions::Playlist_Default:
        /* new case to handle Open with yarock simple case (enqueue and play the new tracks) */
        if( !m_options.urls().isEmpty() )
        {
            m_playqueue_index = m_playqueue->size() -1;
            m_playqueue->manager()->playlistAddUrls(m_options.urls());
            connect(m_playqueue->manager(), SIGNAL(playlistPopulated()), this, SLOT(slot_play_after_playqueue_loaded()), Qt::UniqueConnection);
        }
        break;
      default:break;  
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
/* => enqueue new tracks + play first enqueued one */
void MainWindow::slot_play_after_playqueue_loaded()
{
     this->stopPlayer();
     m_playqueue->setRequestedTrackAt(m_playqueue_index + 1);
     this->slot_play_from_playqueue();
}


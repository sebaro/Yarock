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
#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

// qt
#include <QMainWindow>
#include <QSystemTrayIcon>

#include "commandlineoptions.h"
// class CommandlineOptions;

// player engine
class EngineBase;

// widgets
class PlaylistView;
class PlaylistWidget;
class BrowserView;
class CentralWidget;
class MinimalWidget;
class StatusManager;

// Model
class PlayqueueModel;
class LocalTrackModel;
class LocalPlaylistModel;
class StreamModel;
class HistoModel;


// Core
class ThreadManager;
class HistoManager;
class VirtualPlayqueue;
class GlobalShortcuts;

// Dbus & Mpris
class DbusNotification;
class MprisManager;
/*
********************************************************************************
*                                                                              *
*    Class MainWindow                                                          *
*                                                                              *
********************************************************************************
*/
class MainWindow : public QMainWindow
{
Q_OBJECT
    static MainWindow         *INSTANCE;

  public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static MainWindow* instance() { return INSTANCE; }

    void commandlineOptionsHandle();
    void set_command_line(const CommandlineOptions& options);
    
  private:
    /* Mainwindow        */
    void createActions();
    void connectSlots();

    /* Settings          */
    void reloadSystraySettings();
    void savePlayingTrack();
    void restorePlayingTrack();

    /* Database Method   */
    void createDatabase();
    void rebuildDatabase();

  private slots:

#ifdef TEST_FLAG    
    /* Debug */
    void slot_start_test();
#endif
    /* Mainwindow        */
    void slot_systray_clicked(QSystemTrayIcon::ActivationReason);
    void slot_widget_mode_switch();
    void slot_commandline_received(const QByteArray& serialized_options);
    void slot_on_settings_saved();
    void slot_on_aboutYarock();
    void slot_on_yarock_quit();
    void slot_on_show_settings();

    /* Playqueue        */
    void slot_playqueue_add();
    void slot_playqueue_clear();
    void slot_playqueue_save();
    void slot_playqueue_save_auto();
    void slot_restore_playqueue();
    void slot_play_after_playqueue_loaded();

    /*  Player           */
    void slot_player_enqueue_next();
    void slot_player_on_state_change();
    void slot_play_from_playqueue();
    void slot_play_from_collection();
    void slot_player_on_track_change ();

    void playOrPause();
    void stopPlayer();
    void playNext();
    void playPrev();

    /* Database slots    */
    void slot_database_start();
    void slot_database_dialog();

    /* Jump to item slot */
    void set_enable_jump_to(bool b);

    /* Equalizer         */
    void slot_eq_openDialog();
    void slot_eq_enableChange(bool eqActivated);
    void slot_eq_paramChange(int, QList<int>);

  protected:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent* event);

  private:
    bool                 is_first_start;

    enum E_PLAYING_FROM   {FromCollection, FromPlayQueue};

    E_PLAYING_FROM        _playRequestFrom;

    EngineBase            *_player;

    StatusManager         *_statusManager;

    CentralWidget         *_centralWidget;

    PlaylistView          *_playlistView;
    
    PlayqueueModel        *m_playqueue;

    PlaylistWidget        *_playlistwidget;

    LocalTrackModel       *m_localTrackModel;
    LocalPlaylistModel    *m_localPlaylistModel;
    HistoModel            *m_histoModel;
    BrowserView           *m_browserView;

    VirtualPlayqueue      *m_virtual_queue;

    StreamModel           *m_streamModel;

    ThreadManager         *m_thread_manager;

    QSystemTrayIcon        *m_trayIcon;
    bool                    m_canClose;

    MinimalWidget          *m_minimalwidget;
    HistoManager           *m_histoManager;

    GlobalShortcuts        *m_globalShortcuts;
    DbusNotification       *m_dbus_notifier;
    MprisManager           *m_mpris_manager;

    CommandlineOptions     m_options;
    
    int                    m_playqueue_index;

signals:
    void playlistFinished();
};

#endif // _MAINWINDOW_H_


cmake_policy(SET CMP0011 NEW)

# ------------------------------------------------------------------------------
#  SOURCES
# ------------------------------------------------------------------------------
SET(YAROCK_INCLUDES  ${CMAKE_SOURCE_DIR}/src/
		     ${CMAKE_SOURCE_DIR}/src/core
                     ${CMAKE_SOURCE_DIR}/src/core/database
                     ${CMAKE_SOURCE_DIR}/src/core/history
                     ${CMAKE_SOURCE_DIR}/src/core/mediaitem
                     ${CMAKE_SOURCE_DIR}/src/core/mediasearch
                     ${CMAKE_SOURCE_DIR}/src/core/player
                     ${CMAKE_SOURCE_DIR}/src/core/player/vlc
                     ${CMAKE_SOURCE_DIR}/src/covers
                     ${CMAKE_SOURCE_DIR}/src/infosystem
                     ${CMAKE_SOURCE_DIR}/src/infosystem/services
                     ${CMAKE_SOURCE_DIR}/src/infosystem/tasks
                     ${CMAKE_SOURCE_DIR}/src/dbus
                     ${CMAKE_SOURCE_DIR}/src/models
                     ${CMAKE_SOURCE_DIR}/src/models/local
                     ${CMAKE_SOURCE_DIR}/src/models/online
                     ${CMAKE_SOURCE_DIR}/src/models/stream   
                     ${CMAKE_SOURCE_DIR}/src/models/filesystem                     
                     ${CMAKE_SOURCE_DIR}/src/online
                     ${CMAKE_SOURCE_DIR}/src/playqueue
                     ${CMAKE_SOURCE_DIR}/src/settings
                     ${CMAKE_SOURCE_DIR}/src/smartplaylist
                     ${CMAKE_SOURCE_DIR}/src/views
                     ${CMAKE_SOURCE_DIR}/src/views/about
                     ${CMAKE_SOURCE_DIR}/src/views/context
                     ${CMAKE_SOURCE_DIR}/src/views/filesystem
                     ${CMAKE_SOURCE_DIR}/src/views/local
                     ${CMAKE_SOURCE_DIR}/src/views/settings
                     ${CMAKE_SOURCE_DIR}/src/views/stream
                     ${CMAKE_SOURCE_DIR}/src/widgets
                     ${CMAKE_SOURCE_DIR}/src/widgets/dialogs
                     ${CMAKE_SOURCE_DIR}/src/widgets/equalizer
                     ${CMAKE_SOURCE_DIR}/src/widgets/nowplaying
                     ${CMAKE_SOURCE_DIR}/src/widgets/main
                     ${CMAKE_SOURCE_DIR}/src/widgets/popupcompleter
)

SET(SOURCES 
           ${CMAKE_SOURCE_DIR}/src/global_actions.cpp
           ${CMAKE_SOURCE_DIR}/src/commandlineoptions.cpp
           ${CMAKE_SOURCE_DIR}/src/debug.cpp
           ${CMAKE_SOURCE_DIR}/src/main.cpp
           ${CMAKE_SOURCE_DIR}/src/mainwindow.cpp
           ${CMAKE_SOURCE_DIR}/src/networkaccess.cpp           
           ${CMAKE_SOURCE_DIR}/src/global_shortcuts.cpp
           ${CMAKE_SOURCE_DIR}/src/settings.cpp
           ${CMAKE_SOURCE_DIR}/src/threadmanager.cpp
           ${CMAKE_SOURCE_DIR}/src/utilities.cpp
)

set (SOURCES 
           ${SOURCES}
           ${CMAKE_SOURCE_DIR}/src/core/database/database.cpp
           ${CMAKE_SOURCE_DIR}/src/core/database/database_cmd.cpp
           ${CMAKE_SOURCE_DIR}/src/core/database/databasebuilder.cpp
           ${CMAKE_SOURCE_DIR}/src/core/history/histomanager.cpp
           ${CMAKE_SOURCE_DIR}/src/core/history/histomanager.cpp

           ${CMAKE_SOURCE_DIR}/src/core/mediaitem/mediaitem.cpp
           ${CMAKE_SOURCE_DIR}/src/core/mediaitem/mediamimedata.cpp
           ${CMAKE_SOURCE_DIR}/src/core/mediaitem/playlist_parser.cpp
           ${CMAKE_SOURCE_DIR}/src/core/mediaitem/tag.cpp
           
           ${CMAKE_SOURCE_DIR}/src/core/mediasearch/media_search.cpp
           ${CMAKE_SOURCE_DIR}/src/core/mediasearch/media_search_engine.cpp
           ${CMAKE_SOURCE_DIR}/src/core/mediasearch/search_query.cpp
           ${CMAKE_SOURCE_DIR}/src/core/mediasearch/search_query_widget.cpp

           ${CMAKE_SOURCE_DIR}/src/core/player/engine.cpp
           ${CMAKE_SOURCE_DIR}/src/core/player/engine_base.cpp
           ${CMAKE_SOURCE_DIR}/src/core/player/engine_phonon.cpp
           ${CMAKE_SOURCE_DIR}/src/core/player/engine_vlc.cpp
           ${CMAKE_SOURCE_DIR}/src/core/player/vlc/vlc_lib.cpp
           ${CMAKE_SOURCE_DIR}/src/core/player/vlc/vlc_media.cpp
)

set (SOURCES 
           ${SOURCES}           
           ${CMAKE_SOURCE_DIR}/src/covers/covercache.cpp
)           

set (SOURCES 
           ${SOURCES}           
           ${CMAKE_SOURCE_DIR}/src/dbus/dbusnotification.cpp           
           ${CMAKE_SOURCE_DIR}/src/dbus/mpris_manager.cpp
           ${CMAKE_SOURCE_DIR}/src/dbus/mpris2.cpp        
)    

set (SOURCES 
           ${SOURCES}           
           ${CMAKE_SOURCE_DIR}/src/models/local/local_track_model.cpp
           ${CMAKE_SOURCE_DIR}/src/models/local/local_track_populator.cpp
           ${CMAKE_SOURCE_DIR}/src/models/local/histo_model.cpp
           ${CMAKE_SOURCE_DIR}/src/models/local/local_playlist_model.cpp
           ${CMAKE_SOURCE_DIR}/src/models/local/local_playlist_populator.cpp
           ${CMAKE_SOURCE_DIR}/src/models/stream/service_base.cpp
           ${CMAKE_SOURCE_DIR}/src/models/stream/service_shoutcast.cpp
           ${CMAKE_SOURCE_DIR}/src/models/stream/service_tunein.cpp
           ${CMAKE_SOURCE_DIR}/src/models/stream/service_xspf.cpp
           ${CMAKE_SOURCE_DIR}/src/models/stream/service_dirble.cpp           
           ${CMAKE_SOURCE_DIR}/src/models/stream/stream_model.cpp 
           ${CMAKE_SOURCE_DIR}/src/models/filesystem/file_model.cpp
)

set (SOURCES 
           ${SOURCES}           
           ${CMAKE_SOURCE_DIR}/src/online/lastfm.cpp
)
   
set (SOURCES 
           ${SOURCES}           
           ${CMAKE_SOURCE_DIR}/src/playqueue/playqueue_base.cpp
           ${CMAKE_SOURCE_DIR}/src/playqueue/playlistdbwriter.cpp
           ${CMAKE_SOURCE_DIR}/src/playqueue/playqueue_model.cpp
           ${CMAKE_SOURCE_DIR}/src/playqueue/playqueue_proxymodel.cpp
           ${CMAKE_SOURCE_DIR}/src/playqueue/playlistpopulator.cpp
           ${CMAKE_SOURCE_DIR}/src/playqueue/playlistview.cpp
           ${CMAKE_SOURCE_DIR}/src/playqueue/playlistwidget.cpp
           ${CMAKE_SOURCE_DIR}/src/playqueue/playlistwriter.cpp
           ${CMAKE_SOURCE_DIR}/src/playqueue/task_manager.cpp
           ${CMAKE_SOURCE_DIR}/src/playqueue/virtual_playqueue.cpp                
)

   
set (SOURCES 
           ${SOURCES}           
           ${CMAKE_SOURCE_DIR}/src/smartplaylist/smartplaylist.cpp        
)
      
set (SOURCES 
           ${SOURCES}                            
           ${CMAKE_SOURCE_DIR}/src/views/browser_view.cpp           
           ${CMAKE_SOURCE_DIR}/src/views/item_button.cpp
           ${CMAKE_SOURCE_DIR}/src/views/item_common.cpp
           ${CMAKE_SOURCE_DIR}/src/views/item_menu.cpp           
           ${CMAKE_SOURCE_DIR}/src/views/scene_base.cpp           
           ${CMAKE_SOURCE_DIR}/src/views/about/about_scene.cpp
           ${CMAKE_SOURCE_DIR}/src/views/local/qpixmapfilter.cpp
           ${CMAKE_SOURCE_DIR}/src/views/local/local_item.cpp
           ${CMAKE_SOURCE_DIR}/src/views/local/local_scene.cpp
           ${CMAKE_SOURCE_DIR}/src/views/local/local_scene_dashboard.cpp
           ${CMAKE_SOURCE_DIR}/src/views/local/local_scene_playing.cpp
           ${CMAKE_SOURCE_DIR}/src/views/local/local_scene_playqueue.cpp
           ${CMAKE_SOURCE_DIR}/src/views/context/context_item.cpp
           ${CMAKE_SOURCE_DIR}/src/views/context/context_widget.cpp
           ${CMAKE_SOURCE_DIR}/src/views/context/context_scene.cpp
           ${CMAKE_SOURCE_DIR}/src/views/context/lyricseditor.cpp
           ${CMAKE_SOURCE_DIR}/src/views/stream/stream_item.cpp
           ${CMAKE_SOURCE_DIR}/src/views/stream/stream_scene.cpp
           ${CMAKE_SOURCE_DIR}/src/views/stream/stream_loader.cpp           
           ${CMAKE_SOURCE_DIR}/src/views/filesystem/file_scene.cpp
           ${CMAKE_SOURCE_DIR}/src/views/filesystem/file_graphicitem.cpp
           ${CMAKE_SOURCE_DIR}/src/views/settings/settings_scene.cpp           
           ${CMAKE_SOURCE_DIR}/src/views/settings/settings_widget.cpp
           ${CMAKE_SOURCE_DIR}/src/views/settings/addfolder_widget.cpp
)
                
set (SOURCES 
           ${SOURCES}           
           ${CMAKE_SOURCE_DIR}/src/widgets/dialogs/dialog_base.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/dialogs/addstreamdialog.cpp           
           ${CMAKE_SOURCE_DIR}/src/widgets/dialogs/dboperationdialog.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/dialogs/shortcutdialog.cpp           
           ${CMAKE_SOURCE_DIR}/src/widgets/dialogs/filedialog.cpp           
           ${CMAKE_SOURCE_DIR}/src/widgets/dialogs/first_time_dialog.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_album.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_artist.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_track.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_common.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_playlist.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_smart.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_search.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/main/centralwidget.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/main/main_right.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/main/main_left.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/main/maintoolbutton.cpp           
           ${CMAKE_SOURCE_DIR}/src/widgets/main/menumodel.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/main/menuwidget.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/main/menubar.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/nowplaying/nowplayingview.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/popupcompleter/popup_item.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/popupcompleter/popup_model.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/popupcompleter/popup_view.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/popupcompleter/search_popup.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/audiocontrols.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/customsplitter.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/equalizer/equalizer_dialog.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/equalizer/equalizer_preset.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/equalizer/equalizer_slider.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/exlineedit.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/searchline_edit.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/minimalwidget.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/playertoolbar.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/ratingwidget.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/seekslider.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/spacer.cpp
           ${CMAKE_SOURCE_DIR}/src/widgets/statuswidget.cpp           
           ${CMAKE_SOURCE_DIR}/src/widgets/sort_widget.cpp            
           ${CMAKE_SOURCE_DIR}/src/widgets/header_widget.cpp            
)

set (SOURCES 
           ${SOURCES}           
           ${CMAKE_SOURCE_DIR}/src/infosystem/info_service.cpp
           ${CMAKE_SOURCE_DIR}/src/infosystem/info_system.cpp
           ${CMAKE_SOURCE_DIR}/src/infosystem/info_system_worker.cpp           
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/service_discogs.cpp
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/service_echonest.cpp
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/service_lastfm.cpp
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/service_musicbrainz.cpp
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/service_lyrics.cpp
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/ultimatelyricsprovider.cpp
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/ultimatelyricsreader.cpp
           ${CMAKE_SOURCE_DIR}/src/infosystem/tasks/tagsearch.cpp           
)

# ------------------------------------------------------------------------------
#  YAROCK_HEADERS
# ------------------------------------------------------------------------------
SET(YAROCK_HEADERS 
           ${CMAKE_SOURCE_DIR}/src/commandlineoptions.h
           ${CMAKE_SOURCE_DIR}/src/debug.h
           ${CMAKE_SOURCE_DIR}/src/global_actions.h
           ${CMAKE_SOURCE_DIR}/src/global_shortcuts.h
           ${CMAKE_SOURCE_DIR}/src/main.h
           ${CMAKE_SOURCE_DIR}/src/mainwindow.h
           ${CMAKE_SOURCE_DIR}/src/networkaccess.h           
           ${CMAKE_SOURCE_DIR}/src/settings.h
           ${CMAKE_SOURCE_DIR}/src/threadmanager.h
           ${CMAKE_SOURCE_DIR}/src/utilities.h
           ${CMAKE_SOURCE_DIR}/src/views.h
           ${CMAKE_SOURCE_DIR}/src/core/database/database.h
           ${CMAKE_SOURCE_DIR}/src/core/database/database_cmd.h
           ${CMAKE_SOURCE_DIR}/src/core/database/databasebuilder.h
           ${CMAKE_SOURCE_DIR}/src/core/history/histomanager.h
           ${CMAKE_SOURCE_DIR}/src/core/history/histomanager.h
           ${CMAKE_SOURCE_DIR}/src/core/mediaitem/mediaitem.h
           ${CMAKE_SOURCE_DIR}/src/core/mediaitem/mediaitem_replaygain.h
           ${CMAKE_SOURCE_DIR}/src/core/mediaitem/mediaitem_statistic.h
           ${CMAKE_SOURCE_DIR}/src/core/mediaitem/mediamimedata.h
           ${CMAKE_SOURCE_DIR}/src/core/mediaitem/playlist_parser.h
           ${CMAKE_SOURCE_DIR}/src/core/mediaitem/tag.h
           ${CMAKE_SOURCE_DIR}/src/core/mediasearch/media_search.h
           ${CMAKE_SOURCE_DIR}/src/core/mediasearch/media_search_engine.h
           ${CMAKE_SOURCE_DIR}/src/core/mediasearch/search_query.h
           ${CMAKE_SOURCE_DIR}/src/core/mediasearch/search_query_widget.h
           ${CMAKE_SOURCE_DIR}/src/core/player/engine.h
           ${CMAKE_SOURCE_DIR}/src/core/player/engine_base.h
           ${CMAKE_SOURCE_DIR}/src/core/player/engine_phonon.h
           ${CMAKE_SOURCE_DIR}/src/core/player/engine_vlc.h
           ${CMAKE_SOURCE_DIR}/src/core/player/vlc/vlc_lib.h
           ${CMAKE_SOURCE_DIR}/src/core/player/vlc/vlc_media.h
           ${CMAKE_SOURCE_DIR}/src/covers/covercache.h
           ${CMAKE_SOURCE_DIR}/src/dbus/dbusnotification.h           
           ${CMAKE_SOURCE_DIR}/src/dbus/mpris2.h   
           ${CMAKE_SOURCE_DIR}/src/infosystem/info_service.h
           ${CMAKE_SOURCE_DIR}/src/infosystem/info_system.h
           ${CMAKE_SOURCE_DIR}/src/infosystem/info_system_worker.h
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/service_discogs.h
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/service_echonest.h
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/service_lastfm.h
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/service_musicbrainz.h           
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/service_lyrics.h  
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/ultimatelyricsprovider.h
           ${CMAKE_SOURCE_DIR}/src/infosystem/services/ultimatelyricsreader.h
           ${CMAKE_SOURCE_DIR}/src/infosystem/tasks/tagsearch.h
           ${CMAKE_SOURCE_DIR}/src/models/local/local_track_model.h
           ${CMAKE_SOURCE_DIR}/src/models/local/local_track_populator.h
           ${CMAKE_SOURCE_DIR}/src/models/local/histo_model.h
           ${CMAKE_SOURCE_DIR}/src/models/local/local_playlist_model.h
           ${CMAKE_SOURCE_DIR}/src/models/local/local_playlist_populator.h
           ${CMAKE_SOURCE_DIR}/src/models/stream/service_base.h
           ${CMAKE_SOURCE_DIR}/src/models/stream/service_shoutcast.h
           ${CMAKE_SOURCE_DIR}/src/models/stream/service_tunein.h
           ${CMAKE_SOURCE_DIR}/src/models/stream/service_xspf.h
           ${CMAKE_SOURCE_DIR}/src/models/stream/service_dirble.h
           ${CMAKE_SOURCE_DIR}/src/models/stream/stream_model.h           
           ${CMAKE_SOURCE_DIR}/src/models/filesystem/file_model.h
           ${CMAKE_SOURCE_DIR}/src/online/lastfm.h
           ${CMAKE_SOURCE_DIR}/src/playqueue/playqueue_base.h
           ${CMAKE_SOURCE_DIR}/src/playqueue/playqueue_model.h
           ${CMAKE_SOURCE_DIR}/src/playqueue/playlistdbwriter.h
           ${CMAKE_SOURCE_DIR}/src/playqueue/playlistpopulator.h
           ${CMAKE_SOURCE_DIR}/src/playqueue/playqueue_proxymodel.h
           ${CMAKE_SOURCE_DIR}/src/playqueue/playlistview.h
           ${CMAKE_SOURCE_DIR}/src/playqueue/playlistwidget.h
           ${CMAKE_SOURCE_DIR}/src/playqueue/playlistwriter.h
           ${CMAKE_SOURCE_DIR}/src/playqueue/task_manager.h  
           ${CMAKE_SOURCE_DIR}/src/playqueue/virtual_playqueue.h  
           ${CMAKE_SOURCE_DIR}/src/smartplaylist/smartplaylist.h        
           ${CMAKE_SOURCE_DIR}/src/views/item_common.cpp
           ${CMAKE_SOURCE_DIR}/src/views/item_button.h           
           ${CMAKE_SOURCE_DIR}/src/views/item_menu.h
           ${CMAKE_SOURCE_DIR}/src/views/browser_view.h           
           ${CMAKE_SOURCE_DIR}/src/views/scene_base.h           
           ${CMAKE_SOURCE_DIR}/src/views/about/about_scene.h
           ${CMAKE_SOURCE_DIR}/src/views/local/qpixmapfilter.h
           ${CMAKE_SOURCE_DIR}/src/views/local/local_item.h
           ${CMAKE_SOURCE_DIR}/src/views/local/local_scene.h
           ${CMAKE_SOURCE_DIR}/src/views/local/local_scene_dashboard.h
           ${CMAKE_SOURCE_DIR}/src/views/local/local_scene_playing.h
           ${CMAKE_SOURCE_DIR}/src/views/local/local_scene_playqueue.h
           ${CMAKE_SOURCE_DIR}/src/views/local/local_scene_rate.h
           ${CMAKE_SOURCE_DIR}/src/views/context/context_item.h
           ${CMAKE_SOURCE_DIR}/src/views/context/context_widget.h
           ${CMAKE_SOURCE_DIR}/src/views/context/context_scene.h
           ${CMAKE_SOURCE_DIR}/src/views/context/lyricseditor.h
           ${CMAKE_SOURCE_DIR}/src/views/stream/stream_item.h
           ${CMAKE_SOURCE_DIR}/src/views/stream/stream_scene.h
           ${CMAKE_SOURCE_DIR}/src/views/stream/stream_loader.h
           ${CMAKE_SOURCE_DIR}/src/views/filesystem/file_scene.h
           ${CMAKE_SOURCE_DIR}/src/views/filesystem/file_graphicitem.h  
           ${CMAKE_SOURCE_DIR}/src/views/settings/settings_scene.h           
           ${CMAKE_SOURCE_DIR}/src/views/settings/settings_widget.h             
           ${CMAKE_SOURCE_DIR}/src/views/settings/addfolder_widget.h
           ${CMAKE_SOURCE_DIR}/src/widgets/dialogs/dialog_base.h
           ${CMAKE_SOURCE_DIR}/src/widgets/dialogs/addstreamdialog.h           
           ${CMAKE_SOURCE_DIR}/src/widgets/dialogs/dboperationdialog.h
           ${CMAKE_SOURCE_DIR}/src/widgets/dialogs/shortcutdialog.h           
           ${CMAKE_SOURCE_DIR}/src/widgets/dialogs/filedialog.h
           ${CMAKE_SOURCE_DIR}/src/widgets/dialogs/first_time_dialog.h
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_album.h
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_artist.h
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_track.h
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_common.h
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_playlist.h
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_smart.h
           ${CMAKE_SOURCE_DIR}/src/widgets/editors/editor_search.h
           ${CMAKE_SOURCE_DIR}/src/widgets/main/centralwidget.h
           ${CMAKE_SOURCE_DIR}/src/widgets/main/main_right.h
           ${CMAKE_SOURCE_DIR}/src/widgets/main/main_left.h
           ${CMAKE_SOURCE_DIR}/src/widgets/main/maintoolbutton.h           
           ${CMAKE_SOURCE_DIR}/src/widgets/main/menumodel.h
           ${CMAKE_SOURCE_DIR}/src/widgets/main/menuwidget.h
           ${CMAKE_SOURCE_DIR}/src/widgets/main/menubar.h
           ${CMAKE_SOURCE_DIR}/src/widgets/nowplaying/nowplayingview.h
           ${CMAKE_SOURCE_DIR}/src/widgets/popupcompleter/popup_item.h
           ${CMAKE_SOURCE_DIR}/src/widgets/popupcompleter/popup_model.h
           ${CMAKE_SOURCE_DIR}/src/widgets/popupcompleter/popup_view.h
           ${CMAKE_SOURCE_DIR}/src/widgets/popupcompleter/search_popup.h
           ${CMAKE_SOURCE_DIR}/src/widgets/audiocontrols.h
           ${CMAKE_SOURCE_DIR}/src/widgets/customsplitter.h
           ${CMAKE_SOURCE_DIR}/src/widgets/equalizer/equalizer_dialog.h
           ${CMAKE_SOURCE_DIR}/src/widgets/equalizer/equalizer_preset.h
           ${CMAKE_SOURCE_DIR}/src/widgets/equalizer/equalizer_slider.h
           ${CMAKE_SOURCE_DIR}/src/widgets/exlineedit.h
           ${CMAKE_SOURCE_DIR}/src/widgets/searchline_edit.h
           ${CMAKE_SOURCE_DIR}/src/widgets/minimalwidget.h
           ${CMAKE_SOURCE_DIR}/src/widgets/playertoolbar.h
           ${CMAKE_SOURCE_DIR}/src/widgets/ratingwidget.h
           ${CMAKE_SOURCE_DIR}/src/widgets/seekslider.h
           ${CMAKE_SOURCE_DIR}/src/widgets/spacer.h
           ${CMAKE_SOURCE_DIR}/src/widgets/statuswidget.h           
           ${CMAKE_SOURCE_DIR}/src/widgets/sort_widget.h  
           ${CMAKE_SOURCE_DIR}/src/widgets/header_widget.h
)

     
set (YAROCK_SOURCES ${SOURCES} PARENT_SCOPE)
set (YAROCK_HEADERS ${YAROCK_HEADERS} PARENT_SCOPE)
set (YAROCK_INCLUDES ${YAROCK_INCLUDES} PARENT_SCOPE)      
      

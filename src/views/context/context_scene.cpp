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

#include "context_scene.h"

#include "core/player/engine.h"

/*   info providers   */
#include "info_system.h"


#include "settings.h"
#include "utilities.h"
#include "debug.h"

#include <QGraphicsView>
/*
********************************************************************************
*                                                                              *
*    Class ContextScene                                                        *
*                                                                              *
********************************************************************************
*/
ContextScene::ContextScene(QWidget* parent) : SceneBase(parent)
{
}

/*******************************************************************************
    initScene
*******************************************************************************/
void ContextScene::initScene()
{
    //! set engine player for update
    m_engine_player = Engine::instance();
    connect(m_engine_player, SIGNAL(mediaChanged()), this, SLOT(slot_filter_metadata_change()));
    connect(m_engine_player, SIGNAL(engineStateChanged()), this, SLOT(slot_filter_enginestate_change()));
    connect(m_engine_player, SIGNAL(mediaMetaDataChanged()), this, SLOT(populateScene()));

    connect( InfoSystem::instance(),
                SIGNAL( info( INFO::InfoRequestData, QVariant ) ),
                SLOT( infoSystemInfo( INFO::InfoRequestData, QVariant ) ), Qt::UniqueConnection );    

    /* init contexte scene */
    m_mode = ContextScene::Stopped;
    
    //!  Playing scene widgets
    m_artist_info_widget  = new ArtistInfoWidget(parentView());
    m_similar_info_widget = new ArtistSimilarWidget(parentView());
    m_disco_info_widget   = new DiscoInfoWidget(parentView());
    m_album_info_widget   = new AlbumInfoWidget( parentView() );
    m_lyrics_info_widget  = new LyricsInfoWidget( parentView() );

    connect(m_artist_info_widget, SIGNAL(updated()), this, SLOT(slot_update_draw()));
    connect(m_similar_info_widget, SIGNAL(updated()), this, SLOT(slot_update_draw()));
    connect(m_disco_info_widget, SIGNAL(updated()), this, SLOT(slot_update_draw()));
    connect(m_album_info_widget, SIGNAL(updated()), this, SLOT(slot_update_draw()));
    connect(m_lyrics_info_widget, SIGNAL(updated()), this, SLOT(slot_update_draw()));

    m_artist_info_widget->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    m_similar_info_widget->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    m_disco_info_widget->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    m_album_info_widget->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    m_lyrics_info_widget->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

    
    //! now playing widget
    m_nowplaying_info_widget = new NowPlayingInfoWidget(parentView());
    connect(m_engine_player, SIGNAL(mediaMetaDataChanged()), m_nowplaying_info_widget, SLOT(update()));
    m_nowplaying_info_widget->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

    //! layout 
    m_layout    = new QGraphicsLinearLayout(Qt::Vertical);
    m_layout->setSpacing(20);
    m_layout->addItem(m_nowplaying_info_widget);    

    m_container = new QGraphicsWidget();
    m_container->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    m_container->setLayout(m_layout);
    m_container->setPos(0,5);

    m_mode = Stopped;
    this->addItem(m_container);

    init_widget(Stopped);
      
    setInit(true);
}


/*******************************************************************************
  init_widget
*******************************************************************************/
void ContextScene::init_widget(Mode new_mode)
{
    if(m_mode == new_mode)
      return;
    
    m_mode = new_mode;
    
    if( m_mode == StreamPlay ) 
    {
      Debug::debug() << "ContextScene init_widget StreamPlay";
      m_nowplaying_info_widget->hide();
      m_album_info_widget->hide();
      m_lyrics_info_widget->show();
      m_artist_info_widget->show();
      m_similar_info_widget->show();
      m_disco_info_widget->show();      
      
      m_layout->removeItem(m_nowplaying_info_widget);
      m_layout->removeItem(m_album_info_widget);

      m_layout->addItem(m_artist_info_widget);
      m_layout->addItem(m_similar_info_widget);
      m_layout->addItem(m_disco_info_widget);
      m_layout->addItem(m_lyrics_info_widget);
    }
    else if( m_mode == TrackPlay ) 
    {
      Debug::debug() << "ContextScene init_widget TrackPlay";
      
      m_nowplaying_info_widget->hide();
      m_artist_info_widget->show();
      m_similar_info_widget->show();
      m_disco_info_widget->show();
      m_album_info_widget->show();
      m_lyrics_info_widget->show(); 
      
      m_layout->removeItem(m_nowplaying_info_widget);      
      m_layout->addItem(m_artist_info_widget);
      m_layout->addItem(m_similar_info_widget);
      m_layout->addItem(m_disco_info_widget);
      m_layout->addItem(m_album_info_widget);
      m_layout->addItem(m_lyrics_info_widget);
    }
    else if ( m_mode ==  Stopped ) 
    {
      Debug::debug() << "ContextScene init_widget Stopped";
      
      m_artist_info_widget->hide();
      m_similar_info_widget->hide();
      m_disco_info_widget->hide();
      m_album_info_widget->hide();
      m_lyrics_info_widget->hide();  
      m_nowplaying_info_widget->show();

      m_layout->removeItem(m_artist_info_widget);
      m_layout->removeItem(m_similar_info_widget);
      m_layout->removeItem(m_disco_info_widget);
      m_layout->removeItem(m_album_info_widget);
      m_layout->removeItem(m_lyrics_info_widget);
      m_layout->addItem(m_nowplaying_info_widget);
    }
}

/*******************************************************************************
    resizeScene
*******************************************************************************/
void ContextScene::resizeScene()
{
    //Debug::debug() << "   [ContextScene] resizeScene ";

    slot_update_draw();
} 

/*******************************************************************************
  ContextScene::slot_filter_metadata_change
*******************************************************************************/
void ContextScene::slot_filter_metadata_change()
{
     Debug::debug () << "   [ContextScene] slot_filter_metadata_change " << m_engine_player->stateToString(m_engine_player->state());
     /* vlc and gstreamer send first state change to PLAYING then metadatachange signal */
     /* not working for Mplayer phonon backend (first metadat after STOPPED state is lost) */  
     if( m_engine_player->state() == ENGINE::PLAYING )
       populateScene();
}

void ContextScene::slot_filter_enginestate_change()
{
    if(m_engine_player->playingTrack() && !MEDIA::isLocal(m_engine_player->playingTrack()->url) )
      return;
  
    populateScene();
}


/*******************************************************************************
  ContextScene::populateScene
*******************************************************************************/
void ContextScene::populateScene()
{
    if(SETTINGS()->_viewMode != VIEW::ViewContext)
      return;
    
    Debug::debug() << "   [ContextScene] populateScene ";

    /*-------------------------------------------------*/
    /*  PLAYING state                                  */
    /* ------------------------------------------------*/
    if(m_engine_player->state() == ENGINE::PLAYING && m_engine_player->playingTrack())
    {
      /* set widget */
      init_widget(m_engine_player->playingTrack()->type() == TYPE_TRACK ? TrackPlay : StreamPlay);

      /* init request for InfoSystem */
      m_requests_ids.clear();
      QList<INFO::InfoRequestData> requests;
      
      INFO::InfoStringHash hash;
      hash["artist"] = m_engine_player->playingTrack()->artist;
      hash["album"]  = m_engine_player->playingTrack()->album;
      hash["title"]  = m_engine_player->playingTrack()->title;
      
      if(m_metadata["ARTIST"] != m_engine_player->playingTrack()->artist) 
      {
          Debug::warning() << "   [ContextScene] new artist";

          m_artist_info_widget->clear();
          m_similar_info_widget->clear();
          m_disco_info_widget->clear();
  
          if(!hash["artist"].isEmpty()) 
          {
            requests << INFO::InfoRequestData(INFO::InfoArtistReleases, hash);
            requests << INFO::InfoRequestData(INFO::InfoArtistBiography, hash);
            requests << INFO::InfoRequestData(INFO::InfoArtistImages, hash);
            requests << INFO::InfoRequestData(INFO::InfoArtistSimilars, hash);

            m_artist_info_widget->set_artist_name( hash["artist"] );
          }
          else
          {
            m_artist_info_widget->hide();
            m_similar_info_widget->hide();
            m_disco_info_widget->hide();
          }
      }

      if( (m_metadata["ALBUM"] != m_engine_player->playingTrack()->album)
        && (m_mode != StreamPlay) ) 
      {
          Debug::warning() << "   [ContextScene] new album";
          requests << INFO::InfoRequestData(INFO::InfoAlbumInfo, hash);
          requests << INFO::InfoRequestData(INFO::InfoAlbumCoverArt, hash);

          m_album_info_widget->clear();
          m_album_info_widget->set_track( m_engine_player->playingTrack() );
      }

      if(m_metadata["TITLE"] != m_engine_player->playingTrack()->title) 
      {
          Debug::warning() << "   [ContextScene] new track";
          requests << INFO::InfoRequestData(INFO::InfoTrackLyrics, hash);

          m_lyrics_info_widget->clear();

          if(!hash["title"].isEmpty()) 
          {
            m_lyrics_info_widget->set_song_name( m_engine_player->playingTrack()->title );
          }
          else
          {
            m_lyrics_info_widget->hide();
          }
      }

      /* trigger info system request */
      foreach(INFO::InfoRequestData request, requests)
      {
         m_requests_ids << request.requestId;         
         InfoSystem::instance()->getInfo( request );
      }
      
      /* register new metadata */
      m_metadata["ARTIST"] = m_engine_player->playingTrack()->artist;
      m_metadata["ALBUM"]  = m_engine_player->playingTrack()->album;
      m_metadata["TITLE"]  = m_engine_player->playingTrack()->title;      
    }
    /*-------------------------------------------------*/
    /*  STOPPED state                                  */
    /* ------------------------------------------------*/
    else if(m_engine_player->state() == ENGINE::STOPPED && !m_engine_player->playingTrack())
    {
      //Debug::warning() << "   [ContextScene] ENGINE::STOPPED";
      init_widget(Stopped);

      m_metadata.clear();
    }
    
    slot_update_draw();
}

void ContextScene::infoSystemInfo(INFO::InfoRequestData request, QVariant output )
{
    if(!m_requests_ids.contains(request.requestId))
      return;

    //Debug::debug() << Q_FUNC_INFO << " request.type" << request.type;
    
    m_requests_ids.removeOne(request.requestId);
    
    QList<INFO::InfoRequestData> requests;    

    if (request.type == INFO::InfoArtistBiography)
    {
        m_artist_info_widget->setData(request, output);
    }          
    else if (request.type == INFO::InfoArtistImages )
    {
        INFO::InfoStringHash input = request.data.value< INFO::InfoStringHash >();
        
        if( input.contains("#uri") )
          m_similar_info_widget->setData(request, output);
        else
          m_artist_info_widget->setData(request, output);
    }
    else if (request.type == INFO::InfoArtistSimilars)
    {
        m_similar_info_widget->setData(request, output);
        
        /* relancer une recherche pour les images des artistes similaires */
        foreach (QVariant artist , output.toList())
        {
            QVariantMap map =  qvariant_cast<QVariantMap>(artist);
          
            INFO::InfoStringHash hash;
            hash["artist"]     = map.value("name").toString();

            if(map.contains("uri"))
              hash["#uri"]   = map.value("uri").toString();
          
            requests << INFO::InfoRequestData(INFO::InfoArtistImages, hash);           
        }  
    }
    else if(request.type == INFO::InfoArtistReleases ) 
    {
        m_disco_info_widget->setData(request, output);
        
        QVariantMap vmap =  qvariant_cast<QVariantMap>(output);

        QVariantList releases = vmap.value("releases").toList();
       
        /* request for discography album cover */
        foreach(QVariant release, releases)
        {
            QVariantMap map =  qvariant_cast<QVariantMap>(release);

            INFO::InfoStringHash hash;
            hash["artist"] = map.value("artist").toString();
            hash["album"]  = map.value("album").toString();

            //hash["#uri"]   = map.value("uri").toString();
            if(map.contains("mbid"))
              hash["#mbid"]  = map.value("mbid").toString();
            
            requests << INFO::InfoRequestData(INFO::InfoAlbumCoverArt, hash);            
        }
   }
   else if (request.type == INFO::InfoAlbumCoverArt )
   {
       m_album_info_widget->setData(request, output);
       m_disco_info_widget->setData(request, output);
   }
   else if (request.type == INFO::InfoTrackLyrics )
   {
       m_lyrics_info_widget->setData(request, output);
   }     
   else if (request.type == INFO::InfoAlbumInfo)
   {
       m_album_info_widget->setData(request, output);
   }


    /* trigger info system request */
    foreach(INFO::InfoRequestData request, requests)
    {
       m_requests_ids << request.requestId;         
       InfoSystem::instance()->getInfo( request );
    }
}




/*******************************************************************************
  ContextScene::slot_update_draw
*******************************************************************************/
void ContextScene::slot_update_draw()
{
    //Debug::debug() << "   [ContextScene] slot_update_draw ";
    m_layout->invalidate();
    m_container->update();

    /* we need to ajust SceneRect */
    setSceneRect ( itemsBoundingRect().adjusted(0, -10, 0, 40) );
}


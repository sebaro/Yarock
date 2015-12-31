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

#include "nowplayingpopup.h"
#include "core/player/engine.h"
#include "core/mediaitem/mediaitem.h"
#include "core/database/database_cmd.h"
#include "covers/covercache.h"
#include "widgets/ratingwidget.h"

#include "widgets/editors/editor_track.h"
#include "widgets/main/main_right.h"
#include "widgets/spacer.h"
#include "widgets/tagwidget.h"
#include "widgets/flowlayout.h"

#include "playqueue/playqueue_model.h"

#include "mainwindow.h"

#include "online/lastfm.h"

#include "utilities.h"
#include "global_actions.h"
#include "debug.h"

#include <QPainter>
#include <QApplication>
#include <QPalette>

#include <QtGui>

#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#else
#include <QtCore>
#endif


NowPlayingPopup* NowPlayingPopup::INSTANCE = 0;
/*
********************************************************************************
*                                                                              *
*    Class NowPlayingPopup                                                     *
*                                                                              *
********************************************************************************
*/
NowPlayingPopup::NowPlayingPopup(QWidget *parent) : QWidget(parent)
{
    INSTANCE = this;
    m_playertoolbar = parent;
    m_mainwindow = MainWindow::instance();
    setParent( m_mainwindow );

    this->setWindowFlags(Qt::Popup);

    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Window));
    this->setPalette(palette);

    this->setAutoFillBackground(true);
    this->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    
    /* ----- UI ----- */
    ui_rating          = new RatingWidget();
    ui_rating->set_draw_frame( false );
    ui_rating->setMaximumWidth(75);
 
    ui_image           = new QLabel(this);
    ui_image->setAlignment(Qt::AlignCenter);
    ui_image->setFixedHeight (120);
    ui_image->setFixedWidth (120);
    ui_image->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    ui_artist_image    = new QLabel(this);
    ui_artist_image->setAlignment(Qt::AlignCenter);
    ui_artist_image->setFixedHeight (120);
    ui_artist_image->setFixedWidth (120);
    ui_artist_image->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    
    ui_btn_station = new QPushButton(this);
    ui_btn_station->setFocusPolicy( Qt::NoFocus );
    ui_btn_station->setFlat(true);
    ui_btn_station->setIcon(QIcon(":/images/media-url-18x18.png"));
      
    ui_btn_title  = new QPushButton(this);
    ui_btn_title->setFocusPolicy( Qt::NoFocus );
    ui_btn_title->setFlat(true);
    ui_btn_title->setIcon(QIcon(":/images/track-18x18.png"));

    ui_btn_artist  = new QPushButton(this);
    ui_btn_artist->setFocusPolicy( Qt::NoFocus );
    ui_btn_artist->setFlat(true);
    ui_btn_artist->setIcon(QIcon(":/images/view-artist_18x18.png"));      

    ui_btn_audio = new QPushButton(this);
    ui_btn_audio->setFocusPolicy( Qt::NoFocus );
    ui_btn_audio->setFlat(true);
    ui_btn_audio->setIcon(QIcon(":/images/files-18x18.png"));    
    ui_btn_audio->setEnabled(false);
    
    ui_btn_station->setStyleSheet("QPushButton { margin: 1px; padding: 0px; }");
    ui_btn_title->setStyleSheet("QPushButton { margin: 1px; padding: 0px; }");
    ui_btn_artist->setStyleSheet("QPushButton { margin: 1px; padding: 0px; }");
    ui_btn_audio->setStyleSheet("QPushButton { margin: 1px; padding: 0px; }");
      
    QFont font1 = QApplication::font();
    font1.setPointSize(font1.pointSize()*1.2);
    ui_btn_station->setFont( font1 );
    ui_btn_title->setFont( font1 );
    ui_btn_artist->setFont( font1 );
    ui_btn_audio->setFont( font1 );
      

    /* toolbar actions */
    ui_toolbar = new QToolBar(this);
    ui_toolbar->setOrientation(Qt::Vertical);
    ui_toolbar->setIconSize( QSize( 14, 14 ) );
    ui_toolbar->addAction( ACTIONS()->value(BROWSER_JUMP_TO_TRACK) );
    ui_toolbar->addAction( ACTIONS()->value(PLAYING_TRACK_EDIT)  );
    ui_toolbar->addAction( ACTIONS()->value(PLAYING_TRACK_LOVE) );

    QAction* PLAYING_TRACK_ADD_QUEUE = new QAction(QIcon(":/images/media-playlist-48x48.png"), tr("Add to play queue"), 0);
    ui_toolbar->addAction( PLAYING_TRACK_ADD_QUEUE );

    QAction* PLAYING_TRACK_FAVORITE = new QAction(QIcon(":/images/favorites-48x48.png"), tr("Add to favorites"), 0);
    ui_toolbar->addAction( PLAYING_TRACK_FAVORITE );

    
    ui_tags_widget = new QWidget();
    ui_tags_widget->setLayout(new FlowLayout(5,0,0,0,6,4));

    QVBoxLayout* cover_layout = new QVBoxLayout();
    cover_layout->setContentsMargins(0, 0, 0, 0);
    cover_layout->setSpacing(2);    
    cover_layout->addWidget( ui_rating );
    cover_layout->addWidget( ui_image );
    cover_layout->setAlignment(ui_rating, Qt::AlignTop | Qt::AlignCenter);

    QVBoxLayout* labels_layout = new QVBoxLayout();
    labels_layout->setContentsMargins(0, 0, 0, 0);
    labels_layout->setSpacing(2);
    labels_layout->addItem(new QSpacerItem(2, 2, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    labels_layout->addWidget( ui_btn_station );
    labels_layout->addWidget( ui_btn_title );
    labels_layout->addWidget( ui_btn_artist );
    labels_layout->addWidget( ui_btn_audio );
    labels_layout->addWidget( ui_tags_widget );
    labels_layout->addItem(new QSpacerItem(2, 2, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    
    labels_layout->setAlignment(ui_btn_station, Qt::AlignLeft);
    labels_layout->setAlignment(ui_btn_title, Qt::AlignLeft);
    labels_layout->setAlignment(ui_btn_artist, Qt::AlignLeft);
    labels_layout->setAlignment(ui_btn_audio, Qt::AlignLeft);
    labels_layout->setAlignment(ui_tags_widget, Qt::AlignVCenter);
    

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget( new FixedSpacer(this, QSize(5,0)));
    layout->addWidget( ui_toolbar );
    layout->addWidget( new FixedSpacer(this, QSize(5,0)));
    layout->addLayout( cover_layout );
    layout->addWidget( new FixedSpacer(this, QSize(10,0)));
    layout->addLayout( labels_layout );
    layout->addItem( new QSpacerItem(2, 2, QSizePolicy::Expanding, QSizePolicy::MinimumExpanding));
    layout->addWidget( ui_artist_image );
    layout->addWidget( new FixedSpacer(this, QSize(10,0)));

    /* ----- connection ----- */
    connect(ui_rating, SIGNAL(RatingChanged(float)), this, SLOT(slot_rating_changed(float)));
     
    connect(ACTIONS()->value(PLAYING_TRACK_LOVE), SIGNAL(triggered()), this, SLOT(slot_on_lastfm_love()));
    connect(ACTIONS()->value(PLAYING_TRACK_EDIT), SIGNAL(triggered()), this, SLOT(slot_on_track_edit()));
    connect(PLAYING_TRACK_ADD_QUEUE, SIGNAL(triggered()), this, SLOT(slot_on_track_add_to_queue()));
    connect(PLAYING_TRACK_FAVORITE, SIGNAL(triggered()), this, SLOT(slot_on_add_to_favorite()));
    

    connect(ui_btn_station, SIGNAL(clicked()), this, SLOT(slot_on_websearch()));
    connect(ui_btn_title, SIGNAL(clicked()), this, SLOT(slot_on_websearch()));
    connect(ui_btn_artist, SIGNAL(clicked()), this, SLOT(slot_on_websearch()));
    
    
    /* ----- init ----- */
    m_track = 0;
}



void NowPlayingPopup::resizeEvent(QResizeEvent* event)
{
    Debug::debug() << "  [NowPlayingPopup] resizeEvent";

    QWidget::resizeEvent(event);
   
    QPoint location = m_mainwindow->mapToGlobal(
        QPoint(3,m_mainwindow->height() - m_playertoolbar->height() - this->height() - 4 /*padding*/)
    );
    this->move( location );
}


/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::updateWidget                                              */
/* ---------------------------------------------------------------------------*/ 
void NowPlayingPopup::updateWidget()
{
    //Debug::debug() << "  [NowPlayingPopup] updateWidget";

    /* ----- update content ----- */
    if( m_track != Engine::instance()->playingTrack() )
    {
        m_track = Engine::instance()->playingTrack();
        
        /* update actions */
        ACTIONS()->value(PLAYING_TRACK_EDIT)->setEnabled( m_track->id != -1 );
        ACTIONS()->value(PLAYING_TRACK_LOVE)->setEnabled( LastFmService::instance()->isAuthenticated() );
    
        /* update image DEFAULT SIZE IS 120x120 */
        QPixmap pix = CoverCache::instance()->cover( m_track );
        ui_image->setPixmap( pix );
        
        /* audio properties */
        setAudioProperties();
        
        /* -------- TRACK ---------- */
        if(m_track->type() == TYPE_TRACK)
        {
            ui_btn_station->hide();
            
            if(m_track->id != -1 )
            {
              ui_rating->set_rating( m_track->rating );
              ui_rating->set_user_rating( true );
              ui_rating->set_enable( true );
              ui_rating->show();
            }
            else
            {
                ui_rating->set_enable( false );
                ui_rating->hide();
            }
        }
        /* -------- STREAM --------- */
        else
        {
            ui_btn_station->show();
            ui_btn_station->setText( m_track->extra["station"].toString() );

            ui_rating->set_enable( false );
            ui_rating->hide();
        }
    }
    
    
    if( m_metadata["ARTIST"] != m_track->artist )
    {
        ui_artist_image->clear();
        
        slot_get_artist_image();
        
        slot_download_artists_tags();
    }
    
    /* ----- ALWAYS update title - album - artist label ----- */
    const int width = m_mainwindow->width() - 120*2-35;
    
    const QString artist = m_track->album.isEmpty() ? m_track->artist : m_track->artist + " - " + m_track->album;

    QString clippedText = QFontMetrics(ui_btn_artist->font()).elidedText(artist, Qt::ElideRight, width);
    ui_btn_artist->setText(clippedText);


    QString title_or_url = m_track->title.isEmpty() ? m_track->url : m_track->title;

    if(m_track->type() == TYPE_STREAM)
      title_or_url = m_track->title.isEmpty() ? m_track->extra["station"].toString() : m_track->title;

    clippedText = QFontMetrics(ui_btn_title->font()).elidedText(title_or_url, Qt::ElideRight, width);
    ui_btn_title->setText( clippedText );
    

    /* ----- move - resize - show popup ----- */
    QPoint location = m_mainwindow->mapToGlobal(
        QPoint(3,m_mainwindow->height() - m_playertoolbar->height() - this->height() - 4 /*padding*/)
    );
    this->move( location );
    this->show();
    this->raise();
    this->resize(m_mainwindow->width() -6, this->height() );
    
    /* ----- register new metadata ----- */
    m_metadata["ARTIST"] = Engine::instance()->playingTrack()->artist;
    m_metadata["ALBUM"]  = Engine::instance()->playingTrack()->album;
    m_metadata["TITLE"]  = Engine::instance()->playingTrack()->title;
}


/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::slot_rating_changed                                       */
/* ---------------------------------------------------------------------------*/ 
void NowPlayingPopup::slot_rating_changed(float)
{
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();
    if( track )
    {
        /* apply rating */
        track->rating = ui_rating->rating();

        /* save change to database */
        QList<MEDIA::MediaPtr> medias;
        medias << track;
        QtConcurrent::run(DatabaseCmd::rateMediaItems, medias);
    }
}


/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::slot_on_lastfm_love                                       */
/* ---------------------------------------------------------------------------*/ 
void NowPlayingPopup::slot_on_lastfm_love()
{
    //Debug::debug() << "NowPlayingPopup::slot_on_lastfm_love";
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();
    
    if( track  )
      LastFmService::instance()->love(track);
}


/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::slot_on_track_edit                                        */
/* ---------------------------------------------------------------------------*/ 
void NowPlayingPopup::slot_on_track_edit()
{
    MainRightWidget::instance()->addWidget(new EditorTrack( m_track ));
}

/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::slot_on_track_add_to_queue                                */
/* ---------------------------------------------------------------------------*/ 
void NowPlayingPopup::slot_on_track_add_to_queue()
{
    Playqueue::instance()->addMediaItem( m_track );
}


/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::slot_get_artist_image                                     */
/* ---------------------------------------------------------------------------*/ 
void NowPlayingPopup::slot_get_artist_image()
{
    Debug::debug() << "   [NowPlayingPopup] slot_get_artist_image";
    
    /* track from collection */
    if( m_track->id != -1 )
    {
      if( MEDIA::MediaPtr album = m_track->parent() )
      {
        if( MEDIA::MediaPtr artist = album->parent() )
        {
          QPixmap pix = CoverCache::instance()->image( artist );
          ui_artist_image->setPixmap( pix );
        }
      } 
    }
    else
    {
        if( !Engine::instance()->playingTrack()->artist.isEmpty() )
            slot_download_image();
    }
}


/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::slot_load_image_from_file                                 */
/* ---------------------------------------------------------------------------*/
void NowPlayingPopup::slot_download_image()
{
    Debug::debug() << "   [NowPlayingPopup] slot_download_image";
    INFO::InfoStringHash hash;
    
    hash["artist"]                = Engine::instance()->playingTrack()->artist;
    INFO::InfoRequestData request = INFO::InfoRequestData(INFO::InfoArtistImages, hash);

    m_requests_ids << request.requestId;  
    
    connect( InfoSystem::instance(),
                SIGNAL( info( INFO::InfoRequestData, QVariant ) ),
                SLOT( slot_on_infosystem_received( INFO::InfoRequestData, QVariant ) ), Qt::UniqueConnection );   
    
    /* start system info */
    InfoSystem::instance()->getInfo( request );
}

/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::slot_load_image_from_file                                 */
/* ---------------------------------------------------------------------------*/
void NowPlayingPopup::slot_download_artists_tags()
{
    Debug::debug() << "   [NowPlayingPopup] slot_download_image";
    INFO::InfoStringHash hash;

    hash["artist"]                  = Engine::instance()->playingTrack()->artist;
    INFO::InfoRequestData request   = INFO::InfoRequestData(INFO::InfoArtistTerms, hash);    

    m_requests_ids << request.requestId;  
    
    connect( InfoSystem::instance(),
                SIGNAL( info( INFO::InfoRequestData, QVariant ) ),
                SLOT( slot_on_infosystem_received( INFO::InfoRequestData, QVariant ) ), Qt::UniqueConnection );   
    
    /* start system info */
    InfoSystem::instance()->getInfo( request );
}

/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::slot_on_infosystem_received                               */
/* ---------------------------------------------------------------------------*/
void NowPlayingPopup::slot_on_infosystem_received(INFO::InfoRequestData request, QVariant output)
{
    Debug::debug() << "   [NowPlayingPopup] slot_on_infosystem_received";
    /* check request */
    if(!m_requests_ids.contains(request.requestId))
      return;    
    
    m_requests_ids.removeOne(request.requestId);

    /* get data from request */  
    if ( request.type == INFO::InfoArtistImages )
    {
        QImage image = UTIL::artistImageFromByteArray( output.toByteArray() );
    
        if( image.isNull() )
          return;

        ui_artist_image->setPixmap( QPixmap::fromImage(image) );
        ui_artist_image->update();
    }
    else if ( request.type == INFO::InfoArtistTerms )
    {
        foreach (QWidget * child, ui_tags_widget->findChildren<QWidget*>()) 
          delete child;
        
        QVariantList list = output.toList();
        foreach(QVariant tag, list)
        {
           TagWidget *tagWidget = new TagWidget(tag.toString(), ui_tags_widget);
            
           ui_tags_widget->layout()->addWidget( tagWidget );
          
           connect(tagWidget, SIGNAL(Clicked()), SLOT(slot_on_tag_clicked()));
        }
    }    
    /* disconnect info system */
    if ( m_requests_ids.isEmpty() )
    {
       disconnect( InfoSystem::instance(),
                   SIGNAL( info( INFO::InfoRequestData, QVariant ) ),
                   this, SLOT( slot_on_infosystem_received( INFO::InfoRequestData, QVariant ) ) );        
    }        
}

/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::slot_on_tag_clicked                                       */
/* ---------------------------------------------------------------------------*/
void NowPlayingPopup::slot_on_tag_clicked()
{
    Debug::debug() << "   [NowPlayingPopup] slot_on_tag_clicked";
    
    TagWidget *tag = qobject_cast<TagWidget *>(sender());

    ACTIONS()->value(TAG_CLICKED)->setData(tag->text());
    
    ACTIONS()->value(TAG_CLICKED)->trigger();
}

/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::slot_on_websearch                                         */
/* ---------------------------------------------------------------------------*/
void NowPlayingPopup::slot_on_websearch()
{
    Debug::debug() << "   [NowPlayingPopup] slot_on_websearch";
    
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    
    if( button == ui_btn_station )
    {
      if( !m_track->extra["website"].toString().isEmpty() )
        QDesktopServices::openUrl( QUrl(m_track->extra["website"].toString()) );
    }
    else if (button == ui_btn_title)
    {
        QDesktopServices::openUrl( 
            QUrl( QString("http://google.com/search?q=%1").arg(m_track->artist + m_track->title) )
        );
    }
    else if (button == ui_btn_artist)
    {
        QDesktopServices::openUrl( 
            QUrl( QString("http://google.com/search?q=%1").arg(m_track->artist) )
        );
    }
}
    
    

/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::slot_on_add_to_favorite                                   */
/* ---------------------------------------------------------------------------*/
void NowPlayingPopup::slot_on_add_to_favorite()
{
    if( m_track->type() == TYPE_TRACK && m_track->id != -1 )
    {
        MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast( m_track->parent() );

        album->isFavorite = !album->isFavorite;

        DatabaseCmd::updateFavorite(album, album->isFavorite);
    }
    else if( m_track->type() == TYPE_STREAM )
    {
        MEDIA::TrackPtr parent = MEDIA::TrackPtr::staticCast( m_track->parent() );
        
        if(parent && parent->type() == TYPE_STREAM)
          DatabaseCmd::addStreamToFavorite( parent );
        else
          DatabaseCmd::addStreamToFavorite( m_track );
    }
}

/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::setAudioProperties                                        */
/* ---------------------------------------------------------------------------*/
void NowPlayingPopup::setAudioProperties()
{
    QString info = "";

    if( m_track->type() == TYPE_TRACK && m_track->id != -1 )
    {
        MEDIA::ExtraFromDataBase(m_track);
              
        info = QFileInfo(m_track->url).suffix().toLower();
        info += " - ";
        info += m_track->extra["bitrate"].toString().isEmpty() ? "?" : m_track->extra["bitrate"].toString();
        info += "kbps - ";
        info += m_track->extra["samplerate"].toString().isEmpty() ? "?" : m_track->extra["samplerate"].toString();
        info += "Hz";
    }
    else if( m_track->type() == TYPE_STREAM )
    {
        info = m_track->extra["format"].toString().isEmpty() ? "?" : m_track->extra["format"].toString();
        info += " - ";
        info += m_track->extra["bitrate"].toString().isEmpty() ? "?" : m_track->extra["bitrate"].toString();
        info += "kbps - ";
        info += m_track->extra["samplerate"].toString().isEmpty() ? "?" : m_track->extra["samplerate"].toString();
        info += "Hz";
    }

    ui_btn_audio->setText( info );
}




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

// local
#include "playertoolbar.h"
#include "volumebutton.h"
#include "nowplayingpopup.h"
#include "audiocontrols.h"
#include "seekslider.h"
#include "toolbuttonbase.h"

#include "widgets/spacer.h"
#include "covers/covercache.h"

#include "settings.h"
#include "core/player/engine.h"
#include "utilities.h"
#include "global_actions.h"
#include "debug.h"

// Qt
#include <QLayout>
#include <QWidgetAction>
#include <QtCore>

/*
********************************************************************************
*                                                                              *
*    Class PlayerToolBar                                                       *
*                                                                              *
********************************************************************************
*/
PlayerToolBar* PlayerToolBar::INSTANCE = 0;

PlayerToolBar::PlayerToolBar(QWidget *parent) : QWidget( parent )
{
    INSTANCE = this;

    m_player = Engine::instance();

    this->setObjectName(QString::fromUtf8("playerToolBar"));
    this->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

    
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    this->setPalette(palette);
    this->setAutoFillBackground(true);
    
    /* -- cover, title, album -- */
      ui_image           = new QLabel();
      ui_image->setAlignment(Qt::AlignCenter);
      ui_image->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    
    
      QFont font1 = QApplication::font();
      font1.setWeight(87);
      font1.setPointSize(font1.pointSize()*1.1);

      ui_label_title     = new QLabel();
      ui_label_title->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );      
      ui_label_title->setFont( font1 );
      ui_label_title->setAlignment(Qt::AlignBottom);

      QFont font2 = QApplication::font();
      font2.setPointSize(font2.pointSize()*1.1);

      ui_label_album     = new QLabel();
      ui_label_album->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );      
      ui_label_album->setFont( font2 );
      ui_label_album->setAlignment(Qt::AlignTop);
    
      ui_collection_info = new QLabel();
      ui_collection_info->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );      
      ui_collection_info->setFont( font2 );
      ui_collection_info->setAlignment(Qt::AlignVCenter);
      
      
      QVBoxLayout* vl1 = new QVBoxLayout();
      vl1->setContentsMargins(0, 0, 0, 0);
      vl1->setSpacing(0);
      //vl1->addSpacerItem ( new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding) );
      vl1->addWidget( ui_label_title , Qt::AlignVCenter | Qt::AlignLeft);
      vl1->addWidget( ui_collection_info, Qt::AlignVCenter | Qt::AlignLeft);      
      vl1->addWidget( ui_label_album , Qt::AlignVCenter | Qt::AlignLeft);
      //vl1->addSpacerItem ( new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding) );
    
      QHBoxLayout* hl1 = new QHBoxLayout();
      hl1->setContentsMargins(0, 0, 0, 0);
      hl1->setSpacing(8);
      hl1->addWidget( ui_image );
      hl1->addLayout( vl1 );

      /* popup & now playing widget */
      m_popup = 0;
      
      m_now_playing_widget = new QWidget( this );
      m_now_playing_widget->setLayout( hl1 );
      m_now_playing_widget->setMinimumHeight(60);
      m_now_playing_widget->installEventFilter(this);
      m_now_playing_widget->setFocusPolicy( Qt::ClickFocus );
      
      QColor color = SETTINGS()->_baseColor;
      qreal saturation = color.saturationF();
      saturation *= 0.5;
      color.setHsvF( color.hueF(), saturation, color.valueF(), color.alphaF() );
        
      m_now_playing_widget->setStyleSheet(
          QString("QWidget::focus { border: none; background-color:%1;}")
          .arg(color.name())
        ); 
    
    /* -- time track position -- */
      QFont font = QApplication::font();
      font.setPointSize(font.pointSize()*1.4);
    
      m_currentTime = new QLabel(this);
      m_currentTime->setFont(font);
      m_currentTime->setAlignment( Qt::AlignLeft );
      m_currentTime->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    
      m_totalTime = new QLabel(this);
      m_totalTime->setFont(font);
      m_totalTime->setAlignment( Qt::AlignRight );
      m_totalTime->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );

      m_separator = new QLabel(this);
      m_separator->setFont(font);
      m_separator->setAlignment( Qt::AlignRight );
      m_separator->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
      m_separator->setText("/");
      m_separator->hide();
    
      m_pauseState = new QLabel(this);
      m_pauseState->setFont(font);
      m_pauseState->setAlignment( Qt::AlignRight );
      m_pauseState->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
      m_pauseState->setText(tr("[paused]"));
      m_pauseState->hide();
    
      QPalette p = m_pauseState->palette();
      p.setColor(m_pauseState->foregroundRole(), SETTINGS()->_baseColor);
      m_pauseState->setPalette(p);  

      QHBoxLayout *trackTimeLayout = new QHBoxLayout();
      trackTimeLayout->setContentsMargins( 0, 0, 0, 0);
      trackTimeLayout->setSpacing(1);
      trackTimeLayout->setMargin(0);
      trackTimeLayout->addWidget( m_pauseState, 0, Qt::AlignVCenter | Qt::AlignLeft);
      trackTimeLayout->addWidget( new FixedSpacer(this, QSize(10,0)) );
      trackTimeLayout->addWidget( m_currentTime, 0, Qt::AlignVCenter | Qt::AlignRight);
      trackTimeLayout->addWidget( m_separator, 0, Qt::AlignVCenter | Qt::AlignRight);
      trackTimeLayout->addWidget( m_totalTime, 0, Qt::AlignVCenter | Qt::AlignRight);

    
    /* -- ToolButton creation -- */
    ToolButtonBase* ui_prev_button = new ToolButtonBase(this);
    ui_prev_button->setDefaultAction(ACTIONS()->value(ENGINE_PLAY_PREV));
    ui_prev_button->setIconSize( QSize( 32, 32 ) );
    
    ToolButtonBase* ui_next_button = new ToolButtonBase(this);
    ui_next_button->setDefaultAction(ACTIONS()->value(ENGINE_PLAY_NEXT));
    ui_next_button->setIconSize( QSize( 32, 32 ) );

    ToolButtonBase* ui_play_button = new ToolButtonBase(this);
    ui_play_button->setDefaultAction(ACTIONS()->value(ENGINE_PLAY));
    ui_play_button->setIconSize( QSize( 32, 32 ) );

    ToolButtonBase* ui_stop_button = new ToolButtonBase(this);
    ui_stop_button->setDefaultAction(ACTIONS()->value(ENGINE_STOP));
    ui_stop_button->setIconSize( QSize( 32, 32 ) );

    ToolButtonBase* ui_equalizer_button = new ToolButtonBase(this);
    ui_equalizer_button->setDefaultAction(ACTIONS()->value(ENGINE_AUDIO_EQ));
    ui_equalizer_button->setIconSize( QSize( 32, 32 ) );
    
 
    /* -- tool button layout -- */
    QGridLayout* gl = new QGridLayout();
    gl->setContentsMargins(0, 0, 0, 0);
    gl->addWidget(new FixedSpacer(this, QSize(5,0)) , 0, 0);
    gl->addWidget( new VolumeButton(this), 0, 1) ;
    gl->addWidget( ui_equalizer_button, 0, 2) ;
    gl->addWidget( new FixedSpacer(this, QSize(10,0)), 0,3 );
    gl->addWidget( m_now_playing_widget , 0,4);
    
    gl->addWidget( ui_prev_button, 0, 5) ;
    gl->addWidget( ui_play_button, 0, 6) ;
    gl->addWidget( ui_stop_button, 0, 7) ;
    gl->addWidget( ui_next_button, 0, 8) ;
    gl->addLayout( trackTimeLayout, 0, 9, Qt::AlignRight) ;
    gl->addWidget( new FixedSpacer(this, QSize(10,0)), 0,10 ,Qt::AlignRight);
    gl->addWidget( new RepeatControl(this) , 0, 11, Qt::AlignRight);
    gl->addWidget( new ShuffleControl(this), 0, 12, Qt::AlignRight);    
    gl->addWidget( new FixedSpacer(this, QSize(5,0)) , 0, 13,Qt::AlignRight);
    
    gl->setColumnStretch( 4, 1);
    gl->setColumnStretch( 9, 1);
    
    /* -- widget layout -- */
      QVBoxLayout *layout = new QVBoxLayout(this);
      layout->setContentsMargins( 0, 0, 0, 0 );
      layout->setSpacing(0);
      layout->addWidget(new SeekSlider(this));
      layout->addLayout(gl);
    
    /* -- signals connection -- */
    connect(this->m_player, SIGNAL(mediaTick(qint64)), this, SLOT(slot_update_time_position(qint64)));
    connect(this->m_player, SIGNAL(mediaMetaDataChanged()), this, SLOT(slot_update_track_playing_info()));
    connect(this->m_player, SIGNAL(mediaChanged()), this, SLOT(slot_update_track_playing_info()));
    connect(this->m_player, SIGNAL(engineStateChanged()), this, SLOT(slot_update_track_playing_info()));
}


void PlayerToolBar::clear()
{
    //Debug::debug() << "      [PlayerToolBar] clear";
    ui_image->clear();
    ui_label_album->clear();
    ui_label_title->clear();
    
    m_currentTime->clear();
    m_totalTime->clear();
    m_pauseState->hide();
    m_separator->hide();
}


bool PlayerToolBar::eventFilter(QObject* obj, QEvent* event)
{
    //Debug::debug() << "      [PlayerToolBar] eventFilter";

    QWidget *m = qobject_cast<QWidget*>(obj);
        
    if (m && (m == m_now_playing_widget) )
    {
        if (event->type() == QEvent::Enter)
        {
             if(m_player->state() != ENGINE::STOPPED && m_player->playingTrack()) 
             {
                 m_now_playing_widget->setCursor(Qt::PointingHandCursor);
                 m_now_playing_widget->setFocus();
             }
             
             m_now_playing_widget->update();
        }
        else if (event->type() == QEvent::Leave)
        {
           m_now_playing_widget->clearFocus();
           m_now_playing_widget->unsetCursor();
           m_now_playing_widget->update();
        }
        else if (event->type() == QEvent::MouseButtonPress)
        {
            if(m_player->state() != ENGINE::STOPPED && m_player->playingTrack())
            {
                if( !m_popup )
                    m_popup = new NowPlayingPopup(this);
                
                m_popup->updateWidget();
                m_now_playing_widget->clearFocus();
            }

            return true;
        }
    }

    return QObject::eventFilter (obj, event);
}



void PlayerToolBar::slot_update_track_playing_info()
{
    //Debug::debug() << "      [PlayerToolBar] slot_update_track_playing_info";
    MEDIA::TrackPtr track = m_player->playingTrack();

    /* update now playing widget */     
    if(m_player->state() != ENGINE::STOPPED && track)
    {
        ui_collection_info->hide();
        
        /* ----- update total time for current track ----- */
        slot_update_total_time( m_player->currentTotalTime() );

        if( m_player->state() == ENGINE::PAUSED )
          m_pauseState->show();
        else
          m_pauseState->hide();
    
        // Debug::debug() << "## Now playing TITLE :" << track->title;
        // Debug::debug() << "## Now playing NAME  :" << track->name;
        // Debug::debug() << "## Now playing URL   :" << track->url;
        // Debug::debug() << "## Now playing GENRE :" << track->genre;

        /* ----- update image ----- */
        QPixmap pix = CoverCache::instance()->cover(track);
        QPixmap newpix = pix.scaled(QSize(60,60), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);      
        ui_image->setPixmap( newpix );

        /* ----- update tracks title/album/artist ----- */
        QString title_or_url = track->title.isEmpty() ? track->url : track->title;
        if(track->type() == TYPE_STREAM)
            title_or_url = track->title.isEmpty() ? track->extra["station"].toString() : track->title;

        const int width = m_now_playing_widget->width() - 70;

        QString clippedText = QFontMetrics(ui_label_title->font()).elidedText(title_or_url, Qt::ElideRight, width);

        ui_label_title->setText( clippedText );

        const QString album = track->album.isEmpty() ? track->artist : track->artist + " - " + track->album;

        clippedText = QFontMetrics(ui_label_album->font()).elidedText(album, Qt::ElideRight, width);

        ui_label_album->setText ( clippedText );

        /* ----- update now playing popup */
        if(m_popup && m_popup->isVisible())
            m_popup->updateWidget();
    }
    else
    {
        this->clear();
        ui_collection_info->show();

        switch( m_mode )
        {
            case (VIEW::ViewAlbum)    :       ui_image->setPixmap( QPixmap(":/images/album.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewArtist)   :       ui_image->setPixmap( QPixmap(":/images/view-artist.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewTrack)    :       ui_image->setPixmap( QPixmap(":/images/track-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewGenre)    :       ui_image->setPixmap( QPixmap(":/images/genre.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewYear)     :       ui_image->setPixmap( QPixmap(":/images/date-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewFavorite) :       ui_image->setPixmap( QPixmap(":/images/favorites-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewPlaylist) :       ui_image->setPixmap( QPixmap(":/images/media-playlist-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewSmartPlaylist) :  ui_image->setPixmap( QPixmap(":/images/smart-playlist-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewDirble)        :  ui_image->setPixmap( QPixmap(":/images/dirble.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewRadionomy)     :  ui_image->setPixmap( QPixmap(":/images/radionomy.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewTuneIn)        :  ui_image->setPixmap( QPixmap(":/images/tunein_48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewFavoriteRadio) :  ui_image->setPixmap( QPixmap(":/images/favorites-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;

            default:ui_image->clear();break;   
        }
        
        /* ----- update now playing popup */
        if(m_popup && m_popup->isVisible())
            m_popup->hide();
    }
}

void PlayerToolBar::setCollectionInfo(QString info,VIEW::Id mode)
{
    m_mode = mode;
    
    ui_collection_info->setText(info);
    
    if(m_player->state() == ENGINE::STOPPED || !m_player->playingTrack())
    {
        switch( m_mode )
        {
            case (VIEW::ViewAlbum)    :       ui_image->setPixmap( QPixmap(":/images/album.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewArtist)   :       ui_image->setPixmap( QPixmap(":/images/view-artist.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewTrack)    :       ui_image->setPixmap( QPixmap(":/images/track-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewGenre)    :       ui_image->setPixmap( QPixmap(":/images/genre.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewYear)     :       ui_image->setPixmap( QPixmap(":/images/date-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewFavorite) :       ui_image->setPixmap( QPixmap(":/images/favorites-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewPlaylist) :       ui_image->setPixmap( QPixmap(":/images/media-playlist-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewSmartPlaylist) :  ui_image->setPixmap( QPixmap(":/images/smart-playlist-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewDirble)        :  ui_image->setPixmap( QPixmap(":/images/dirble.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewRadionomy)     :  ui_image->setPixmap( QPixmap(":/images/radionomy.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewTuneIn)        :  ui_image->setPixmap( QPixmap(":/images/tunein_48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
            case (VIEW::ViewFavoriteRadio) :  ui_image->setPixmap( QPixmap(":/images/favorites-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;

            default:ui_image->clear();break;   
        }
    }
}


void PlayerToolBar::slot_update_time_position(qint64 newPos /*ms*/)
{
    //Debug::debug() << "      [PlayerToolBar] slot_update_time_position " << newPos;

    if (newPos <= 0)
    {
        m_currentTime->clear();
        return;
    }

    m_currentTime->setText( UTIL::durationToString( newPos / 1000 ) );
}


void PlayerToolBar::slot_update_total_time(qint64 newTotalTime /*ms*/)
{
    if (newTotalTime <= 0) 
    {
        m_separator->hide();
        m_totalTime->clear();
        return;
    }
    //Debug::debug() << "      [PlayerToolBar] slot_update_total_time update to :" << newTotalTime;
    //Debug::debug() << "      [PlayerToolBar] slot_update_total_time update to :" << UTIL::durationToString( newTotalTime / 1000 );

    m_separator->show();
    m_totalTime->setText( UTIL::durationToString( newTotalTime / 1000 ) );
}

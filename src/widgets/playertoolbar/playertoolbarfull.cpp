
#include "playertoolbarfull.h"
#include "volumebutton.h"
#include "audiocontrols.h"
#include "seekslider.h"
#include "iconmanager.h"

#include "widgets/editors/editor_track.h"
#include "widgets/main/main_right.h"
#include "widgets/spacer.h"
#include "widgets/ratingwidget.h"
#include "widgets/spacer.h"
#include "widgets/tagwidget.h"

#include "playqueue/playqueue_model.h"

#include "core/mediaitem/mediaitem.h"
#include "core/database/database_cmd.h"
#include "covers/covercache.h"
#include "core/player/engine.h"
#include "core/database/database.h"

#include "settings.h"
#include "utilities.h"
#include "global_actions.h"
#include "debug.h"

#include "online/lastfm.h"

#include <QLayout>
#include <QtGui>
#include <QtConcurrent>
#include <QFuture>


PlayerToolBarFull::PlayerToolBarFull(QWidget *parent) : PlayerToolBarBase(parent) {
    m_player = Engine::instance();

    this->setObjectName(QString::fromUtf8("playerToolBar"));
    this->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Window, palette.color(QPalette::Base));
    this->setPalette(palette);
    this->setAutoFillBackground(true);

    /* --------------------------------------------------------------------- */
    ui_rating          = new RatingWidget();
    ui_rating->set_draw_frame( false );
    ui_rating->setMaximumWidth(75);

    ui_album_image           = new QLabel(this);
    ui_album_image->setAlignment(Qt::AlignCenter);
    ui_album_image->setMinimumHeight(128);
    ui_album_image->setMaximumHeight(128);

    ui_artist_image    = new QLabel(this);
    ui_artist_image->setAlignment(Qt::AlignCenter);
    ui_artist_image->setMinimumHeight(128);
    ui_artist_image->setMaximumHeight(128);

    ui_btn_station = new ToolButtonBase(this, QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum) );
    ui_btn_station->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    ui_btn_station->setIcon(QIcon(":/images/media-url-18x18.png"));

    ui_btn_title  = new ToolButtonBase(this, QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum) );
    ui_btn_title->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    ui_btn_title->setIcon( IconManager::instance()->icon( "track"));


    ui_btn_artist  = new ToolButtonBase(this, QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum) );
    ui_btn_artist->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    ui_btn_artist->setIcon(IconManager::instance()->icon( "artist"));

    ui_btn_audio = new ToolButtonBase(this, QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum) );
    ui_btn_audio->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    ui_btn_audio->setIcon(QIcon(":/images/files-18x18.png"));
    ui_btn_audio->setEnabled(false);

    QFont font1 = QApplication::font();
    font1.setPointSize(font1.pointSize());
    ui_btn_station->setFont( font1 );
    ui_btn_title->setFont( font1 );
    ui_btn_artist->setFont( font1 );
    ui_btn_audio->setFont( font1 );

    /* toolbar actions */
    PLAYING_TRACK_ADD_QUEUE = new QAction(IconManager::instance()->icon("playlist1"), tr("Add to play queue"), 0);

    PLAYING_TRACK_FAVORITE = new QAction(QIcon(":/images/favorites-48x48.png"), tr("Add to favorites"), 0);

    ToolButtonBase* btn_jump = new ToolButtonBase(this);
    btn_jump->setDefaultAction( ACTIONS()->value(BROWSER_JUMP_TO_TRACK) );
    btn_jump->setIconSize( QSize( 16, 16 ) );

    ToolButtonBase* btn_edit = new ToolButtonBase(this);
    btn_edit->setDefaultAction( ACTIONS()->value(PLAYING_TRACK_EDIT) );
    btn_edit->setIconSize( QSize( 16, 16 ) );

    ToolButtonBase* btn_love = new ToolButtonBase(this);
    btn_love->setDefaultAction( ACTIONS()->value(PLAYING_TRACK_LOVE) );
    btn_love->setIconSize( QSize( 16, 16 ) );

    ToolButtonBase* btn_add = new ToolButtonBase(this);
    btn_add->setDefaultAction( PLAYING_TRACK_ADD_QUEUE );
    btn_add->setIconSize( QSize( 16, 16 ) );

    ToolButtonBase* btn_favorite = new ToolButtonBase(this);
    btn_favorite->setDefaultAction( PLAYING_TRACK_FAVORITE );
    btn_favorite->setIconSize( QSize( 16, 16 ) );

    ToolButtonBase* ui_expand_button = new ToolButtonBase(this);
    ui_expand_button->setIcon( IconManager::instance()->icon( "chevron-down") );

    QVBoxLayout* toolbar_layout = new QVBoxLayout();
    toolbar_layout->setContentsMargins(0, 0, 0, 0);
    toolbar_layout->setSpacing(0);
//     toolbar_layout->addItem(new QSpacerItem(2, 2, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    toolbar_layout->addWidget( btn_jump );
    toolbar_layout->addWidget( btn_edit );
    toolbar_layout->addWidget( btn_love );
    toolbar_layout->addWidget( btn_add );
    toolbar_layout->addWidget( btn_favorite );
//     toolbar_layout->addItem(new QSpacerItem(2, 2, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));

    ui_tags_widget = new QWidget();
    ui_tags_widget->setLayout(new QVBoxLayout());

    QVBoxLayout* cover_layout = new QVBoxLayout();
    cover_layout->setContentsMargins(0, 0, 0, 0);
    cover_layout->setSpacing(0);
    cover_layout->addWidget( ui_album_image );

    QVBoxLayout* labels_layout = new QVBoxLayout();
    labels_layout->setContentsMargins(0, 0, 0, 0);
    labels_layout->setSpacing(0);
    labels_layout->addItem(new QSpacerItem(2, 2, QSizePolicy::Minimum, QSizePolicy::Expanding));

    labels_layout->addWidget( ui_rating );
    labels_layout->addWidget( ui_btn_station );
    labels_layout->addWidget( ui_btn_title );
    labels_layout->addWidget( ui_btn_artist );
    labels_layout->addWidget( ui_btn_audio );
    labels_layout->addItem(new QSpacerItem(2, 2, QSizePolicy::Minimum, QSizePolicy::Expanding));

    labels_layout->setAlignment(ui_btn_station, Qt::AlignLeft | Qt::AlignVCenter);
    labels_layout->setAlignment(ui_btn_title, Qt::AlignLeft | Qt::AlignVCenter);
    labels_layout->setAlignment(ui_btn_artist, Qt::AlignLeft | Qt::AlignVCenter);
    labels_layout->setAlignment(ui_btn_audio, Qt::AlignLeft | Qt::AlignVCenter);
    labels_layout->setAlignment(ui_rating, Qt::AlignCenter);

    QHBoxLayout* layout_now_playing = new QHBoxLayout();
    layout_now_playing->setSpacing(8);
    layout_now_playing->setContentsMargins(0, 0, 0, 0);
    layout_now_playing->addLayout( toolbar_layout, Qt::AlignVCenter | Qt::AlignLeft);
    layout_now_playing->addLayout( cover_layout , Qt::AlignVCenter | Qt::AlignLeft);
    layout_now_playing->addLayout( labels_layout , Qt::AlignVCenter | Qt::AlignLeft);

    m_now_playing_widget = new QWidget( this );
    m_now_playing_widget->setLayout( layout_now_playing );

    QHBoxLayout* artist_layout = new QHBoxLayout();
    artist_layout->setSpacing(0);
    artist_layout->setContentsMargins(0, 0, 0, 0);
    artist_layout->addItem(new QSpacerItem(2, 2, QSizePolicy::Expanding, QSizePolicy::Minimum));
    artist_layout->addWidget( ui_tags_widget );
    artist_layout->addWidget( ui_artist_image );

    artist_layout->setAlignment(ui_tags_widget, Qt::AlignRight | Qt::AlignVCenter);
    artist_layout->setAlignment(ui_artist_image, Qt::AlignRight | Qt::AlignVCenter);

    /* -- time track position -- */
    QFont font = QApplication::font();
    font.setPointSize(font.pointSize() * 1.5);

    m_currentTime = new QLabel(this);
    m_currentTime->setFont(font);
    m_currentTime->setAlignment( Qt::AlignLeft );
    m_currentTime->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    m_currentTime->setMaximumHeight(32);

    m_totalTime = new QLabel(this);
    m_totalTime->setFont(font);
    m_totalTime->setAlignment( Qt::AlignRight );
    m_totalTime->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    m_totalTime->setMaximumHeight(32);

    m_separator = new QLabel(this);
    m_separator->setFont(font);
    m_separator->setAlignment( Qt::AlignRight );
    m_separator->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    m_separator->setText("/");
    m_separator->hide();
    m_separator->setMaximumHeight(32);

    QHBoxLayout *trackTimeLayout = new QHBoxLayout();
    trackTimeLayout->setContentsMargins( 0, 0, 0, 0);
    trackTimeLayout->setSpacing(1);
    //trackTimeLayout->setContentsMargins(0);
    trackTimeLayout->addSpacerItem ( new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum) );
    trackTimeLayout->addWidget( m_currentTime, 0, Qt::AlignVCenter | Qt::AlignCenter);
    trackTimeLayout->addWidget( m_separator, 0, Qt::AlignVCenter | Qt::AlignCenter);
    trackTimeLayout->addWidget( m_totalTime, 0, Qt::AlignVCenter | Qt::AlignCenter);
    trackTimeLayout->addSpacerItem ( new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum) );

    /* -- Player tool buton creation -- */
    ToolButtonBase* ui_prev_button = new ToolButtonBase(this);
    ui_prev_button->setDefaultAction(ACTIONS()->value(ENGINE_PLAY_PREV));
    ui_prev_button->setIconSize( QSize( 32, 32 ) );
    ui_prev_button->setMaximumHeight(32);

    ToolButtonBase* ui_next_button = new ToolButtonBase(this);
    ui_next_button->setDefaultAction(ACTIONS()->value(ENGINE_PLAY_NEXT));
    ui_next_button->setIconSize( QSize( 32, 32 ) );
    ui_next_button->setMaximumHeight(32);

    ToolButtonBase* ui_play_button = new ToolButtonBase(this);
    ui_play_button->setDefaultAction(ACTIONS()->value(ENGINE_PLAY));
    ui_play_button->setIconSize( QSize( 32, 32 ) );
    ui_play_button->setMaximumHeight(32);

    ToolButtonBase* ui_stop_button = new ToolButtonBase(this);
    ui_stop_button->setDefaultAction(ACTIONS()->value(ENGINE_STOP));
    ui_stop_button->setIconSize( QSize( 32, 32 ) );
    ui_stop_button->setMaximumHeight(32);

    QHBoxLayout* vh2 = new QHBoxLayout();
    vh2->addWidget( ui_prev_button , Qt::AlignVCenter | Qt::AlignCenter);
    vh2->addWidget( ui_play_button , Qt::AlignVCenter | Qt::AlignCenter);
    vh2->addWidget( ui_stop_button , Qt::AlignVCenter | Qt::AlignCenter);
    vh2->addWidget( ui_next_button , Qt::AlignVCenter | Qt::AlignCenter);

    /* -- audio control buttons -- */
    ToolButtonBase* ui_equalizer_button = new ToolButtonBase(this);
    ui_equalizer_button->setDefaultAction(ACTIONS()->value(ENGINE_AUDIO_EQ));
    ui_equalizer_button->setIconSize( QSize( 32, 32 ) );
    ui_equalizer_button->setMaximumHeight(32);

    VolumeButton* vbutton = new VolumeButton(this);
    vbutton->setMaximumHeight(32);

    RepeatControl* rcontrl = new RepeatControl(this);
    rcontrl->setMaximumHeight(32);

    ShuffleControl* scontrl = new ShuffleControl(this);
    scontrl->setMaximumHeight(32);

    QHBoxLayout* vh3 = new QHBoxLayout();
    vh3->addWidget( vbutton , Qt::AlignVCenter | Qt::AlignCenter);
    vh3->addWidget( ui_equalizer_button , Qt::AlignVCenter | Qt::AlignCenter);
    vh3->addWidget( rcontrl , Qt::AlignVCenter | Qt::AlignCenter);
    vh3->addWidget( scontrl , Qt::AlignVCenter | Qt::AlignCenter);

    QVBoxLayout* vl2 = new QVBoxLayout();
    vl2->setContentsMargins(0, 0, 0, 0);
    vl2->setSpacing(4);
    vl2->addSpacerItem ( new QSpacerItem(1, 10, QSizePolicy::Minimum, QSizePolicy::Expanding) );
    vl2->addLayout(vh2, Qt::AlignCenter  ) ;
    vl2->addLayout(trackTimeLayout);
    vl2->addLayout(vh3, Qt::AlignCenter  ) ;
    vl2->addSpacerItem ( new QSpacerItem(1, 10, QSizePolicy::Minimum, QSizePolicy::Expanding) );

    /* -- Final layout -- */
    QGridLayout* gl = new QGridLayout();
    gl->setContentsMargins(0, 0, 0, 0);
    gl->addWidget(new FixedSpacer(this, QSize(5,0)) , 0, 0);
    gl->addWidget( m_now_playing_widget , 0, 1,Qt::AlignLeft | Qt::AlignVCenter ) ;
    gl->addLayout( vl2, 0, 2, Qt::AlignCenter | Qt::AlignVCenter ) ;
    gl->addLayout( artist_layout, 0, 3) ;
    gl->addWidget( ui_expand_button, 0, 4, Qt::AlignRight) ;
    gl->addWidget( new FixedSpacer(this, QSize(5,0)) , 0, 5,Qt::AlignRight);

    gl->setColumnStretch( 1, 1);
    gl->setColumnStretch( 3, 1);

    /* -- widget layout -- */
    QVBoxLayout *layoutx = new QVBoxLayout(this);
    layoutx->setContentsMargins( 0, 0, 0, 0 );
    layoutx->setSpacing(0);
    layoutx->addWidget(new SeekSlider(this));
    layoutx->addLayout(gl);

    /* ----- connection ----- */
    connect(this->m_player, SIGNAL(mediaTick(qint64)), this, SLOT(slot_update_time_position(qint64)));
    connect(this->m_player, SIGNAL(mediaMetaDataChanged()), this, SLOT(slot_update_track_playing_info()));
    connect(this->m_player, SIGNAL(mediaChanged()), this, SLOT(slot_update_track_playing_info()));
    connect(this->m_player, SIGNAL(engineStateChanged()), this, SLOT(slot_update_track_playing_info()));

    connect( ui_expand_button, SIGNAL( clicked ( bool ) ), this, SIGNAL( switchToolBarType() ) );

    connect( ui_rating, SIGNAL(RatingChanged(float)), this, SLOT(slot_rating_changed(float)));

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

void PlayerToolBarFull::fullUpdate() {
    m_track = Engine::instance()->playingTrack();

    slot_update_track_playing_info();
}

void PlayerToolBarFull::clear() {
    //Debug::debug() << "      [PlayerToolBarFull] clear";
    m_metadata["ARTIST"].clear();
    m_metadata["ALBUM"].clear();
    m_metadata["TITLE"].clear();

    ui_rating->hide();
    ui_btn_station->hide();
    ui_btn_title->hide();
    ui_btn_artist->hide();
    ui_btn_audio->hide();

    foreach (QWidget * child, ui_tags_widget->findChildren<QWidget*>())
          delete child;

    m_currentTime->clear();
    m_totalTime->clear();
    m_separator->hide();
}

void PlayerToolBarFull::slot_update_track_playing_info() {
    Debug::debug() << "      [PlayerToolBarFull] slot_update_track_playing_info";

    m_track = Engine::instance()->playingTrack();

    /* update now playing widget */
    if(m_player->state() != ENGINE::STOPPED && m_track)
    {
        ui_btn_title->show();
        ui_btn_artist->show();
        ui_btn_audio->show();

        /* ----- update total time for current track ----- */
        slot_update_total_time( m_player->currentTotalTime() );


        /* update actions */
        PLAYING_TRACK_ADD_QUEUE->setEnabled( true );
        PLAYING_TRACK_FAVORITE->setEnabled( true );
        ACTIONS()->value(PLAYING_TRACK_EDIT)->setEnabled( m_track->id != -1 );
        ACTIONS()->value(PLAYING_TRACK_LOVE)->setEnabled( LastFmService::instance()->isAuthenticated() );

        /* update album image */
        QPixmap pix = CoverCache::instance()->cover( m_track );

        ui_album_image->setPixmap( pix.scaled(QSize(128,128), Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );

        /* audio properties */
        setAudioProperties();


        /* -------- TRACK ---------- */
        if(m_track->type() == TYPE_TRACK)
        {
            ui_btn_station->hide();

            if( m_track->id != -1 )
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


        if( m_metadata["ARTIST"] != m_track->artist )
        {
            ui_artist_image->clear();

            slot_get_artist_image();

            slot_download_artists_tags();
        }

        /* ----- register new metadata ----- */
        m_metadata["ARTIST"] = m_track->artist;
        m_metadata["ALBUM"]  = m_track->album;
        m_metadata["TITLE"]  = m_track->title;

        /* ----- ALWAYS update title - album - artist label ----- */
        const int width = qobject_cast<QWidget*>(this->parent())->width()/4;

        const QString artist = m_track->album.isEmpty() ? m_track->artist : m_track->artist + " - " + m_track->album;

        QString clippedText = QFontMetrics(ui_btn_artist->font()).elidedText(artist, Qt::ElideRight, width);
        ui_btn_artist->setText(clippedText);


        QString title_or_url = m_track->title.isEmpty() ? m_track->url : m_track->title;

        if(m_track->type() == TYPE_STREAM)
          title_or_url = m_track->title.isEmpty() ? m_track->extra["station"].toString() : m_track->title;

        clippedText = QFontMetrics(ui_btn_title->font()).elidedText(title_or_url, Qt::ElideRight, width);
        ui_btn_title->setText( clippedText );
    }
    else
    {
        this->clear();

        PLAYING_TRACK_ADD_QUEUE->setEnabled( false );
        PLAYING_TRACK_FAVORITE->setEnabled( false );
        ACTIONS()->value(PLAYING_TRACK_EDIT)->setEnabled( false );
        ACTIONS()->value(PLAYING_TRACK_LOVE)->setEnabled( false );

        m_currentTime->setText("STOPPED");

        QPixmap pix = QPixmap(":/images/default-cover-256x256.png");

        ui_album_image->setPixmap( pix.scaledToHeight(128, Qt::SmoothTransformation) );
        ui_artist_image->setPixmap( pix.scaledToHeight(128, Qt::SmoothTransformation) );
    }

}

void PlayerToolBarFull::setCollectionInfo(QString info,VIEW::Id mode) {
  Q_UNUSED(info)
  Q_UNUSED(mode)
}


void PlayerToolBarFull::setAudioProperties() {
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


void PlayerToolBarFull::slot_get_artist_image() {
    //Debug::debug() << "   [PlayerToolBarFull] slot_get_artist_image m_track" << m_track;

    /* track from collection */
    if(Database::instance()->param()._option_artist_image == true)
    {
      if(m_track && m_track->id != -1 && m_track->parent())
      {
          //Debug::debug() << "   [PlayerToolBarFull] slot_get_artist_image m_track->id " << m_track->id;
          if(m_track->parent()->type() == TYPE_ALBUM)
          {
              if(MEDIA::MediaPtr artist = m_track->parent()->parent())
              {
                  QPixmap pix = CoverCache::instance()->image( artist );

                  ui_artist_image->setPixmap( pix.scaledToHeight(128, Qt::SmoothTransformation) );

                  return;
              }
          }
      }
    }

    if( Database::instance()->param()._option_download_cover == true )
    {
          if( !Engine::instance()->playingTrack()->artist.isEmpty() )
            slot_download_image();
    }
}

void PlayerToolBarFull::slot_download_image() {
    Debug::debug() << "   [PlayerToolBarFull] slot_download_image";
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

void PlayerToolBarFull::slot_download_artists_tags() {
    Debug::debug() << "   [PlayerToolBarFull] slot_download_artists_tags";
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

void PlayerToolBarFull::slot_on_infosystem_received(INFO::InfoRequestData request, QVariant output) {
    //Debug::debug() << "   [PlayerToolBarFull] slot_on_infosystem_received";
    /* check request */
    if(!m_requests_ids.contains(request.requestId))
      return;

    if(m_player->state() == ENGINE::STOPPED || !m_track)
      return;

    m_requests_ids.removeOne(request.requestId);

    /* get data from request */
    if ( request.type == INFO::InfoArtistImages )
    {
        QImage image = UTIL::artistImageFromByteArray( output.toByteArray(), SETTINGS()->_coverSize );

        if( image.isNull() )
          return;

        ui_artist_image->setPixmap( QPixmap::fromImage(image).scaledToHeight(128, Qt::SmoothTransformation) );
        ui_artist_image->update();
    }
    else if ( request.type == INFO::InfoArtistTerms )
    {
        foreach (QWidget * child, ui_tags_widget->findChildren<QWidget*>())
          delete child;

        QVariantList list = output.toList();
        foreach(QVariant tag, list)
        {
           //Debug::debug() << "   [PlayerToolBarFull] slot_on_infosystem_received tag " << tag.toString();

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

void PlayerToolBarFull::slot_on_tag_clicked() {
    Debug::debug() << "   [PlayerToolBarFull] slot_on_tag_clicked";

    TagWidget *tag = qobject_cast<TagWidget *>(sender());

    ACTIONS()->value(TAG_CLICKED)->setData(tag->text());

    ACTIONS()->value(TAG_CLICKED)->trigger();
}

void PlayerToolBarFull::slot_on_websearch() {
    Debug::debug() << "   [PlayerToolBarFull] slot_on_websearch";

    ToolButtonBase* button = dynamic_cast<ToolButtonBase*>(sender());

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

void PlayerToolBarFull::slot_on_add_to_favorite() {
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

void PlayerToolBarFull::slot_rating_changed(float) {
    if( m_track )
    {
        /* apply rating */
        m_track->rating = ui_rating->rating();

        /* save change to database */
        QList<MEDIA::MediaPtr> medias;
        medias << m_track;
        QFuture<void> future = QtConcurrent::run(DatabaseCmd::rateMediaItems, medias);
    }
}

void PlayerToolBarFull::slot_on_lastfm_love() {
    //Debug::debug() << "PlayerToolBarFull::slot_on_lastfm_love";
    MEDIA::TrackPtr m_track = Engine::instance()->playingTrack();

    if( m_track  )
      LastFmService::instance()->love( m_track );
}

void PlayerToolBarFull::slot_on_track_edit() {
    MainRightWidget::instance()->addWidget(new EditorTrack( m_track ));
}

void PlayerToolBarFull::slot_on_track_add_to_queue() {
    Playqueue::instance()->addMediaItem( m_track );
}

void PlayerToolBarFull::slot_update_time_position(qint64 newPos /*ms*/) {
    //Debug::debug() << "      [PlayerToolBarFull] slot_update_time_position " << newPos;

    if (newPos <= 0)
    {
        m_currentTime->clear();
        return;
    }

    m_currentTime->setText( UTIL::durationToString( newPos / 1000 ) );
}

void PlayerToolBarFull::slot_update_total_time(qint64 newTotalTime /*ms*/) {
    if (newTotalTime <= 0)
    {
        m_separator->hide();
        m_totalTime->clear();
        return;
    }

    m_separator->show();
    m_totalTime->setText( UTIL::durationToString( newTotalTime / 1000 ) );
}

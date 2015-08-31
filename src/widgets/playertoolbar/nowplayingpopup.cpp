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

#include "nowplayingpopup.h"
#include "core/player/engine.h"
#include "core/mediaitem/mediaitem.h"
#include "core/database/database_cmd.h"
#include "covers/covercache.h"
#include "widgets/ratingwidget.h"

#include "widgets/main/main_right.h"
#include "widgets/editors/editor_track.h"

#include "online/lastfm.h"

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

/*
********************************************************************************
*                                                                              *
*    Class NowPlayingPopup                                                     *
*                                                                              *
********************************************************************************
*/
NowPlayingPopup::NowPlayingPopup(QWidget *parent) : QWidget(parent)
{
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    this->setPalette(palette);

    this->setAutoFillBackground(true);
    this->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
    
    /* ----- UI ----- */
    ui_rating          = new RatingWidget();
    ui_rating->set_draw_frame( false );
    ui_rating->setMaximumWidth(75);
 
    ui_image           = new QLabel(this);
    ui_image->setAlignment(Qt::AlignCenter);
    ui_image->setFixedHeight (120);
    ui_image->setFixedWidth (120);
    ui_image->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    ui_label_title     = new QLabel(this);
    ui_label_album     = new QLabel(this);
    
    QFont font1 = QApplication::font();
    font1.setBold( true );
    font1.setPointSize(font1.pointSize()*1.3);
        
    ui_label_title->setFont( font1 );
    
    QFont font2 = QApplication::font();
    font2.setPointSize(font2.pointSize()*1.3);

    ui_label_album->setFont( font2 );
    
    ui_toolbar = new QToolBar(this);
    ui_toolbar->setOrientation(Qt::Horizontal);
    ui_toolbar->setIconSize( QSize( 14, 14 ) );
    ui_toolbar->addAction( ACTIONS()->value(PLAYING_TRACK_EDIT)  );
    ui_toolbar->addAction( ACTIONS()->value(BROWSER_JUMP_TO_TRACK) );
    ui_toolbar->addAction( ACTIONS()->value(PLAYING_TRACK_LOVE) );


    QHBoxLayout* h1 = new QHBoxLayout();
    h1->addWidget( ui_rating );
    h1->addItem(new QSpacerItem(2, 2, QSizePolicy::Expanding, QSizePolicy::Minimum));
    
    
    QVBoxLayout* labels_layout = new QVBoxLayout();
    labels_layout->setContentsMargins(20, 10, 20, 10);
    labels_layout->setSpacing(4);
    labels_layout->addItem(new QSpacerItem(2, 2, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    labels_layout->addWidget( ui_label_title );
    labels_layout->addWidget( ui_label_album );
    labels_layout->addLayout( h1 );
    labels_layout->addWidget( ui_toolbar );
    labels_layout->addItem(new QSpacerItem(2, 2, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget( ui_image );
    layout->addLayout( labels_layout );


    /* ----- connection ----- */
    connect(ui_rating, SIGNAL(RatingChanged(float)), this, SLOT(slot_rating_changed(float)));
     
    connect(ACTIONS()->value(PLAYING_TRACK_LOVE), SIGNAL(triggered()), this, SLOT(slot_on_lastfm_love()));
    connect(ACTIONS()->value(PLAYING_TRACK_EDIT), SIGNAL(triggered()), this, SLOT(slot_on_track_edit()));
}

/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::updateWidget                                               */
/* ---------------------------------------------------------------------------*/ 
void NowPlayingPopup::updateWidget()
{
     slot_update_widget();
    
     updateGeometry();

     update();
}

/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::slot_update_widget                                         */
/* ---------------------------------------------------------------------------*/ 
void NowPlayingPopup::slot_update_widget()
{
    //Debug::debug() << "  [NowPlayingPopup] slot_update_widget";
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();

    /* update actions */
    ACTIONS()->value(PLAYING_TRACK_EDIT)->setEnabled( (track && track->id != -1) ? true : false );
    ACTIONS()->value(PLAYING_TRACK_LOVE)->setEnabled( (track && track->id != -1 && LastFmService::instance()->isAuthenticated() )
                                                ? true : false );
    /* update widget */     
    if(Engine::instance()->state() != ENGINE::STOPPED && track)
    {
        // Debug::debug() << "## Now playing TITLE :" << track->title;
        // Debug::debug() << "## Now playing NAME  :" << track->name;
        // Debug::debug() << "## Now playing URL   :" << track->url;
        // Debug::debug() << "## Now playing GENRE :" << track->genre;

        /* update image DEFAULT SIZE IS 120x120 */
        QPixmap pix = CoverCache::instance()->cover(track);
        ui_image->setPixmap( pix );

        /* update labels title/album/artist */
        QString title_or_url = track->title.isEmpty() ? track->url : track->title;
        if(track->type() == TYPE_STREAM)
          title_or_url = track->title.isEmpty() ? track->name : track->title;

        ui_label_title->setText( title_or_url/*clippedText*/ );

        const QString album = track->album.isEmpty() ? track->artist : track->artist + " - " + track->album;

        ui_label_album->setText (album /*clippedText*/ );

        /* update rating */
        if(track->id != -1 )
        {
          ui_rating->set_rating( track->rating );
          ui_rating->set_user_rating( true );
          ui_rating->set_enable( true );
          ui_rating->show();
        }
        else
        {
          ui_rating->set_rating( 0.0 );
          ui_rating->set_user_rating( true );
          ui_rating->set_enable( false );
          ui_rating->show();
        }
    }
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
    
    if( track && track->type() == TYPE_TRACK )
      LastFmService::instance()->love(track);
}


/* ---------------------------------------------------------------------------*/
/* NowPlayingPopup::slot_on_track_edit                                        */
/* ---------------------------------------------------------------------------*/ 
void NowPlayingPopup::slot_on_track_edit()
{
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();
  
    MainRightWidget::instance()->addWidget(new EditorTrack(track));
}



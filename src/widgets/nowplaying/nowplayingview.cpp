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

#include "nowplayingview.h"
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
*    Class NowPlayingView                                                      *
*                                                                              *
********************************************************************************
*/
NowPlayingView::NowPlayingView(QWidget *parent) : QWidget(parent)
{
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    this->setPalette(palette);

    this->setFixedHeight (130);
    this->setFocusPolicy( Qt::NoFocus );
    this->setAutoFillBackground(true);
    this->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    
    /* ----- global actions ----- */
    ACTIONS()->insert(PLAYING_TRACK_EDIT,new QAction(QIcon(":/images/edit-48x48.png"), tr("Edit"), this));
    ACTIONS()->insert(PLAYING_TRACK_LOVE, new QAction(QIcon(":/images/lastfm.png"), tr("Send LastFm love"), this));


    /* ----- UI ----- */
    ui_rating          = new RatingWidget();
    ui_rating->set_draw_frame( false );
    ui_rating->setMaximumWidth(75);
 
    ui_image           = new QLabel();
    ui_image->setAlignment(Qt::AlignCenter);
    ui_image->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    
    ui_label_title     = new QLabel();
    ui_label_album     = new QLabel();
    
    QFont font = QApplication::font();
    font.setBold( true );
    ui_label_title->setFont( font );
    ui_label_title->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    
    ui_label_album->setFont( QApplication::font() );
    
    ui_toolbar = new QToolBar();
    ui_toolbar->setOrientation(Qt::Horizontal);
    ui_toolbar->setIconSize( QSize( 14, 14 ) );
    ui_toolbar->addAction( ACTIONS()->value(PLAYING_TRACK_EDIT)  );
    ui_toolbar->addAction( ACTIONS()->value(PLAYQUEUE_JUMP_TO_TRACK) );
    ui_toolbar->addAction( ACTIONS()->value(PLAYING_TRACK_LOVE) );


    QHBoxLayout* h1 = new QHBoxLayout();
    h1->addWidget( ui_rating );
    h1->addItem(new QSpacerItem(2, 2, QSizePolicy::Expanding, QSizePolicy::Minimum));
    
    
    QVBoxLayout* labels_layout = new QVBoxLayout();
    labels_layout->setSpacing(4);
    labels_layout->setContentsMargins(10, 0, 0, 0);
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
     
    connect(Engine::instance(), SIGNAL(mediaChanged()), this, SLOT(slot_update_widget()));
    connect(Engine::instance(), SIGNAL(mediaMetaDataChanged()), this, SLOT(slot_update_widget()));
    connect(Engine::instance(), SIGNAL(engineStateChanged()), this, SLOT(slot_update_widget()));

    connect(ACTIONS()->value(PLAYING_TRACK_LOVE), SIGNAL(triggered()), this, SLOT(slot_on_lastfm_love()));
    connect(ACTIONS()->value(PLAYING_TRACK_EDIT), SIGNAL(triggered()), this, SLOT(slot_on_track_edit()));


    /* ----- initialization ----- */    
    slot_update_widget();
}


/* ---------------------------------------------------------------------------*/
/* NowPlayingView::slot_update_widget                                         */
/* ---------------------------------------------------------------------------*/ 
void NowPlayingView::slot_update_widget()
{
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();
  
    /* update actions */
    ACTIONS()->value(PLAYING_TRACK_EDIT)->setEnabled( (track && track->id != -1) ? true : false );
    ACTIONS()->value(PLAYING_TRACK_LOVE)->setEnabled( (track && track->id != -1 && LastFmService::instance()->isAuthenticated() )
                                                ? true : false );
    
    /* update widget */     
    if(Engine::instance()->state() != ENGINE::STOPPED && track)
    {
        /* update image */
        QPixmap pix = CoverCache::instance()->cover(track);
        QPixmap newpix = pix.scaled(QSize(100,100), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);      
        ui_image->setPixmap( newpix );

        /* update toolbar */
        ui_toolbar->show();

        /* update labels title/album/artist */
        const QString title_or_url = track->title.isEmpty() ? track->url : track->title;

        int width = this->width() - 110 - 10 ;
        QString clippedText = QFontMetrics(ui_label_title->font()).elidedText(title_or_url, Qt::ElideRight, width);
        ui_label_title->setText( clippedText );

        ui_label_album->setText ( QFontMetrics(ui_label_album->font()).elidedText(track->artist + " - " + track->album, Qt::ElideRight, width) );

        /* update rating */
        if(track->id != -1 ) {
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
    else
    {
        ui_image->clear(); 
        ui_toolbar->hide();
        ui_label_title->clear();
        ui_label_album->clear();
        ui_rating->hide();
    }

    this->update();
}


/* ---------------------------------------------------------------------------*/
/* NowPlayingView::slot_rating_changed                                        */
/* ---------------------------------------------------------------------------*/ 
void NowPlayingView::slot_rating_changed(float)
{
    Debug::debug() << "NowPlayingView::slot_rating_changed";
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
/* NowPlayingView::slot_on_lastfm_love                                        */
/* ---------------------------------------------------------------------------*/ 
void NowPlayingView::slot_on_lastfm_love()
{
    Debug::debug() << "NowPlayingView::slot_on_lastfm_love";
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();
    
    if( track && track->type() == TYPE_TRACK )
      LastFmService::instance()->love(track);
}


/* ---------------------------------------------------------------------------*/
/* NowPlayingView::slot_on_track_edit                                         */
/* ---------------------------------------------------------------------------*/ 
void NowPlayingView::slot_on_track_edit()
{
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();
  
    MainRightWidget::instance()->addWidget(new EditorTrack(track));
}



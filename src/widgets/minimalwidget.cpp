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

#include "minimalwidget.h"

#include "core/player/engine.h"
#include "core/mediaitem/mediaitem.h"
#include "covers/covercache.h"
#include "widgets/spacer.h"
#include "global_actions.h"

#include "ratingwidget.h"

#include "debug.h"

#include <QLayout>
#include <QPainter>
#include <QToolButton>
#include <QApplication>

/*
********************************************************************************
*                                                                              *
*    MinimalWidget                                                             *
*                                                                              *
********************************************************************************
*/
MinimalWidget::MinimalWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute( Qt::WA_Hover );
    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);

    /* labels */
    ui_rating          = new RatingWidget();
    ui_rating->set_draw_frame( false );
    ui_rating->setMaximumWidth(75);

    ui_image           = new QLabel();
    ui_image->setAlignment(Qt::AlignCenter);
    ui_image->setFixedSize(120,120);
    
    ui_label_title     = new QLabel();
    ui_label_album     = new QLabel();

    ui_label_title->setAlignment(Qt::AlignCenter);
    ui_label_album->setAlignment(Qt::AlignCenter);
    
    QFont font1 = QApplication::font();
    font1.setBold( true );
    font1.setPointSize(QApplication::font().pointSize()*1.2);
    
    QFont font2 = QApplication::font();
    font2.setPointSize(QApplication::font().pointSize()*1.1);
    
    ui_label_title->setFont( font1 );
    ui_label_title->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    
    ui_label_album->setFont( font2 );

    
    QHBoxLayout* hl0 = new QHBoxLayout();
    hl0->setSpacing(0);
    hl0->setContentsMargins(0, 0, 0, 0);
    hl0->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hl0->addWidget( ui_image );
    hl0->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));

    QHBoxLayout* hl1 = new QHBoxLayout();
    hl1->setSpacing(0);
    hl1->setContentsMargins(0, 0, 0, 0);
    hl1->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hl1->addWidget( ui_rating );
    hl1->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));    

    
    QVBoxLayout* vl0 = new QVBoxLayout();
    vl0->setSpacing(2);
    vl0->setContentsMargins(4, 4, 4, 4);
    vl0->addWidget( ui_label_title ,Qt::AlignHCenter);
    vl0->addWidget( ui_label_album ,Qt::AlignHCenter);
    vl0->addLayout( hl0 );
    vl0->addLayout( hl1 );

    /* TOOLBAR */
    ui_toolbar = new QToolBar();
    ui_toolbar->setOrientation(Qt::Horizontal);
    ui_toolbar->setIconSize( QSize( 28, 28 ) );
    ui_toolbar->addAction( ACTIONS()->value(ENGINE_PLAY_PREV)  );
    ui_toolbar->addAction( ACTIONS()->value(ENGINE_PLAY) );
    ui_toolbar->addAction( ACTIONS()->value(ENGINE_STOP) );
    ui_toolbar->addAction( ACTIONS()->value(ENGINE_PLAY_NEXT) );
    ui_toolbar->addAction( ACTIONS()->value(APP_MODE_NORMAL) );

     
    QHBoxLayout* hl2 = new QHBoxLayout();
    hl2->setSpacing(0);
    hl2->setContentsMargins(0, 0, 0, 0);
    hl2->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hl2->addWidget( ui_toolbar );
    hl2->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
    
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout( vl0 );
    layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
    layout->addLayout( hl2 );
    
    this->setMinimumSize(QSize(200,200));
    this->adjustSize();
    this->setFixedSize( sizeHint() );
    this->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    /* signals connection */    
    connect(Engine::instance(), SIGNAL(mediaChanged()), this, SLOT(slot_update()));
    connect(Engine::instance(), SIGNAL(mediaMetaDataChanged()), this, SLOT(slot_update()));
    connect(Engine::instance(), SIGNAL(engineStateChanged()), this, SLOT(slot_update()));
    
    installEventFilter(this);
}

/* ---------------------------------------------------------------------------*/
/* MinimalWidget::eventFilter                                                 */
/* ---------------------------------------------------------------------------*/
bool MinimalWidget::eventFilter( QObject */*obj*/, QEvent *event )
{
    QEvent::Type type = event->type();

    if ( type == QEvent::MouseMove )
    {
      move( static_cast<QMouseEvent*>( event )->globalPos() );
      return true;
    }

    return false;
}

/* ---------------------------------------------------------------------------*/
/* MinimalWidget::slot_update                                                 */
/* ---------------------------------------------------------------------------*/
void MinimalWidget::slot_update()
{
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();
  
    /* update widget */     
    if(Engine::instance()->state() != ENGINE::STOPPED && track)
    {
        /* update image */
        QPixmap pix = CoverCache::instance()->cover(track);
        ui_image->setPixmap( pix );

        /* update labels title/album/artist */
        const QString title_or_url = track->title.isEmpty() ? track->url : track->title;

        int width = this->width() - 20 ;
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
        ui_label_title->clear();
        ui_label_album->clear();
        ui_rating->hide();
    }

    this->update();
}

/* ---------------------------------------------------------------------------*/
/* MinimalWidget::showEvent                                                   */
/* ---------------------------------------------------------------------------*/
void MinimalWidget::showEvent ( QShowEvent * event )
{
   slot_update();

   QWidget::showEvent(event);
}

/* ---------------------------------------------------------------------------*/
/* MinimalWidget::paintEvent                                                  */
/* ---------------------------------------------------------------------------*/
void MinimalWidget::paintEvent(QPaintEvent *)
{
    //! draw background
    QPainter painter(this);

    QColor back_color = QApplication::palette().color(QPalette::Normal, QPalette::Background);
    
    
    back_color.setAlpha(70);

    painter.setBrush(QBrush(back_color));
    painter.fillRect(rect(), back_color);
}


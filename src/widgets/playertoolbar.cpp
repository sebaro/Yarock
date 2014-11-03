/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2014 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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
#include "widgets/audiocontrols.h"
#include "widgets/spacer.h"
#include "widgets/seekslider.h"

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
*    Class VolumeToolButton                                                    *
*                                                                              *
********************************************************************************
*/
VolumeToolButton::VolumeToolButton(QWidget *parent) : QToolButton( parent )
{
    this->setIconSize( QSize( 26, 26 ) );
    this->setAutoRaise(true);
    this->setIcon(QIcon(":/images/volume-icon.png"));
    this->setPopupMode (QToolButton::InstantPopup);
    
    /* ---- label ---- */
    m_volume_label= new QLabel( this );
    m_volume_label->setAlignment( Qt::AlignCenter );
    m_volume_label->setMinimumWidth( QFontMetrics(QApplication::font()).width(QString("100%")) +1 );
    m_volume_label->setMaximumWidth( QFontMetrics(QApplication::font()).width(QString("100%")) +1 );
    m_volume_label->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum );

    /* ---- volume slider ---- */
    m_slider = new QSlider();
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setMaximum(100);
    m_slider->setRange(0, 100);
    m_slider->setPageStep(5);
    m_slider->setSingleStep(1);
    m_slider->setMinimumWidth( 120 );
    m_slider->setMaximumWidth( 120 );
    m_slider->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    
    /* ---- mute button ---- */
    QToolButton* mute_button = new QToolButton();
    mute_button->setDefaultAction(ACTIONS()->value(ENGINE_VOL_MUTE));
    mute_button->setIconSize( QSize( 22, 22 ) );
    mute_button->setAutoRaise(true);
    mute_button->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    /* ---- layout ---- */
    QHBoxLayout * mainBox = new QHBoxLayout(this);
    mainBox->setSpacing(4);
    mainBox->setContentsMargins(4,0,4,0);
    mainBox->addWidget(mute_button, 0, Qt::AlignVCenter | Qt::AlignLeft );
    mainBox->addWidget(m_slider, 0, Qt::AlignVCenter | Qt::AlignLeft );
    mainBox->addWidget(m_volume_label,0, Qt::AlignVCenter | Qt::AlignLeft );


    QWidget* main_widget = new QWidget(this);
    main_widget->setLayout(mainBox);
    main_widget->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );

    QWidgetAction * sliderActionWidget = new QWidgetAction( this );
    sliderActionWidget->setDefaultWidget( main_widget );

    /* ---- popup menu ---- */
    m_menu = new QMenu(this);
    m_menu->addAction( sliderActionWidget );
    m_menu->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
    
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    m_menu->setPalette(palette);
    
    /* ---- signals connections ---- */
    connect(ACTIONS()->value(ENGINE_VOL_MUTE), SIGNAL(triggered()), this, SLOT(slot_mute_toggle_action()));
    connect(Engine::instance() , SIGNAL(muteStateChanged()), this, SLOT(slot_mute_change()));
    connect(Engine::instance() , SIGNAL(volumeChanged()), this, SLOT(slot_volume_change()));
    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(slot_apply_volume(int)));

    connect(this, SIGNAL(clicked()), this, SLOT(slot_show_menu()));
    
    /* ---- init  ---- */
    Engine::instance()->setMuted(false);
    m_slider->setValue(Engine::instance()->volume());    
}


void VolumeToolButton::slot_show_menu()
{
    if(!m_menu->isVisible()) {
      QPoint location = this->mapToGlobal(QPoint(this->width() + 4,-4));
      m_menu->popup(location);
      m_menu->show();
    }
}


void VolumeToolButton::slot_volume_change()
{
//     Debug::debug() << "- VolumeToolButton -> slot_volume_change percent: ";
    int volume = Engine::instance()->volume();
    m_volume_label->setText( QString::number( volume ) + '%' );
}

void VolumeToolButton::slot_apply_volume(int vol)
{
//     Debug::debug() << "- VolumeToolButton -> slot_apply_volume : " << vol;

    Engine::instance()->setVolume(vol);
}

void VolumeToolButton::slot_mute_change()
{
//     Debug::debug() << "VolumeToolButton -> volumeMuteChange()";

    if (!Engine::instance()->isMuted())
    {
      this->setIcon(QIcon(":/images/volume-icon.png"));
      (ACTIONS()->value(ENGINE_VOL_MUTE))->setIcon(QIcon(":/images/volume-icon.png"));
    }
    else
    {
      this->setIcon(QIcon(":/images/volume-muted.png"));
      (ACTIONS()->value(ENGINE_VOL_MUTE))->setIcon(QIcon(":/images/volume-muted.png"));
    }
}


void VolumeToolButton::slot_mute_toggle_action()
{
    bool isMuted = Engine::instance()->isMuted();
    Engine::instance()->setMuted(!isMuted);
}


/*
********************************************************************************
*                                                                              *
*    Class PlayerToolBar                                                       *
*                                                                              *
********************************************************************************
*/
PlayerToolBar::PlayerToolBar(QWidget *parent) : QWidget( parent )
{
    m_player = Engine::instance();

    this->setObjectName(QString::fromUtf8("playerToolBar"));
    this->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    this->setPalette(palette);
    this->setAutoFillBackground(true);
    
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
    
    /* -- playing track label -- */
    m_playingTrack = new QLabel(this);
    m_playingTrack->setFont( QApplication::font() );    
    m_playingTrack->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    
    QHBoxLayout *hbl1 = new QHBoxLayout();
    hbl1->setContentsMargins( 0, 0, 0, 0);
    hbl1->setSpacing(1);
    hbl1->setMargin(0);
    hbl1->addWidget(m_currentTime, 0, Qt::AlignVCenter | Qt::AlignLeft);
    hbl1->addWidget(m_separator, 0, Qt::AlignVCenter | Qt::AlignLeft);
    hbl1->addWidget(m_totalTime, 0, Qt::AlignVCenter | Qt::AlignLeft);
    hbl1->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));

    QVBoxLayout *vl1 = new QVBoxLayout();
    vl1->addLayout( hbl1 );
    vl1->addWidget( m_playingTrack );
    
    
    /* -- ToolButton creation -- */
    QToolButton* ui_prev_button = new QToolButton(this);
    ui_prev_button->setDefaultAction(ACTIONS()->value(ENGINE_PLAY_PREV));
    ui_prev_button->setIconSize( QSize( 32, 32 ) );
    ui_prev_button->setAutoRaise(true);
    
    QToolButton* ui_next_button = new QToolButton(this);
    ui_next_button->setDefaultAction(ACTIONS()->value(ENGINE_PLAY_NEXT));
    ui_next_button->setIconSize( QSize( 32, 32 ) );
    ui_next_button->setAutoRaise(true);

    QToolButton* ui_play_button = new QToolButton(this);
    ui_play_button->setDefaultAction(ACTIONS()->value(ENGINE_PLAY));
    ui_play_button->setIconSize( QSize( 32, 32 ) );
    ui_play_button->setAutoRaise(true);

    QToolButton* ui_stop_button = new QToolButton(this);
    ui_stop_button->setDefaultAction(ACTIONS()->value(ENGINE_STOP));
    ui_stop_button->setIconSize( QSize( 32, 32 ) );
    ui_stop_button->setAutoRaise(true);

    QToolButton* ui_equalizer_button = new QToolButton(this);
    ui_equalizer_button->setDefaultAction(ACTIONS()->value(ENGINE_AUDIO_EQ));
    ui_equalizer_button->setIconSize( QSize( 32, 32 ) );
    ui_equalizer_button->setAutoRaise(true);
    
    
    
    //! volume tool button and menu
    VolumeToolButton* ui_volume_button = new VolumeToolButton(this);

  
    //! PlayerToolBar setup
    QHBoxLayout *hb0 = new QHBoxLayout();
    hb0->setContentsMargins( 0, 0, 0, 0);
    hb0->setSpacing(0);     
    hb0->addWidget( new FixedSpacer(this, QSize(5,0)) );
    hb0->addWidget( ui_volume_button );
    hb0->addWidget( ui_equalizer_button );
    hb0->addWidget( new FixedSpacer(this, QSize(10,0)) );
    hb0->addLayout(vl1);
    hb0->addWidget( ui_prev_button );
    hb0->addWidget( ui_play_button );
    hb0->addWidget( ui_stop_button );
    hb0->addWidget( ui_next_button );
    hb0->addItem( new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hb0->addWidget( new FixedSpacer(this, QSize(10,0)) );
    hb0->addWidget( new RepeatControl(this) );
    hb0->addWidget( new ShuffleControl(this) );    
    hb0->addWidget( new FixedSpacer(this, QSize(5,0)) );


    /* -- widget layout -- */
    QVBoxLayout *vl0 = new QVBoxLayout(this);
    vl0->setContentsMargins( 0, 0, 0, 4 );
    vl0->setSpacing(4);
    vl0->addWidget(new SeekSlider(this));
    vl0->addLayout(hb0);
    
    
    /* -- signals connection -- */
    connect(this->m_player, SIGNAL(mediaTick(qint64)), this, SLOT(slot_update_time_position(qint64)));
    connect(this->m_player, SIGNAL(mediaTotalTimeChanged(qint64)), this, SLOT(slot_update_total_time(qint64)));

    connect(this->m_player, SIGNAL(mediaMetaDataChanged()), this, SLOT(slot_update_track_playing_info()));
    connect(this->m_player, SIGNAL(mediaChanged()), this, SLOT(slot_update_track_playing_info()));
    connect(this->m_player, SIGNAL(engineStateChanged()), this, SLOT(slot_update_track_playing_info()));
}


void PlayerToolBar::clear()
{
    //Debug::debug() << "    [PlayerToolBar] clear";
    m_playingTrack->clear();
    m_currentTime->clear();
    m_totalTime->clear();
    m_separator->hide();
}


void PlayerToolBar::slot_update_track_playing_info()
{
    //Debug::debug() << "    [PlayerToolBar] slot_update_track_playing_info";

    if(m_player->state() == ENGINE::STOPPED && !m_player->playingTrack())
    {
        this->clear();
        return;
    }
   
    if(m_player->playingTrack())
    {
        slot_update_total_time( m_player->currentTotalTime() );

        QString playingText;
        MEDIA::TrackPtr track = m_player->playingTrack();
        if(track->type() == TYPE_TRACK)
        {
            playingText = QString(tr("<b>%1</b> by <b>%2</b> on <b>%3</b>"))
                                .arg(track->title,track->artist,track->album);
        }
        else
        {
            if(!track->title.isEmpty() && !track->album.isEmpty() && !track->artist.isEmpty())
            {
                //! after meta data update
                playingText = QString(tr("<b>%1</b> by <b>%2</b> on <b>%3</b>"))
                               .arg(track->title,track->artist,track->album) ;
            }
            else
            {
                //! before meta data update
                playingText = QString( tr("<b>%1</b> stream")).arg(track->name);
            }
        }
    
        QFontMetrics metrix(m_playingTrack->font());
        int width = m_playingTrack->width() - 4;
        QString clippedText = metrix.elidedText(playingText, Qt::ElideRight, width);
        m_playingTrack->setText(clippedText);
   }
}


void PlayerToolBar::slot_update_time_position(qint64 newPos)
{
    //Debug::debug() << "    [PlayerToolBar] slot_update_time_position " << newPos;

    if(m_player->state() == ENGINE::STOPPED && !m_player->playingTrack()) {
       this->clear();
       return;
    }

    if (newPos <= 0) {
        m_currentTime->clear();
        return;
    }

    const QTime displayTime = displayTime.addMSecs(newPos);
    QString timeString;

    if (newPos > 3600000)
        timeString = displayTime.toString("h:mm:ss");
    else
        timeString = displayTime.toString("m:ss");

    m_currentTime->setText(timeString);
}


void PlayerToolBar::slot_update_total_time(qint64 newTotalTime)
{
    //Debug::debug() << "    [PlayerToolBar] slot_update_total_time";

    if(m_player->state() == ENGINE::STOPPED && !m_player->playingTrack()) {
       m_separator->hide();
       return;
    }

    if (newTotalTime <= 0) {
        m_separator->hide();
        m_totalTime->clear();
        return;
    }

    m_separator->show();

    const QTime displayTime = displayTime.addMSecs(newTotalTime);
    QString timeString;

    if (newTotalTime > 3600000)
        timeString = displayTime.toString("h:mm:ss");
    else
        timeString = displayTime.toString("m:ss");

    m_totalTime->setText(timeString);
}

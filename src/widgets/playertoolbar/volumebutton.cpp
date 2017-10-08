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
#include "volumebutton.h"
#include "widgets/playertoolbar/audiocontrols.h"

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
*    Class VolumeButton                                                        *
*                                                                              *
********************************************************************************
*/
VolumeButton::VolumeButton(QWidget *parent) : ToolButtonBase( parent )
{
    this->setIconSize( QSize( 26, 26 ) );
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
    
    int maxVolume = Engine::instance()->maxVolume();

    m_slider->setMaximum(maxVolume);
    m_slider->setRange(0, maxVolume);
    
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
    mainBox->setContentsMargins(8,8,8,8);
    mainBox->addWidget(mute_button, 0, Qt::AlignVCenter | Qt::AlignLeft );
    mainBox->addWidget(m_slider, 0, Qt::AlignVCenter | Qt::AlignLeft );
    mainBox->addWidget(m_volume_label,0, Qt::AlignVCenter | Qt::AlignLeft );


    m_popup = new QWidget(this);
    m_popup->setLayout(mainBox);
    m_popup->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );

    QWidgetAction * sliderActionWidget = new QWidgetAction( this );
    sliderActionWidget->setDefaultWidget( m_popup );

    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    m_popup->setPalette(palette);
    
    /* ---- popup menu ---- */
    m_menu = new QMenu(this);
    m_menu->addAction( sliderActionWidget );
    m_menu->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
    m_menu->setWindowFlags(Qt::Popup);
    m_menu->setStyleSheet( QString ("QMenu {background-color: none;border: none;}") );

      
    /* ---- init  ---- */
    Engine::instance()->setMuted(false);
    
    int volume = Engine::instance()->volume();
    m_volume_label->setText( QString::number( volume ) + '%' );
    
    m_slider->setValue( volume );
    
    /* ---- signals connections ---- */
    connect(Engine::instance() , SIGNAL(muteStateChanged()), this, SLOT(slot_mute_change()));
    connect(Engine::instance() , SIGNAL(volumeChanged()), this, SLOT(slot_volume_change()));
    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(slot_apply_volume(int)));
    connect(this, SIGNAL(clicked()), this, SLOT(slot_show_menu()));
}


void VolumeButton::slot_show_menu()
{
    m_popup->setMinimumHeight( this->height() );

    if( !m_menu->isVisible() )
    {
      QPoint location = this->mapToGlobal(QPoint(this->width() + 4,(this->height()-m_popup->height())/2));
      m_menu->popup(location);
      m_menu->updateGeometry();
      m_menu->show();
    }
}


void VolumeButton::slot_volume_change()
{
    //Debug::debug() << "      [VolumeButton] slot_volume_change percent: ";
    int volume = Engine::instance()->volume();
    m_volume_label->setText( QString::number( volume ) + '%' );
}

void VolumeButton::slot_apply_volume(int vol)
{
    //Debug::debug() << "      [VolumeButton] slot_apply_volume : " << vol;

    Engine::instance()->setVolume(vol);
    if( vol <= 0 )
    {
      this->setIcon(QIcon(":/images/volume-muted.png"));
      (ACTIONS()->value(ENGINE_VOL_MUTE))->setIcon(QIcon(":/images/volume-muted.png"));        
    }
    else if( !Engine::instance()->isMuted() )        
    {
      this->setIcon(QIcon(":/images/volume-icon.png"));
      (ACTIONS()->value(ENGINE_VOL_MUTE))->setIcon(QIcon(":/images/volume-icon.png"));
    }
}

void VolumeButton::slot_mute_change()
{
    //Debug::debug() << "      [VolumeButton] volumeMuteChange";

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


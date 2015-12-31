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

#include "equalizer_slider.h"

#include <QLabel>
#include <QSpacerItem>
#include <QLayout>
/*
********************************************************************************
*                                                                              *
*    Class EqualizerSlider                                                     *
*                                                                              *
********************************************************************************
*/
EqualizerSlider::EqualizerSlider(const QString& name, QWidget *parent)
  : QWidget(parent)
{
    this->resize(33, 224);

    /* slider */
    m_slider = new QSlider(this);
    m_slider->setMinimum( -100 );
    m_slider->setMaximum( 100 );
    m_slider->setOrientation(Qt::Vertical);

    /* equalizer band name */ 
    QLabel *l_name = new QLabel(this);
    l_name->setAlignment(Qt::AlignCenter);
    l_name->setText( name );
    
    /* equalizer value */ 
    l_value = new QLabel(this);
    l_value->setAlignment(Qt::AlignCenter);

    /*  layout */
    QHBoxLayout *hl = new QHBoxLayout();
    hl->addItem( new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum) );
    hl->addWidget( m_slider) ;
    hl->addItem( new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setContentsMargins(0, 0, 0, 0);    
    verticalLayout->addWidget( l_value );
    verticalLayout->addLayout( hl );
    verticalLayout->addWidget( l_name );

    /* signals connection */
    connect(m_slider, SIGNAL(valueChanged(int)), SLOT(slot_on_slider_value_change()));
    connect(m_slider, SIGNAL(sliderPressed()), SLOT(slot_start_mouse_drag()));
    connect(m_slider, SIGNAL(sliderReleased()), SLOT(slot_end_mouse_drag()));

    m_isMouseDrag = false;
}


int EqualizerSlider::value() const
{
    return m_slider->value();
}

void EqualizerSlider::setValue(int value)
{
    m_slider->setSliderPosition(value);
}

void EqualizerSlider::slot_on_slider_value_change()
{
    l_value->setText( QString::number( value() ) );
    
    if( !m_isMouseDrag )
      emit valueChanged( value() );
}

void EqualizerSlider::slot_start_mouse_drag()
{
    m_isMouseDrag = true;

    l_value->setText( QString::number( value() ) );
}

void EqualizerSlider::slot_end_mouse_drag()
{
    m_isMouseDrag = false;

    emit valueChanged( value() );
}

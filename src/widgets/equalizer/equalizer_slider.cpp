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
    //! ----- setup Gui
    this->resize(33, 224);

    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    QSpacerItem *horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout->addItem(horizontalSpacer);

    _slider = new QSlider(this);
    _slider->setMinimum(-100);
    _slider->setMaximum(100);
    _slider->setOrientation(Qt::Vertical);

    horizontalLayout->addWidget(_slider);

    QSpacerItem  *horizontalSpacer_2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout->addItem(horizontalSpacer_2);

    verticalLayout->addLayout(horizontalLayout);

    QLabel *label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    label->setText(name);

    verticalLayout->addWidget(label);

    //! ----- Connection
    connect(_slider, SIGNAL(valueChanged(int)), SLOT(emitValueChanged()));
    connect(_slider, SIGNAL(sliderPressed()), SLOT(startMouseDrag()));
    connect(_slider, SIGNAL(sliderReleased()), SLOT(endMouseDrag()));

    _isMouseDrag = false;
}


int EqualizerSlider::value() const
{
  return _slider->value();
}

void EqualizerSlider::setValue(int value)
{
  _slider->setSliderPosition(value);
}

void EqualizerSlider::emitValueChanged()
{
  if(!_isMouseDrag)
    emit valueChanged(_slider->value());
}

void EqualizerSlider::startMouseDrag()
{
  _isMouseDrag = true;
}

void EqualizerSlider::endMouseDrag()
{
  _isMouseDrag = false;
  emit valueChanged(_slider->value());
}

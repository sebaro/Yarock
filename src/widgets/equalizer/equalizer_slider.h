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

#ifndef _EQUALIZER_SLIDER_H_
#define _EQUALIZER_SLIDER_H_

//! qt
#include <QWidget>
#include <QSlider>

/*
********************************************************************************
*                                                                              *
*    Class EqualizerSlider                                                     *
*                                                                              *
********************************************************************************
*/
class EqualizerSlider : public QWidget {
  Q_OBJECT

 public:
  EqualizerSlider(const QString& name, QWidget *parent = 0);

  int value() const;
  void setValue(int value);

 signals:
  void valueChanged(int value);

 private:
  QSlider *_slider;
  bool _isMouseDrag;

 private slots:
  void emitValueChanged();
  void startMouseDrag();
  void endMouseDrag();

};

#endif // _EQUALIZER_SLIDER_H_

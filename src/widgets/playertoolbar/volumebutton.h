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

#ifndef _VOLUME_BUTTON_H_
#define _VOLUME_BUTTON_H_

// Qt
#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QSlider>
#include <QMenu>


/*
********************************************************************************
*                                                                              *
*    Class VolumeButton                                                        *
*                                                                              *
********************************************************************************
*/
class VolumeButton : public QToolButton
{
Q_OBJECT
  public:
    VolumeButton(QWidget *);
  
  private slots:
    void slot_show_menu();
    void slot_volume_change();
    void slot_mute_change();
    void slot_apply_volume(int);
    
  private:
    QLabel       *m_volume_label;
    QSlider      *m_slider;
    QMenu        *m_menu;
};

#endif // _VOLUME_BUTTON_H_

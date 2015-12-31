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

#ifndef _TOOLBUTTONBASE_H_
#define _TOOLBUTTONBASE_H_

// Qt
#include <QWidget>
#include <QColor>
#include <QToolButton>
#include "settings.h"


/*
********************************************************************************
*                                                                              *
*    Class ToolButtonBase                                                      *
*                                                                              *
********************************************************************************
*/
class ToolButtonBase : public QToolButton
{
  public:
    ToolButtonBase(QWidget *w) : QToolButton(w)
    {
        setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
        setAutoRaise(true);

        QColor color = SETTINGS()->_baseColor;
        
        qreal saturation = color.saturationF();
        saturation *= 0.5;

        color.setHsvF( color.hueF(), saturation, color.valueF(), color.alphaF() );
    
        setStyleSheet(
          QString("QToolButton::hover { border: none; border-radius: 2px; background-color:%1;}")
          .arg(color.name())
        );    
    };
};

#endif // _TOOLBUTTONBASE_H_

/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2018 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

        setStyleSheet( QString("QToolButton::hover { border: none} QToolButton::checked { border: none;background-color:none}") );
    };


    ToolButtonBase(QWidget *w, QSizePolicy policy, Qt::ToolButtonStyle style=Qt::ToolButtonTextBesideIcon) : QToolButton(w)
    {
        setSizePolicy(policy);
        setAutoRaise(true);
        setToolButtonStyle(style);
 
        
        setStyleSheet( QString("QToolButton::hover { border: none} QToolButton::checked { border: none;background-color:none}") );
    };    
};

#endif // _TOOLBUTTONBASE_H_

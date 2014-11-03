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

#include "header_widget.h"

#include <QPainter>
#include <QApplication>

/*
********************************************************************************
*                                                                              *
*    Class HeaderWidget                                                        *
*                                                                              *
********************************************************************************
*/
HeaderWidget::HeaderWidget( QWidget* parent ) : QWidget(parent)
{
    m_color = QApplication::palette().color( QPalette::Window );
}

void HeaderWidget::paintEvent(QPaintEvent *event)
{
Q_UNUSED(event)
    QPainter customPainter(this);
    customPainter.fillRect(rect(),m_color);
}
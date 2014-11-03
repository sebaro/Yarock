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

#ifndef _HEADER_WIDGET_H_
#define _HEADER_WIDGET_H_

#include <QWidget>
#include <QObject>
#include <QApplication>

/*
********************************************************************************
*                                                                              *
*    Class HeaderWidget                                                        *
*                                                                              *
********************************************************************************
*/
class HeaderWidget : public QWidget
{
Q_OBJECT
public:
  HeaderWidget(QWidget *parent=0);
  void setColor(QColor c) {m_color = c;}
  
protected:
  void paintEvent(QPaintEvent*);

private:
  QColor   m_color;
};

#endif // _HEADER_WIDGET_H_

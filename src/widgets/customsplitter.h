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

#ifndef _CUSTOM_SPLITTER_H_
#define _CUSTOM_SPLITTER_H_

#include <QSplitter>

class QSplitterHandle;
/*
********************************************************************************
*                                                                              *
*    Class CustomSplitter                                                      *
*                                                                              *
********************************************************************************
*/

class CustomSplitter : public QSplitter
{
  public:
    CustomSplitter(QWidget *parent = 0);

  protected:
    QSplitterHandle *createHandle();
    void paintEvent(QPaintEvent *event);
};

#endif // _CUSTOM_SPLITTER_H_


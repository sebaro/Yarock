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

#ifndef _MENU_WIDGET_H_
#define _MENU_WIDGET_H_

#include "customsplitter.h"
#include "menubar.h"

#include <QPushButton>
#include <QStackedWidget>
#include <QWidget>

/*
********************************************************************************
*                                                                              *
*    Class MenuWidget                                                          *
*                                                                              *
********************************************************************************
*/
class MenuWidget : public QWidget
{
Q_OBJECT
    static MenuWidget* INSTANCE;

public:
  MenuWidget(QWidget *parent = 0);
  static MenuWidget* instance() { return INSTANCE; }
    
  void setSplitter(CustomSplitter* s) {m_splitter = s;}

  void restoreState();
  
private slots:
  void slot_change_menu_style();
  
private :
  MenuBar           *m_menuBar;
  QStackedWidget    *m_stackedWidget;
  QWidget           *m_parent;
  CustomSplitter*    m_splitter;
  
  QPushButton       *ui_control_button;
}; 


#endif //_MENU_WIDGET_H_
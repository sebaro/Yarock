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

#ifndef _MAIN_TOOL_BUTTON_H_
#define _MAIN_TOOL_BUTTON_H_

#include <QToolButton>
#include <QSignalMapper>
#include <QActionGroup>
#include <QAction>
#include <QMenu>

/*
********************************************************************************
*                                                                              *
*    Class MainToolButton                                                      *
*                                                                              *
********************************************************************************
*/
class MainToolButton : public QToolButton
{
 Q_OBJECT
  public:
    MainToolButton(QWidget *parent);

  private:
   void updateMultiDbMenu();

  private slots:
     void slot_dbNameClicked();
     void slot_showMenu();

  signals:
    void dbNameChanged();

  private:
    QMenu               *m_menu;
    QMenu               *m_menuChooseDb;
    QAction             *m_menuChooseDbAction;
};


#endif //_MAIN_TOOL_BUTTON_H_

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

#ifndef _MENU_MODEL_H_
#define _MENU_MODEL_H_

#include "views.h"

#include <QStandardItemModel>
#include <QModelIndex>
#include <QAction>


enum MenuItemRole {
    GlobalActionRole    = Qt::UserRole + 1,
    MenuActionRole      = Qt::UserRole + 2,
    ViewModeRole        = Qt::UserRole + 3,
    DataRole            = Qt::UserRole + 4,
    DatabaseChooseRole  = Qt::UserRole + 5
};


class MenuItem;
/*
********************************************************************************
*                                                                              *
*    Class MenuModel                                                           *
*                                                                              *
********************************************************************************
*/
class MenuModel : public QStandardItemModel
{
Q_OBJECT
  static MenuModel *INSTANCE;

  public:
    MenuModel(QObject *parent = 0);
    ~MenuModel();

    static MenuModel* instance() { return INSTANCE; }
    
    QAction* modelIndexAction( QModelIndex idx );
        
    QModelIndex settingsModelIdx();
    
  private:
    MenuItem        *m_db_item;
    MenuItem        *m_settings_item;
    
    void populateChooseDatabase();
    void configureMenuAction();
    void populateMenu();

  private slots:
    void slot_on_menu_triggered();
    void slot_dbNameClicked();
    void slot_database_settingsChanged();

  signals :
    void menu_browser_triggered(VIEW::Id, QVariant);
    void dbNameChanged();
    void databaseMenuChanged();
};

Q_DECLARE_METATYPE(QAction*);
Q_DECLARE_METATYPE(QModelIndex);


#endif // _MENU_MODEL_H_

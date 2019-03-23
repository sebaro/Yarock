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

#ifndef _DATABASE_ADD_DIALOG_H_
#define _DATABASE_ADD_DIALOG_H_

#include "dialog_base.h"

#include <QLineEdit>
#include <QWidget>
/*
********************************************************************************
*                                                                              *
*    Class DatabaseAddDialog                                                   *
*                                                                              *
********************************************************************************
*/
class ExLineEdit;
class AddFolderWidget;

class DatabaseAddDialog : public DialogBase
{
Q_OBJECT
Q_DISABLE_COPY(DatabaseAddDialog)

public:
    DatabaseAddDialog(QWidget *parent = 0);

    QString newDatabaseName();
    
    void setFirstStart() {isFirstStart = true;}
    
protected:
    void showEvent ( QShowEvent * event );
    
private:
    AddFolderWidget          *ui_add_folder;
    ExLineEdit               *ui_line_edit;
    QLabel                   *ui_info_label;
    bool                     isFirstStart;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
};

#endif // _DATABASE_ADD_DIALOG_H_

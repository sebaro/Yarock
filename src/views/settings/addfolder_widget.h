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

#ifndef _ADD_FOLDER_WIDGET_H_
#define _ADD_FOLDER_WIDGET_H_


#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QString>

/*
********************************************************************************
*                                                                              *
*    Class AddFolderWidget                                                     *
*                                                                              *
********************************************************************************
*/
class AddFolderWidget : public QWidget
{
Q_OBJECT
  public:
    AddFolderWidget(QWidget *parent = 0);
    
    QString path() {return ui_file_path->text();}
    void setText(const QString & text) {
      ui_file_path->setText(text);
      ui_file_path->home(false);    
    }

    void hideRemoveButton() {ui_remove_button->hide();}

  private:
    QLineEdit    *ui_file_path;
    QPushButton  *ui_open_button;
    QPushButton  *ui_remove_button;

  private slots:
    void slot_open_folder();
    
  signals:
    void removedClicked();
    void pathChanged();
};

#endif // _ADD_FOLDER_WIDGET_H_

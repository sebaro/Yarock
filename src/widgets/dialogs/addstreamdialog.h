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

#ifndef _ADDSTREAM_DIALOG_H_
#define _ADDSTREAM_DIALOG_H_

#include "dialog_base.h"

// #include <QDialog>
// #include <QWidget>
#include <QLineEdit>

/*
********************************************************************************
*                                                                              *
*    Class AddStreamDialog                                                     *
*                                                                              *
********************************************************************************
*/
//class AddStreamDialog : public QDialog
class AddStreamDialog : public DialogBase
{
Q_OBJECT
  public:
    AddStreamDialog(QWidget *parent = 0,bool active_category=false);
    const QString url() const;
    const QString name() const;
    const QString category() const;

    void setUrl(const QString& url);
    void setName(const QString& name);
    void setCategory(const QString& categorie);

  private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();

  private:
     QLineEdit         *ui_edit_url;
     QLineEdit         *ui_edit_name;
     QLineEdit         *ui_edit_category;
};

#endif // _ADDSTREAM_DIALOG_H_


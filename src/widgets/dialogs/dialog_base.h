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

#ifndef _DIALOG_BASE_H_
#define _DIALOG_BASE_H_

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLayout>
#include <QWidget>
#include <QLabel>
#include <QString>

/*
********************************************************************************
*                                                                              *
*    Class DialogBase                                                          *
*                                                                              *
********************************************************************************
*/
class DialogBase : public QDialog
{
public:
  DialogBase(QWidget *parent = 0, const QString& title=QString());
  void setContentLayout(QLayout*, bool isSpacer=true);
  void setContentWidget(QWidget*, bool isSpacer=true);
  
  QDialogButtonBox* buttonBox() {return ui_buttonBox;}
  
  void setTitle(const QString& title);
  
private:  
  QVBoxLayout      *ui_main_layout;    
  QDialogButtonBox *ui_buttonBox;  
  QLabel           *ui_title_label;
};


/*
********************************************************************************
*                                                                              *
*    Class DialogMessage                                                       *
*                                                                              *
********************************************************************************
*/
class DialogMessage : public DialogBase
{
public:
  DialogMessage(QWidget *parent = 0, const QString& title=QString());
  void setMessage(const QString&);

private:
  QLabel       *ui_message_label;  
};

/*
********************************************************************************
*                                                                              *
*    Class DialogQuestion                                                      *
*                                                                              *
********************************************************************************
*/
class DialogQuestion : public DialogBase
{
Q_OBJECT  
public:
  DialogQuestion(QWidget *parent = 0, const QString& title=QString());
  void setQuestion(const QString&);


private slots:  
  void on_buttonBox_clicked(QAbstractButton * button);
  
private:
  QLabel       *ui_question_label;  
};

/*
********************************************************************************
*                                                                              *
*    Class DialogInput                                                         *
*                                                                              *
********************************************************************************
*/
class DialogInput : public DialogBase
{
public:
  DialogInput(QWidget *parent = 0, const QString& label=QString(), const QString& title=QString());
  QString editValue();
  void setEditValue(const QString&);
  
private:
  QLabel       *ui_label_text;  
  QLineEdit    *ui_line_edit;  
};


#endif // _DIALOG_BASE_H_
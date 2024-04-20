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

#ifndef _SORT_WIDGET_H_
#define _SORT_WIDGET_H_

#include "search_query.h"


#include <QComboBox>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QMenu>
#include <QWidget>

/*
********************************************************************************
*                                                                              *
*    Class SortWidget                                                          *
*                                                                              *
********************************************************************************
*/
class SortWidget : public QWidget
{
Q_OBJECT
public:
  SortWidget(QMenu *parent);

private slots:
  void slot_updateSortOrder(bool first_start =false);
  void on_buttonBox_clicked(QAbstractButton * button);
  
signals:
  void triggered(QVariant);
  
private:
  QMenu                    *m_parent;
  QDialogButtonBox         *ui_buttonBox;
  QComboBox                *ui_sort_field;
  QComboBox                *ui_sort_order;
};


#endif //_SORT_WIDGET_H_

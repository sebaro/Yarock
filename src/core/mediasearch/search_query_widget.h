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

#ifndef _SEARCH_QUERY_WIDGET_H_
#define _SEARCH_QUERY_WIDGET_H_


#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QTimeEdit>
#include <QDateEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QStackedWidget>


class RatingWidget;
class SearchQuery;
/*
********************************************************************************
*                                                                              *
*    Class Search_Query_Widget                                                 *
*                                                                              *
********************************************************************************
*/
class Search_Query_Widget : public QWidget
{
Q_OBJECT
  public:
    Search_Query_Widget(QWidget *parent = 0);
    SearchQuery query();
    void set_query(SearchQuery& query);

  private:
    QComboBox*        ui_search_field;
    QComboBox*        ui_search_rules;

    QLineEdit*        ui_line_edit;     // for string edition
    QSpinBox*         ui_spin_box;      // for number edition
    QTimeEdit*        ui_time_edit;     // for time edition
    QDateEdit*        ui_date_edit;     // for date edition
    RatingWidget*     ui_rating_widget; // for rating edition

    QPushButton*      ui_remove_button;
    QStackedWidget*   ui_stacked_widget;

    QWidget*          ui_page_text;
    QWidget*          ui_page_number;
    QWidget*          ui_page_time;
    QWidget*          ui_page_date;
    QWidget*          ui_page_rating;

  private slots:
    void slotFieldChanged(int i);
    void slotOperatorChanged(int);
  signals:
    void signalRemoveClicked();
};

#endif //_SEARCH_QUERY_WIDGET_H_

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

#ifndef _EDITOR_SEARCH_H_
#define _EDITOR_SEARCH_H_


#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QKeyEvent>

class Search_Query_Widget;

/*
********************************************************************************
*                                                                              *
*    Class EditorSearch                                                        *
*                                                                              *
********************************************************************************
*/
class EditorSearch : public QWidget
{
Q_OBJECT
  public:
    EditorSearch(QWidget *parent = 0);

    bool isActive() { return m_isActive;}
    
    void set_search(QVariant v_search);
    
    QVariant get_search();
    
  private:
    QDialogButtonBox      *ui_buttonBox;
    
    QScrollArea           *ui_scrollarea;
    QComboBox             *ui_search_mode;
    QVBoxLayout           *ui_search_query_layout;
    QPushButton           *ui_add_search_button;

    QList<Search_Query_Widget*>   listSearchWidget;

    bool       m_isActive;

  private:
    QWidget*   create_ui();
    bool       is_search_valid();
    void add_search_query_widget(Search_Query_Widget* widget);
    void remove_search_query_widget(Search_Query_Widget* widget);

  private slots:
    void on_buttonBox_clicked(QAbstractButton *);
    void on_button_AddSearch_clicked();
    void slot_remove_search_query_widget();
    void slot_search_mode_change();
    void keyPressEvent(QKeyEvent* event);

  signals:
    void search_triggered();
};



#endif // _EDITOR_SEARCH_H_

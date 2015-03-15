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

#ifndef _EDITOR_SMART_H_
#define _EDITOR_SMART_H_


#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QSpinBox>

#include "core/mediasearch/media_search.h"
#include "core/mediaitem/mediaitem.h"

class Search_Query_Widget;
class ExLineEdit;
/*
********************************************************************************
*                                                                              *
*    Class EditorSmart                                                         *
*                                                                              *
********************************************************************************
*/
class EditorSmart : public QScrollArea
{
Q_OBJECT
  public:
    EditorSmart(QWidget *parent = 0);

    void setPlaylist(MEDIA::PlaylistPtr playlist);
    
//     void set_search(MediaSearch& media_search, int dbId);
    MediaSearch get_search();

  private:
    MEDIA::PlaylistPtr     m_playlist;
    
    QDialogButtonBox*     ui_buttonBox;
    ExLineEdit           *ui_edit_name;
    
    QComboBox*            ui_search_mode;
    QVBoxLayout*          ui_search_query_layout;
    QPushButton*          ui_add_search_button;

    QList<Search_Query_Widget*>   listSearchWidget;

    QRadioButton*         ui_check_sort_no;
    QRadioButton*         ui_check_sort_random;
    QRadioButton*         ui_check_sort_by;
    QComboBox*            ui_sort_field;
    QComboBox*            ui_sort_order;

    QRadioButton*         ui_check_limit_no;
    QRadioButton*         ui_check_limit_at;
    QSpinBox*             ui_spintime_limit;

    bool starting;
    int m_databaseId;

  private:
    QLayout*   create_ui();
    bool       is_search_valid();
    int        limit();
    MediaSearch::SortType sortType();

  private slots:
    void on_buttonBox_clicked(QAbstractButton * button);
    void on_button_AddSearch_clicked();
    void add_search_query_widget(Search_Query_Widget* widget);
    void slot_remove_search_query_widget();
    void slot_search_mode_change();
    void slot_updateSortOrder();

  signals:
    void close(); 
    void new_smart_playlist(MediaSearch, int );
};



#endif // _EDITOR_SMART_H_

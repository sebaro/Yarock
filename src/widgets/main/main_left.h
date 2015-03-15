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

#ifndef _MAIN_LEFT_H_
#define _MAIN_LEFT_H_

#include "header_widget.h"
#include "widgets/customsplitter.h"
#include "views.h"

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>

class BrowserView;
class SearchLineEdit;
class SearchPopup;
class EditorSearch;
/*
********************************************************************************
*                                                                              *
*    Class MainLeftWidget                                                      *
*                                                                              *
********************************************************************************
*/
class MainLeftWidget : public QWidget
{
Q_OBJECT
    static MainLeftWidget* INSTANCE;
    
public:
    MainLeftWidget(QWidget *parent=0);
    
    static MainLeftWidget* instance() { return INSTANCE; }
    
    void setBrowser(BrowserView*);
    
    void setTitle(const QString&);

    void setBrowserSearch(QVariant);
    QVariant browserSearch();

    void setMode(VIEW::Id mode);

    CustomSplitter* splitter() {return m_viewsSplitter;}
    
private slots:
    void slot_send_quick_filter_change();
    void slot_explorer_popup_setting_change();
    void slot_advance_search_clicked();
    void slot_advanced_search_triggered();
    
signals:
    void browser_search_change(const QVariant&);
    
    void dbNameChanged();
    void settings_save_clicked();
    void settings_cancel_clicked();
     
private:
    void create_header_ui();
    void create_ui_search();
    
    VIEW::Id              m_mode;
    HeaderWidget         *m_header;
    CustomSplitter       *m_viewsSplitter;
    
    /* for header */
    EditorSearch         *ui_editor_search;
    SearchLineEdit       *ui_search_lineedit;
    SearchPopup          *ui_popup_completer;

    HeaderWidget         *m_centralHeader;
    QLabel               *m_title;
    
    QToolButton          *ui_button_up;
    QToolButton          *ui_button_prev;
    QToolButton          *ui_button_next;
    
    QPushButton          *ui_save_button;
    QPushButton          *ui_cancel_button;
    
    QPushButton          *ui_advance_search_button;
};

#endif // _MAIN_LEFT_H_

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
#ifndef _SEARCH_POPUP_H_
#define _SEARCH_POPUP_H_

#include "exlineedit.h"

#include <QFrame>
#include <QWidget>
#include <QTimer>
#include <QEvent>

class PopupModel;
class PopupView;

/*
********************************************************************************
*                                                                              *
*    Class SearchPopup                                                         *
*                                                                              *
********************************************************************************
*/
class SearchPopup : public QFrame
{
Q_OBJECT
public:
    SearchPopup(ExLineEdit *parent = 0);
    ~SearchPopup();
    bool eventFilter(QObject *obj, QEvent *ev);

private slots:
    void slot_popup_item_clicked(const QString&);
    void slot_exec_popup();
    void slot_on_new_search();

protected:
    void hideEvent ( QHideEvent * );

private:
    void show_popup();

private:
    QString            m_old_search;
    ExLineEdit         *m_line_edit;
    QTimer            *m_timer;
    bool               m_enableUpdate;

    QWidget*           m_parent;
    PopupModel*        m_popup_model;
    PopupView*         m_popup_view;
};

#endif // _SEARCH_POPUP_H_

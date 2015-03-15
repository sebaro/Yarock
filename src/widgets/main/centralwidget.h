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

#ifndef _CENTRAL_WIDGET_H_
#define _CENTRAL_WIDGET_H_

#include "views/browser_view.h"

#include "main_right.h"
#include "main_left.h"


#include "widgets/customsplitter.h"
#include "views.h"

#include <QWidget>
#include <QFrame>

class PlaylistWidget;
/*
********************************************************************************
*                                                                              *
*    Class CentralWidget                                                       *
*                                                                              *
********************************************************************************
*/
class CentralWidget : public QFrame
{
Q_OBJECT
public:
    CentralWidget(QWidget *,PlaylistWidget*);

    void saveState();
    void setBrowser(BrowserView* b) {left_widget->setBrowser(b);}

public slots:
    void restoreState();

private slots:
    void slot_show_playlist();
    void slot_show_menu();

private :
    QWidget             *m_parent;

    MainLeftWidget      *left_widget;
    MainRightWidget     *right_widget;
    
    CustomSplitter      *m_viewsSplitter_1;
};


#endif // _CENTRAL_WIDGET_H_

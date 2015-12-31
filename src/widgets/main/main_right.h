/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2016 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

#ifndef _MAIN_RIGHT_H_
#define _MAIN_RIGHT_H_

#include "header_widget.h"

#include <QWidget>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QMap>
#include <QMenu>

class PlaylistWidgetBase;
/*
********************************************************************************
*                                                                              *
*    Class MainRightWidget                                                     *
*                                                                              *
********************************************************************************
*/
class MainRightWidget: public QObject
{
Q_OBJECT
    static MainRightWidget* INSTANCE;
    
public:
    MainRightWidget(QWidget *parent=0);
    
    static MainRightWidget* instance() { return INSTANCE; }
      
    void addWidget(QWidget* widget, bool activate=true);
    
    QWidget * contentWidget() {return m_stackedWidget;}
    QWidget * headerWidget()  {return m_header;}
    
    QList<PlaylistWidgetBase*> playqueueList();
    
    PlaylistWidgetBase* activePlayqueue();
    
public slots:
    void slot_create_new_playlist_editor();
    void slot_create_new_smart_editor();
    
private slots:
    void slot_button_clicked(QAbstractButton*);
    void slot_widget_closed();
    void slot_onmenu_clicked();

private:
    enum WidgetType {
      PLAYQUEUE,
      TAG_EDIT,
      PLAYLIST_EDIT,
      SMART_EDIT,
    };
    
    void addWidget(WidgetType type, QWidget* widget, bool activate);

private:
    QWidget             *m_parent;            
    HeaderWidget        *m_header;
    QButtonGroup        *m_buttons;
    QStackedWidget      *m_stackedWidget;
    
    QMenu               *m_right_menu;
    
    int                 m_buttonId;
    QMap<int /*buttons*/, QWidget* /*widgets*/>  m_ids;
    
signals:
    void playqueueAdded(QWidget*);
    void playqueueRemoved(QWidget*);
};

#endif // _MAIN_RIGHT_H_ 

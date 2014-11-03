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

#ifndef _PLAYLISTWIDGET_H_
#define _PLAYLISTWIDGET_H_

#include "playlistview.h"
#include "playqueue_model.h"

#include <QMenu>
#include <QSignalMapper>
#include <QActionGroup>
#include <QAction>

class ExLineEdit;
class HeaderWidget;
class SortWidget;
class NowPlayingView;
/*
********************************************************************************
*                                                                              *
*    Class PlaylistWidget                                                      *
*                                                                              *
********************************************************************************
*/
class PlaylistWidget : public QWidget
{
Q_OBJECT
  public:
    PlaylistWidget(QWidget *parent, PlaylistView *view, PlayqueueModel* model);

  private:
    void contextMenuEvent ( QContextMenuEvent * event );

  private slots:
    void slot_update_filter();
    void slot_update_playqueue_actions();
    void slot_update_playqueue_status_info();
    void slot_removeduplicate_changed();
    
    void slot_show_now_playing_triggered();
    void slot_show_filter_triggered();
    void slot_show_sortmenu();

  private :
    PlaylistView       *m_view;
    PlayqueueModel     *m_model;
    QMenu              *m_menu;
    QMenu              *m_sort_menu;
    SortWidget         *ui_sort_widget;
    NowPlayingView     *m_nowplaying;
    
    ExLineEdit         *ui_playqueue_filter;
    QAction            *m_action_show_filter;
    
//     HeaderWidget       *ui_filter_container;
    QWidget  *ui_filter_container;
};


#endif // _PLAYLISTWIDGET_H_

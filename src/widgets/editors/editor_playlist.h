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

#ifndef _EDITOR_PLAYLIST_H_
#define _EDITOR_PLAYLIST_H_

#include "playqueue/playlistwidgetbase.h"
#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/mediamimedata.h"

#include "widgets/exlineedit.h"

#include <QWidget>
#include <QObject>
#include <QDialogButtonBox>
#include <QMenu>

class TaskManager;
/*
********************************************************************************
*                                                                              *
*    Class EditorPlaylist                                                      *
*                                                                              *
********************************************************************************
*/
class EditorPlaylist : public QWidget, public PlaylistWidgetBase
{
Q_OBJECT

public:
  EditorPlaylist(QWidget* parent = 0);
  ~EditorPlaylist();

  void setPlaylist(MEDIA::PlaylistPtr playlist);

private slots:
  void slot_save_playlist();  
  void slot_on_playlist_load(MediaMimeData*, int);
  void slot_on_buttonbox_clicked(QAbstractButton * button);

private:
  MEDIA::PlaylistPtr     m_playlist;

  TaskManager           *m_task_manager;

  QMenu                 *m_menu;
  ExLineEdit            *ui_edit_name;
  QDialogButtonBox      *ui_buttonBox;
    
signals:
  void playlistSaved();
  void close();
};

#endif // _EDITOR_PLAYLIST_H_

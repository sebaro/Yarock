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

#ifndef _EDITOR_PLAYLIST_H_
#define _EDITOR_PLAYLIST_H_

#include "core/mediaitem/mediaitem.h"
#include "widgets/exlineedit.h"

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QObject>
#include <QList>
#include <QUrl>
#include <QAction>
#include <QDialogButtonBox>
#include <QMenu>
#include <QToolButton>
#include <QLabel>

class PlayqueueModel;
class PlaylistView;
class TaskManager;


/*
********************************************************************************
*                                                                              *
*    Class EditorPlaylist                                                      *
*                                                                              *
********************************************************************************
*/
class EditorPlaylist : public QWidget
{
Q_OBJECT

public:
  EditorPlaylist(QWidget* parent = 0);
  ~EditorPlaylist();

  PlayqueueModel* model() {return m_model;}

  void setPlaylist(MEDIA::PlaylistPtr playlist);

private:
  void create_ui();

  // Playlist Populator Thread
  void playlistAddFiles(const QStringList &files);
  void playlistAddFile(const QString &file);
  void playlistAddUrls(QList<QUrl> listUrl, int playlist_row=-1);

  // Playlist Writer Thread
  void playlistSaveToFile(const QString &filename);
  void playlistSaveToDb(const QString &name);

protected:
  void contextMenuEvent ( QContextMenuEvent * event );  

private slots:
  void slot_on_buttonbox_clicked(QAbstractButton * button);
  void slot_on_type_change(int);
  void slot_dialog_file_to_save();
  
  void slot_add_file();
  void slot_add_dir();
  void slot_add_url();

private:
  MEDIA::PlaylistPtr     m_playlist;

  TaskManager           *m_task_manager;
  PlayqueueModel        *m_model;
  PlaylistView          *m_view;

  QMenu                 *m_menu;
  QWidget               *ui_save_to_file_widget;
  ExLineEdit            *ui_edit_name;
  QLineEdit             *ui_file_path;
  QToolButton           *ui_file_button;
  QComboBox             *ui_combo_type;
  QDialogButtonBox      *ui_buttonBox;
    
signals:
  void playlistSaved();
  void close();
};

#endif // _EDITOR_PLAYLIST_H_
 

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

#ifndef _EDITOR_STREAM_H_
#define _EDITOR_STREAM_H_

#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/mediamimedata.h"

#include "widgets/editors/editor_common.h"

#include <QWidget>
#include <QObject>
#include <QLabel>
#include <QDialogButtonBox>

class FavoriteStreams;
class QGraphicsScene;

/*
********************************************************************************
*                                                                              *
*    Class EditorStream                                                        *
*                                                                              *
********************************************************************************
*/
class EditorStream : public QWidget
{
Q_OBJECT

public:
  EditorStream(MEDIA::TrackPtr stream,FavoriteStreams* fvmodel,QGraphicsScene* scene);
  ~EditorStream();

private:
  void create_ui();
  void do_changes();
  void save_new_image();

private slots:
  void slot_on_buttonbox_clicked(QAbstractButton * button);

  void slot_load_image_from_file();
  void slot_image_remove();
    
private:
  MEDIA::TrackPtr        m_stream;
  FavoriteStreams       *m_fvs_model;
  QGraphicsScene        *m_scene;

  bool                   m_isChanged;
  bool                   m_isImageChange;

  QImage                 m_new_image;  

  QLabel                *ui_headertitle;
  QLabel                *ui_image;
  EdLineEdit            *ui_edit_name;
  EdLineEdit            *ui_edit_url;
  EdLineEdit            *ui_edit_genre;
  EdLineEdit            *ui_edit_website;
  EdLineEdit            *ui_edit_bitrate;
  EdLineEdit            *ui_edit_samplerate;
  EdLineEdit            *ui_edit_format;
  
  QDialogButtonBox      *ui_buttonBox;
    
signals:
  void close();
};

#endif // _EDITOR_STREAM_H_

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

#ifndef _EDITOR_TRACK_H_
#define _EDITOR_TRACK_H_

#include "core/mediaitem/mediaitem.h"
#include "infosystem/info_system.h"

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QLayout>

class QGraphicsScene;
class RatingWidget;
class EdTrackEdit;
class EdMultiFieldEdit;
/*
********************************************************************************
*                                                                              *
*    Class EditorTrack                                                         *
*                                                                              *
********************************************************************************
*/

class EditorTrack : public QWidget
{
Q_OBJECT
  public:
    EditorTrack(MEDIA::TrackPtr track, QGraphicsScene* scene=0);
    EditorTrack(QList<MEDIA::TrackPtr> tracks, QGraphicsScene* scene=0);
    ~EditorTrack();

    bool isChanged() {return m_isChanged;}
    
  private:
    void create_ui();
    void do_changes_track();
    void init();

  private:
    QList<MEDIA::TrackPtr>  m_tracks;
    
    QGraphicsScene    *m_scene;
    
    bool               m_isChanged;

    QLabel            *ui_headertitle;
    
    QDialogButtonBox  *ui_buttonBox;

    QLineEdit         *ui_artist_name;
    QLineEdit         *ui_album_name;
    
    QLineEdit         *ui_filename;
    QLineEdit         *ui_track_title;
    QLineEdit         *ui_track_genre;
    QLineEdit         *ui_track_number;
    QDateEdit         *ui_track_year;

    QSpinBox          *ui_playcount;
    RatingWidget      *ui_rating;

    QMap<QString, EdMultiFieldEdit*> ui_multifields;
    
  private slots:
    void slot_on_buttonbox_clicked(QAbstractButton * button);
    
signals:
  void close();    
};

#endif // _EDITOR_TRACK_H_

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

#ifndef _EDITOR_ARTIST_H_
#define _EDITOR_ARTIST_H_

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
/*
********************************************************************************
*                                                                              *
*    Class EditorArtist                                                        *
*                                                                              *
********************************************************************************
*/

class EditorArtist : public QWidget
{
Q_OBJECT
  public:
    EditorArtist(MEDIA::ArtistPtr artist,QGraphicsScene* scene=0);
    ~EditorArtist();

    bool isChanged() {return m_isChanged;}
    
  private:
    void create_ui();
    void do_changes_artist();

    void save_new_image();
    void recoverCoverImage(const QString &, const QString &);

  private:
    MEDIA::ArtistPtr     m_artist;
    QGraphicsScene      *m_scene;
    
    bool                 m_isChanged;
    bool                 m_isImageChange;

    QImage               m_new_image;
    
    QLabel               *ui_headertitle;
    
    QDialogButtonBox     *ui_buttonBox;
    QLineEdit            *ui_artist_name;

    QLabel               *ui_image;

    QSpinBox             *ui_playcount;
    RatingWidget         *ui_rating;
    QPushButton          *ui_clear_rating;

    QList<quint64>        m_requests_ids;
    
  private slots:
    void slot_on_buttonbox_clicked(QAbstractButton * button);

    void slot_update_rating_widget(bool isUser=false);    
    void slot_rating_changed(float);
    
    void slot_load_image_from_file();
    void slot_download_image();
    void slot_on_image_received(INFO::InfoRequestData, QVariant );
    void slot_image_remove();
    
signals:
  void close();    
};

#endif // _EDITOR_ARTIST_H_
 

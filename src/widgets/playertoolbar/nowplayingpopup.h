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

#ifndef _NOWPLAYING_POPUP_H_
#define _NOWPLAYING_POPUP_H_

#include "core/mediaitem/mediaitem.h"
#include "infosystem/info_system.h"

//Qt
#include <QWidget>
#include <QResizeEvent>
#include <QToolBar>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QHash>
#include <QString>

/*
********************************************************************************
*                                                                              *
*    Class NowPlayingPopup                                                     *
*                                                                              *
********************************************************************************
*/
class NowPlayingPopup : public QWidget
{
Q_OBJECT
  public:
    NowPlayingPopup(QWidget *parent);

    static NowPlayingPopup         *INSTANCE;
    static NowPlayingPopup* instance() { return INSTANCE; }

    void updateWidget();
  protected:
    void resizeEvent(QResizeEvent* event);

  private:
    void setAudioProperties();

  private slots:
    void slot_rating_changed(float);
    void slot_on_lastfm_love();
    void slot_on_track_edit();
    void slot_on_track_add_to_queue();
    void slot_on_add_to_favorite();

    void slot_get_artist_image();
    void slot_download_image();
    void slot_download_artists_tags();
    void slot_on_infosystem_received(INFO::InfoRequestData request, QVariant output);
    void slot_on_tag_clicked();
    void slot_on_websearch();    

  private:
    QHash< QString, QString >  m_metadata;
    MEDIA::TrackPtr            m_track;
      
    QWidget               *m_mainwindow;
    QWidget               *m_playertoolbar;

    QWidget               *ui_tags_widget;
    
    QLabel                *ui_artist_image;
    QLabel                *ui_image;
    
    QPushButton           *ui_btn_station;
    QPushButton           *ui_btn_title;
    QPushButton           *ui_btn_artist;
    QPushButton           *ui_btn_audio;

    QToolBar              *ui_toolbar;
    class RatingWidget    *ui_rating;
    
    QList<quint64>        m_requests_ids;
};


#endif // _NOWPLAYING_POPUP_H_
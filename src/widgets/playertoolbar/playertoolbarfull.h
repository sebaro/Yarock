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

#ifndef _PLAYER_TOOLBAR_FULL_H_
#define _PLAYER_TOOLBAR_FULL_H_

#include "views.h"
#include "playertoolbarbase.h"
#include "core/mediaitem/mediaitem.h"
#include "infosystem/info_system.h"
#include "toolbuttonbase.h"

// Qt
#include <QWidget>
#include <QToolBar>
#include <QToolButton>
#include <QPushButton>
#include <QLabel>
#include <QResizeEvent>

/*
********************************************************************************
*                                                                              *
*    Class PlayerToolBarFull                                                   *
*                                                                              *
********************************************************************************
*/
class EngineBase;
class PlayerToolBarFull : public PlayerToolBarBase
{
Q_OBJECT

public:
    PlayerToolBarFull(QWidget *parent);

    void setCollectionInfo(QString info,VIEW::Id mode);
        
    void fullUpdate();
    
private:
    void setAudioProperties();
    
private:
    QHash< QString, QString >  m_metadata;
    MEDIA::TrackPtr            m_track;
    QList<quint64>             m_requests_ids;
    
    VIEW::Id                   m_mode;
   
    EngineBase                *m_player;

    QWidget                   *m_now_playing_widget;

    QLabel                    *m_currentTime;
    QLabel                    *m_totalTime;
    QLabel                    *m_separator;
    QLabel                    *m_pauseState;
    
    QWidget               *ui_tags_widget;
    
    QLabel                *ui_artist_image;
    QLabel                *ui_album_image;
    QLabel                *ui_collection_image;
    QLabel                *ui_collection_info;    
    
    ToolButtonBase        *ui_btn_station;
    ToolButtonBase        *ui_btn_title;
    ToolButtonBase        *ui_btn_artist;
    ToolButtonBase        *ui_btn_audio;

    class RatingWidget    *ui_rating;
    
    QAction* PLAYING_TRACK_ADD_QUEUE;
    QAction* PLAYING_TRACK_FAVORITE;
    
private :
    void clear();


private slots:
    void slot_update_track_playing_info();
    void slot_update_time_position(qint64);
    void slot_update_total_time(qint64);
    
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
    
signals:
    void switchToolBarType();    
};

#endif // _PLAYER_TOOLBAR_FULL_H_

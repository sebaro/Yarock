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

#ifndef _PLAYER_TOOLBAR_COMPACT_H_
#define _PLAYER_TOOLBAR_COMPACT_H_

#include "playertoolbarbase.h"
#include "views.h"

// Qt
#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QResizeEvent>

/*
********************************************************************************
*                                                                              *
*    Class PlayerToolBarCompact                                                *
*                                                                              *
********************************************************************************
*/
class EngineBase;
class PlayerToolBarCompact : public PlayerToolBarBase
{
Q_OBJECT

public:
    PlayerToolBarCompact(QWidget *parent);

    void setCollectionInfo(QString info,VIEW::Id mode);
    
    void fullUpdate();
    
private:
    VIEW::Id         m_mode;
   
    EngineBase       *m_player;

    QWidget          *m_now_playing_widget;
    
    QLabel           *ui_image;
    QLabel           *ui_label_title;    
    QLabel           *ui_label_album;    
    QLabel           *ui_collection_info;    
    
    QLabel           *m_currentTime;
    QLabel           *m_totalTime;
    QLabel           *m_separator;
    QLabel           *m_pauseState;

private :
    void clear();

private slots:
    void slot_update_track_playing_info();
    void slot_update_time_position(qint64);
    void slot_update_total_time(qint64);
    
signals:
    void switchToolBarType();
};

#endif // _PLAYER_TOOLBAR_COMPACT_H_

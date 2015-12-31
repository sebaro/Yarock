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
#ifndef _GLOBAL_ACTIONS_H_
#define _GLOBAL_ACTIONS_H_

#include "views.h"

//! Qt
#include <QString>
#include <QAction>
#include <QMap>

/*
********************************************************************************
*                                                                              *
*    ACTION enumerate                                                          *
*                                                                              *
********************************************************************************
*/
    enum ENUM_ACTION {
         APP_QUIT,
         APP_SHOW_YAROCK_ABOUT,
         APP_SHOW_PLAYQUEUE,
         APP_SHOW_SETTINGS,

         APP_MODE_COMPACT,
         APP_MODE_NORMAL,

         APP_ENABLE_SEARCH_POPUP,
         APP_PLAY_ON_SEARCH,

         NEW_PLAYLIST,
         NEW_SMART_PLAYLIST,
         
         BROWSER_PREV,
         BROWSER_NEXT,
         BROWSER_UP, 
         BROWSER_ITEM_RATING_CLICK,
         BROWSER_LOCAL_ITEM_MOUSE_MOVE,
         BROWSER_STREAM_ITEM_MOUSE_MOVE,
         BROWSER_DIR_ITEM_MOUSE_MOVE,
         BROWSER_JUMP_TO_ARTIST,
         BROWSER_JUMP_TO_ALBUM,
         BROWSER_JUMP_TO_TRACK,
         BROWSER_JUMP_TO_MEDIA,

         ENGINE_PLAY,
         ENGINE_STOP,
         ENGINE_PLAY_PREV,
         ENGINE_PLAY_NEXT,
         ENGINE_VOL_MUTE,
         ENGINE_VOL_INC,
         ENGINE_VOL_DEC,
         ENGINE_AUDIO_EQ,

         PLAYQUEUE_ADD_FILE,
         PLAYQUEUE_ADD_DIR,
         PLAYQUEUE_ADD_URL,
         PLAYQUEUE_CLEAR,
         PLAYQUEUE_EXPORT,
         PLAYQUEUE_AUTOSAVE,
         PLAYQUEUE_REMOVE_ITEM,
         PLAYQUEUE_JUMP_TO_TRACK,
         PLAYQUEUE_REMOVE_DUPLICATE,
         PLAYQUEUE_OPTION_SHOW_COVER,
         PLAYQUEUE_OPTION_SHOW_RATING,
         PLAYQUEUE_OPTION_SHOW_NUMBER,
         PLAYQUEUE_TRACK_LOVE,
         
         PLAYING_TRACK_EDIT,
         PLAYING_TRACK_LOVE,

         DATABASE_OPERATION,
         DATABASE_ADD,
         
         TAG_CLICKED
    };


/*
********************************************************************************
*                                                                              *
*    Class GlobalActions                                                       *
*                                                                              *
********************************************************************************
*/
#define ACTIONS() (GlobalActions::instance()->actions())

class GlobalActions
{
    static GlobalActions* INSTANCE;

public :
    GlobalActions();

    static GlobalActions* instance() { return INSTANCE; }

    QMap<ENUM_ACTION, QAction*>* actions() { return m_actions; }

private:
    QMap<ENUM_ACTION, QAction*>* m_actions;
};


#endif // _GLOBAL_ACTIONS_H_

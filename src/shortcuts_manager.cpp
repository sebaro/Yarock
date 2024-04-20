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

#include "shortcuts_manager.h"
#include "global_actions.h"
#include "settings.h"

#ifdef ENABLE_SHORTCUT
#include "shortcuts/globalshortcut.h"
#endif

#include "core/player/engine.h"

#include "debug.h"

ShortcutsManager* ShortcutsManager::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class ShortcutsManager                                                    *
*                                                                              *
********************************************************************************
*/
ShortcutsManager::ShortcutsManager(QObject* parent) : QObject(parent)
{
    INSTANCE = this;

    //! load settings
    reloadSettings();
}

void ShortcutsManager::addShortcut(QString id, QAction* action)
{
    Shortcut shortcut;
    shortcut.action   = action;
    shortcut.id       = id;
    shortcut.status   = false;

    m_shortcuts[id] = shortcut;
}

void ShortcutsManager::reloadSettings()
{
    Debug::debug() << "ShortcutsManager::reloadSettings";
  
    addShortcut("play",            ACTIONS()->value(ENGINE_PLAY) );
    addShortcut("stop",            ACTIONS()->value(ENGINE_STOP) );
    addShortcut("next_track",      ACTIONS()->value(ENGINE_PLAY_NEXT) );
    addShortcut("prev_track",      ACTIONS()->value(ENGINE_PLAY_PREV) );
    addShortcut("inc_volume",      ACTIONS()->value(ENGINE_VOL_INC) );
    addShortcut("dec_volume",      ACTIONS()->value(ENGINE_VOL_DEC) );
    addShortcut("mute_volume",     ACTIONS()->value(ENGINE_VOL_MUTE) );
    addShortcut("jump_to_track",   ACTIONS()->value(BROWSER_JUMP_TO_TRACK) );
    addShortcut("clear_playqueue", ACTIONS()->value(PLAYQUEUE_CLEAR) );
    
    // read settings
    m_shortcuts["play"].key            = QKeySequence::fromString(SETTINGS()->_shortcutsKey["play"]);
    m_shortcuts["stop"].key            = QKeySequence::fromString(SETTINGS()->_shortcutsKey["stop"]);
    m_shortcuts["prev_track"].key      = QKeySequence::fromString(SETTINGS()->_shortcutsKey["prev_track"]);
    m_shortcuts["next_track"].key      = QKeySequence::fromString(SETTINGS()->_shortcutsKey["next_track"]);
    m_shortcuts["inc_volume"].key      = QKeySequence::fromString(SETTINGS()->_shortcutsKey["inc_volume"]);
    m_shortcuts["dec_volume"].key      = QKeySequence::fromString(SETTINGS()->_shortcutsKey["dec_volume"]);
    m_shortcuts["mute_volume"].key     = QKeySequence::fromString(SETTINGS()->_shortcutsKey["mute_volume"]);
    m_shortcuts["jump_to_track"].key   = QKeySequence::fromString(SETTINGS()->_shortcutsKey["jump_to_track"]);
    m_shortcuts["clear_playqueue"].key = QKeySequence::fromString(SETTINGS()->_shortcutsKey["clear_playqueue"]);


    QxtUnregister();

    if(SETTINGS()->_useShortcut)
    { 
      // update Qxt shorcut
      QxtRegister();
    }
    
    emit setting_changed();  
}

void ShortcutsManager::QxtUnregister()
{
    Debug::debug() << "  [ShortcutsManager] Unregister";
    qDeleteAll(m_qxt_shortcuts);
    m_qxt_shortcuts.clear();
}

void ShortcutsManager::QxtRegister()
{
    //Debug::debug() << "  [ShortcutsManager] Register";

    foreach (QString id, m_shortcuts.keys())
    {
#ifdef ENABLE_SHORTCUT
      GlobalShortcut* qxt_shortcut = new GlobalShortcut(this);
      bool is_OK = qxt_shortcut->setShortcut(m_shortcuts[id].key);
      m_shortcuts[id].status = is_OK;
      Debug::debug() << "  [ShortcutsManager] Register shortcut" << m_shortcuts[id].key.toString() << " shortcut.status:" << m_shortcuts[id].status;

      if(m_shortcuts[id].status) {
        connect(qxt_shortcut, SIGNAL(activated()), m_shortcuts[id].action, SLOT(trigger()));
        m_qxt_shortcuts << qxt_shortcut;
      }
#endif        
    }
}


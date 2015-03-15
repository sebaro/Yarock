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

#include "global_shortcuts.h"
#include "global_actions.h"
#include "settings.h"

#include "core/player/engine.h"

#include "debug.h"

GlobalShortcuts* GlobalShortcuts::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class GlobalShortcuts                                                     *
*                                                                              *
********************************************************************************
*/
GlobalShortcuts::GlobalShortcuts(QObject* parent) : QObject(parent)
{
    INSTANCE = this;

    //! internal action
    QAction* action_inc_volume  = new QAction(this);
    QAction* action_dec_volume  = new QAction(this);
    QAction* action_mute_volume = new QAction(this);

    connect(action_inc_volume, SIGNAL(triggered()),this, SLOT(incVolume()));
    connect(action_dec_volume, SIGNAL(triggered()),this, SLOT(decVolume()));
    connect(action_mute_volume, SIGNAL(triggered()),this, SLOT(muteVolume()));

    //! init
    addShortcut("play",          ACTIONS()->value(ENGINE_PLAY) );
    addShortcut("stop",          ACTIONS()->value(ENGINE_STOP) );
    addShortcut("next_track",    ACTIONS()->value(ENGINE_PLAY_NEXT) );
    addShortcut("prev_track",    ACTIONS()->value(ENGINE_PLAY_PREV) );
    addShortcut("inc_volume",    action_inc_volume );
    addShortcut("dec_volume",    action_dec_volume );
    addShortcut("mute_volume",   action_mute_volume );
    addShortcut("jump_to_track", ACTIONS()->value(BROWSER_JUMP_TO_TRACK) );
    addShortcut("clear_playqueue", ACTIONS()->value(PLAYQUEUE_CLEAR) );


    //! load settings
    reloadSettings();
}

void GlobalShortcuts::addShortcut(QString id, QAction* action)
{
    Shortcut shortcut;
    shortcut.action   = action;
    shortcut.id       = id;
    shortcut.status   = false;

    m_shortcuts[id] = shortcut;
}

void GlobalShortcuts::reloadSettings()
{
    // read settings
    m_shortcuts["play"].key          = QKeySequence::fromString(SETTINGS()->_shortcutsKey["play"]);
    m_shortcuts["stop"].key          = QKeySequence::fromString(SETTINGS()->_shortcutsKey["stop"]);
    m_shortcuts["prev_track"].key    = QKeySequence::fromString(SETTINGS()->_shortcutsKey["prev_track"]);
    m_shortcuts["next_track"].key    = QKeySequence::fromString(SETTINGS()->_shortcutsKey["next_track"]);
    m_shortcuts["inc_volume"].key    = QKeySequence::fromString(SETTINGS()->_shortcutsKey["inc_volume"]);
    m_shortcuts["dec_volume"].key    = QKeySequence::fromString(SETTINGS()->_shortcutsKey["dec_volume"]);
    m_shortcuts["mute_volume"].key   = QKeySequence::fromString(SETTINGS()->_shortcutsKey["mute_volume"]);
    m_shortcuts["jump_to_track"].key = QKeySequence::fromString(SETTINGS()->_shortcutsKey["jump_to_track"]);
    m_shortcuts["clear_playqueue"].key = QKeySequence::fromString(SETTINGS()->_shortcutsKey["clear_playqueue"]);


    // update Qxt shorcut
    QxtUnregister();
    QxtRegister();
}

void GlobalShortcuts::QxtUnregister()
{
    Debug::debug() << "## GlobalShortcuts::QxtUnregister";
    qDeleteAll(m_qxt_shortcuts);
    m_qxt_shortcuts.clear();
}

void GlobalShortcuts::QxtRegister()
{
    Debug::debug() << "## GlobalShortcuts::QxtRegister";

    foreach (QString id, m_shortcuts.keys())
    {
      QxtGlobalShortcut* qxt_shortcut = new QxtGlobalShortcut(this);
      bool is_OK = qxt_shortcut->setShortcut(m_shortcuts[id].key);
      m_shortcuts[id].status = is_OK;
      //Debug::debug() << "## GlobalShortcuts::QxtRegister shortcut.status:" << m_shortcuts[id].status;

      connect(qxt_shortcut, SIGNAL(activated()), m_shortcuts[id].action, SLOT(trigger()));
      m_qxt_shortcuts << qxt_shortcut;
    }
}


void GlobalShortcuts::incVolume()
{
    int percent = Engine::instance()->volume() < 100 ? Engine::instance()->volume() + 1 : 100;
    Engine::instance()->setVolume(percent);
}

void GlobalShortcuts::decVolume()
{
    int percent = Engine::instance()->volume() > 0 ? Engine::instance()->volume() -1 : 0;
    Engine::instance()->setVolume(percent);
}

void GlobalShortcuts::muteVolume()
{
    Engine::instance()->setMuted( !Engine::instance()->isMuted() );
}

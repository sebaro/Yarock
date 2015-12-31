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

/* NOTE : 
 This class is base on depreciated qxt library
   - rewrite to remove qxt dependancy
   - using media key patch (from clementine) for Media Key
   - using "Global shortcuts for X11 with Qt 5" from Lukas Holecek 
        -> for Qt5 replace QX11Info by qplatformnativeinterface to get display
*/

#ifndef _GLOBAL_SHORTCUT_P_H_
#define _GLOBAL_SHORTCUT_P_H_

#include "globalshortcut.h"


#include <QAbstractEventDispatcher>
#include <QKeySequence>
#include <QHash>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QAbstractNativeEventFilter>
#endif

/*
********************************************************************************
*                                                                              *
*    Class GlobalShortcutX11Private                                            *
*                                                                              *
********************************************************************************
*/
#if QT_VERSION < 0x050000
class GlobalShortcutX11Private 
#else
class GlobalShortcutX11Private : public QAbstractNativeEventFilter
#endif
{
public:
    GlobalShortcutX11Private(GlobalShortcut* gs);
    ~GlobalShortcutX11Private();

    bool enabled;
    Qt::Key key;
    Qt::KeyboardModifiers mods;

    bool setShortcut(const QKeySequence& shortcut);
    bool unsetShortcut();

    static bool error;
    static int ref;
#if QT_VERSION < 0x050000
    static QAbstractEventDispatcher::EventFilter prevEventFilter;
    static bool eventFilter(void* message);
#else
    bool nativeEventFilter(const QByteArray &, void *message, long *result);
#endif

private:
    static quint32 nativeKeycode(Qt::Key keycode);
    static quint32 nativeModifiers(Qt::KeyboardModifiers modifiers);

    static bool registerShortcut(quint32 nativeKey, quint32 nativeMods);
    static bool unregisterShortcut(quint32 nativeKey, quint32 nativeMods);
    static void activateShortcut(quint32 nativeKey, quint32 nativeMods);

    static QHash<QPair<quint32, quint32>, GlobalShortcut*> shortcuts;
    
private :
    GlobalShortcut * const q_ptr;
};


#endif // _GLOBAL_SHORTCUT_P_H_

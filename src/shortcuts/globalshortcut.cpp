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

/* NOTE : 
 This class is base on depreciated qxt library
   - rewrite to remove qxt dependancy
   - using media key patch (from clementine) for Media Key
   - using "Global shortcuts for X11 with Qt 5" from Lukas Holecek
        -> for Qt5 replace QX11Info by qplatformnativeinterface to get display   
*/

#include "globalshortcut.h"
#include "globalshortcut_p.h"


GlobalShortcut::GlobalShortcut(QObject* parent) : QObject(parent), d_ptr(new GlobalShortcutX11Private(this))
{
}


GlobalShortcut::GlobalShortcut(const QKeySequence& shortcut, QObject* parent)
        : QObject(parent), d_ptr(new GlobalShortcutX11Private(this))
{
    setShortcut(shortcut);
}


GlobalShortcut::~GlobalShortcut()
{
    if (d_ptr->key != 0)
        d_ptr->unsetShortcut();
}

QKeySequence GlobalShortcut::shortcut() const
{
    return QKeySequence(d_ptr->key | d_ptr->mods);    
}

bool GlobalShortcut::setShortcut(const QKeySequence& shortcut)
{
    if (d_ptr->key != 0)
        d_ptr->unsetShortcut();
    return d_ptr->setShortcut(shortcut);    
}

bool GlobalShortcut::isEnabled() const
{
    return d_ptr->enabled;
}

void GlobalShortcut::setEnabled(bool enabled)
{
    d_ptr->enabled = enabled;
}

void GlobalShortcut::setDisabled(bool disabled)
{
    d_ptr->enabled = !disabled;
}

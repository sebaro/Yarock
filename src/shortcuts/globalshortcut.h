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

#ifndef _GLOBAL_SHORTCUT_H_
#define _GLOBAL_SHORTCUT_H_

#include <QObject>
#include <QKeySequence>


class GlobalShortcutX11Private;

class GlobalShortcut : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
    Q_PROPERTY(QKeySequence shortcut READ shortcut WRITE setShortcut)

public:
    explicit GlobalShortcut(QObject* parent = 0);
    explicit GlobalShortcut(const QKeySequence& shortcut, QObject* parent = 0);
    virtual ~GlobalShortcut();

    QKeySequence shortcut() const;
    bool setShortcut(const QKeySequence& shortcut);

    bool isEnabled() const;

    void sendActivated() {emit activated();}

public Q_SLOTS:
    void setEnabled(bool enabled = true);
    void setDisabled(bool disabled = true);

Q_SIGNALS:
    void activated();

private:
  GlobalShortcutX11Private * const d_ptr;
  
};

#endif // _GLOBAL_SHORTCUT_H_

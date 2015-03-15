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
#ifndef _GLOBALSHORTCUTS_H_
#define _GLOBALSHORTCUTS_H_

#include <QObject>
#include <QAction>
#include <QKeySequence>
#include <QMap>

#include "qxtglobalshortcut.h"

/*
********************************************************************************
*                                                                              *
*    Class GlobalShortcuts                                                     *
*                                                                              *
********************************************************************************
*/
class GlobalShortcuts : public QObject
{
Q_OBJECT

public:
  GlobalShortcuts(QObject* parent = 0);

  static GlobalShortcuts         *INSTANCE;
  static GlobalShortcuts* instance() { return INSTANCE; }


  struct Shortcut {
    QString      id;
    QKeySequence key;
    QAction*     action;
    bool         status;
  };

  QMap<QString /*id*/, Shortcut> shortcuts() {return m_shortcuts;}

private:
  void addShortcut(QString id, QAction* action);

  // update Qxt shorcut
  void QxtUnregister();
  void QxtRegister();

public slots:
  void reloadSettings();

private slots:
  void incVolume();
  void decVolume();
  void muteVolume();

private:
  QMap<QString /*id*/, Shortcut> m_shortcuts;
  QList<QxtGlobalShortcut*> m_qxt_shortcuts;
};


#endif // _GLOBALSHORTCUTS_H_

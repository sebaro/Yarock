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

#ifndef _SHORTCUT_DIALOG_H_
#define _SHORTCUT_DIALOG_H_

#include "dialog_base.h"

#include <QLabel>
#include <QModelIndex>
#include <QKeySequence>
#include <QKeyEvent>

/*
********************************************************************************
*                                                                              *
*    Class ShortcutDialog                                                      *
*                                                                              *
********************************************************************************
*/
class ShortcutDialog : public DialogBase
{
Q_OBJECT

public:
    ShortcutDialog(const QString& key,const QString& shortcut, QWidget *parent = 0);

    QString newShortcut();

  protected:
    bool event(QEvent *);
    void hideEvent(QHideEvent* e);
    void showEvent(QShowEvent* e);
  
  private:
    QLabel         *m_currentShortcut;

    QKeySequence    m_old_shortcut;
    QKeySequence    m_new_shortcut;
    QString         m_key;
    QList<int>      m_modifier_keys;

   private slots:
     void accept();

   signals:
     void shortcutChanged(QString key, QKeySequence newshortcut);
};

#endif // _SHORTCUT_DIALOG_H_

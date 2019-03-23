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
#ifndef _SYSTRAY_H_
#define _SYSTRAY_H_

//! Qt
#include <QSystemTrayIcon>
#include <QObject>

/*
********************************************************************************
*                                                                              *
*    Class SysTray                                                             *
*                                                                              *
********************************************************************************
*/
class SysTray : public QObject 
{
Q_OBJECT

public:
  SysTray(QObject* parent = 0);
  ~SysTray();

  void reloadSettings();

  bool isSysTrayOn() const;
  bool isVisible() const;
  void close();
  
private:
    void ubuntu_unity_hack();
    
private slots:
    void slot_systray_clicked(QSystemTrayIcon::ActivationReason reason);
    
    void slot_on_message_clicked();

    void ubuntu_unity_hack_getError();

    void ubuntu_unity_hack_getFinished(int);

private:
    QSystemTrayIcon        *m_trayIcon;

    QMenu                  *m_menu;
};

#endif // _SYSTRAY_H_

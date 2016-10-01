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
#include "systray.h"
#include "settings.h"
#include "global_actions.h"
#include "debug.h"

#include <QMainWindow>
#include <QMenu>
#include <QProcess>
#include <QFile>

const char* kGSettingsFileName     = "gsettings";
const char* kUnityPanel            = "com.canonical.Unity.Panel";
const char* kUnitySystrayWhitelist = "systray-whitelist";

/*
********************************************************************************
*                                                                              *
*    Class SysTray                                                             *
*                                                                              *
********************************************************************************
*/

SysTray::SysTray(QObject* parent) : QObject(parent)
{
    m_trayIcon = 0;
    
    reloadSettings();
}

SysTray::~SysTray()
{
   
}

bool SysTray::isVisible() const 
{
    if( !m_trayIcon )
        return false;
    
    return m_trayIcon->isVisible();
}

bool SysTray::isSysTrayOn() const
{
    return (m_trayIcon != 0);
}


void SysTray::reloadSettings()
{
    // systray de-activated
    if(!SETTINGS()->_useTrayIcon)
    {
      if(m_trayIcon != 0) {
          disconnect(m_trayIcon, 0,this, 0);
          delete m_trayIcon;
          m_trayIcon = 0;
      }
    }
    // systray activated
    else
    {
      if(!m_trayIcon)
      {
        QMenu *m_menu = new QMenu();
        m_menu->addAction(ACTIONS()->value(ENGINE_PLAY_PREV));
        m_menu->addAction(ACTIONS()->value(ENGINE_PLAY));
        m_menu->addAction(ACTIONS()->value(ENGINE_STOP));
        m_menu->addAction(ACTIONS()->value(ENGINE_PLAY_NEXT));
        m_menu->addSeparator();
        m_menu->addAction(ACTIONS()->value(PLAYING_TRACK_LOVE));
        m_menu->addSeparator();
        m_menu->addAction(ACTIONS()->value(APP_QUIT));

        m_trayIcon = new QSystemTrayIcon(this);

        m_trayIcon->setIcon( QIcon(":/icon/yarock_64x64.png") );
        m_trayIcon->setToolTip("Yarock");
        m_trayIcon->setContextMenu(m_menu);
        m_trayIcon->show();

        connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(slot_systray_clicked(QSystemTrayIcon::ActivationReason)));
        connect(m_trayIcon, SIGNAL(messageClicked()), this, SLOT(slot_on_message_clicked()));
        
        ubuntu_unity_hack();
      }
    }
}


void SysTray::close()
{
     if(m_trayIcon != 0) {
          disconnect(m_trayIcon, 0,this, 0);
          delete m_trayIcon;
          m_trayIcon = 0;
      }
}

void SysTray::slot_systray_clicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        QMainWindow* w = qobject_cast<QMainWindow*>(this->parent());
            
        bool is_visible = !w->isVisible();
        w->setVisible(is_visible);
        
        if (is_visible) 
        {
          w->activateWindow();
          w->showNormal();
        }
    }
}

void SysTray::slot_on_message_clicked()
{
    QMainWindow* w = qobject_cast<QMainWindow*>(this->parent());
    w->setFocus();
}

/* hack for ubuntu unity systray panel whitelist (base on clementine work) */
void SysTray::ubuntu_unity_hack()
{
    // Check if we're on Ubuntu first.
    QFile lsb_release("/etc/lsb-release");
    if (lsb_release.open(QIODevice::ReadOnly)) {
        QByteArray data = lsb_release.readAll();
        if (!data.contains("DISTRIB_ID=Ubuntu")) {
            // It's not Ubuntu - don't do anything.
            return;
        }
    }

    // Get the systray whitelist from gsettings.  If this fails we're probably
    // not running on a system with unity
    QProcess* get = new QProcess(this);
    connect(get, SIGNAL(finished(int)), SLOT(ubuntu_unity_hack_getFinished(int)));
    connect(get, SIGNAL(error(QProcess::ProcessError)), SLOT(ubuntu_unity_hack_getError()));
    
    get->start(kGSettingsFileName, QStringList()
             << "get" << kUnityPanel << kUnitySystrayWhitelist);
}

void SysTray::ubuntu_unity_hack_getError() 
{
    QProcess* get = qobject_cast<QProcess*>(sender());
    if (!get)
      return;

    get->deleteLater();
}

void SysTray::ubuntu_unity_hack_getFinished(int exit_code) 
{
    QProcess* get = qobject_cast<QProcess*>(sender());
    if (!get)
      return;

    get->deleteLater();

    if (exit_code != 0) {
      // Probably not running in Unity.
      return;
    }

    QByteArray whitelist = get->readAllStandardOutput();

    Debug::debug() << "Unity whitelist is" << whitelist;

    int index = whitelist.lastIndexOf(']');
    if (index == -1 || whitelist.contains("'yarock'"))
      return;

    whitelist = whitelist.left(index) + QString(", 'yarock'").toUtf8() +
                whitelist.mid(index);

    Debug::debug() << "Setting unity whitelist to" << whitelist;

    QProcess* set = new QProcess(this);
    
    connect(set, SIGNAL(finished(int)), set, SLOT(deleteLater()));
    set->start(kGSettingsFileName, QStringList()
             << "set" << kUnityPanel << kUnitySystrayWhitelist << whitelist);

    Debug::debug() << "Yarock has added itself to the Unity system tray" <<
                      "whitelist, but this won't take effect until the next time" <<
                      "you log out and log back in.";
}



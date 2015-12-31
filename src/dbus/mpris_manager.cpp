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

#include "mpris_manager.h"
#include "dbus/mpris2.h"

#include "settings.h"
#include "debug.h"

#include <QMainWindow>
/*
********************************************************************************
*                                                                              *
*    Class MprisManager                                                        *
*                                                                              *
********************************************************************************
*/
MprisManager::MprisManager(QObject *parent) : QObject(parent)
{
    m_mpris2 = 0;

    reloadSettings();
}


/*******************************************************************************
    ~MprisManager
*******************************************************************************/
MprisManager::~MprisManager()
{
    Debug::debug() << "  [Mpris] delete";

    if(m_mpris2 != 0) 
    {
        delete m_mpris2;
        m_mpris2 = 0;
    }
}

/*******************************************************************************
    reloadSettings
*******************************************************************************/
void MprisManager::reloadSettings()
{
    Debug::debug() << "  [Mpris] reloadSettings";
    if(SETTINGS()->_useMpris)
    {
      if(!m_mpris2)
      {
          m_mpris2 =  new Mpris2(this);
          connect(m_mpris2, SIGNAL(RaiseMainWindow()), this, SLOT(activateMainWindow()));
      }
    }
    else
    {
      if(m_mpris2 != 0) 
      {
          delete m_mpris2;
          m_mpris2 = 0;
      }
    }
}

/*******************************************************************************
    activateMainWindow
*******************************************************************************/
void MprisManager::activateMainWindow()
{
    QMainWindow* w = qobject_cast<QMainWindow*>(this->parent());
    w->activateWindow();
    w->showNormal();
}
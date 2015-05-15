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

#include "engine.h"
#include "settings.h"
#include "config.h"
#include "debug.h"

#include <QLibrary>
#include <QPluginLoader>
#include <QtCore>


EngineBase* Engine::CORE_INSTANCE = 0;


/*
********************************************************************************
*                                                                              *
*    Class Engine                                                              *
*                                                                              *
********************************************************************************
*/ 
Engine::Engine()
{
    /* retrieve engine name */
    QString engine_name;
  
    switch ( SETTINGS()->_engine )
    {
      case ENGINE::VLC :    engine_name = "enginevlc";     break;
      case ENGINE::MPV :    engine_name = "enginempv";     break;
      case ENGINE::PHONON : engine_name = "enginephonon";  break;    
      default:break;
    };

    /* check libraries directory */
    QString lookup_dir;
    //Debug::debug() << "[Engine] ###  QCoreApplication::applicationDirPath() " << QCoreApplication::applicationDirPath() ;
    //Debug::debug() << "[Engine] ###  QString(CMAKE_INSTALL_BIN) " << QString(CMAKE_INSTALL_BIN);
    
    if ( QCoreApplication::applicationDirPath() !=  QString(CMAKE_INSTALL_BIN) )
    {
      lookup_dir = QCoreApplication::applicationDirPath() + "/lib";
    }
    else
    {
      lookup_dir = QString(CMAKE_INSTALL_LIB);
    }

    Debug::debug() << "[Engine] Look into library dir:" << lookup_dir;

    QDir lib_dir = QDir(lookup_dir);

    Debug::debug() << "[Engine] Audio engine library directory:" << lib_dir.canonicalPath();

    foreach (QString fileName, lib_dir.entryList(QDir::Files)) 
    {
        if( !fileName.contains( engine_name ) )
          continue;
        
        if( !QLibrary::isLibrary(fileName) )
          continue;

        Debug::debug() << "[Engine] Try to load " << fileName;

        QPluginLoader loader( lib_dir.absoluteFilePath(fileName) );
    
        QObject *obj = loader.instance();

        if (obj) 
        {
          CORE_INSTANCE = qobject_cast<EngineBase *>(obj);
        }
        else
        {
          Debug::error() << "[Engine] error loading library " << fileName << " [" << QLibrary(fileName).errorString() << "]";
        }
    }

    if( CORE_INSTANCE == 0 )
    {
      CORE_INSTANCE    = new EngineBase("null");
      Debug::error() << "[Engine] no audio engine library loaded !";
    }
}



 

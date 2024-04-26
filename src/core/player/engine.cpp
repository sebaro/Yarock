
#include "engine.h"
#include "settings.h"
#include "config.h"
#include "debug.h"

#include <QLibrary>
#include <QPluginLoader>
#include <QtCore>


EngineBase* Engine::CORE_INSTANCE = 0;

Engine::Engine() {
    /* retrieve engine name */
    QString engine_name;

    switch ( SETTINGS()->_engine )
    {
      case ENGINE::VLC          :    engine_name = "enginevlc";     break;
      case ENGINE::MPV          :    engine_name = "enginempv";     break;
      case ENGINE::PHONON       :    engine_name = "enginephonon";  break;
      case ENGINE::QTMULTIMEDIA :    engine_name = "engineqtmultimedia";  break;
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

        QObject* obj =0;
        try
        {
          QPluginLoader loader( lib_dir.absoluteFilePath(fileName) );

          obj = loader.instance();

          Debug::debug() << loader.errorString();
        }
        catch(...)
        {
          Debug::debug() << "########### Loading library failed ";
        }

        if (obj)
        {
          CORE_INSTANCE = qobject_cast<EngineBase *>(obj);
        }
        else
        {
          m_error = QString ("[Engine] error loading library " + fileName + " [" + QLibrary(fileName).errorString() + "]" );
          Debug::error() << m_error;
        }
    }


    if( CORE_INSTANCE != 0 )
    {
        Debug::debug() << "[Engine] engine status OK:" << CORE_INSTANCE->isEngineOK();
        if( !CORE_INSTANCE->isEngineOK() )
        {
            m_error = QString ("[Engine] engine " + CORE_INSTANCE->name() + " loading failure" );
            Debug::error() << m_error;

            delete CORE_INSTANCE;
            CORE_INSTANCE    = new EngineBase();
        }
    }
    /* no library found */
    else if( CORE_INSTANCE == 0 )
    {
        CORE_INSTANCE    = new EngineBase();

        m_error = QString("[Engine] no audio engine library loaded !");
        Debug::error() << m_error;
    }
}


ENGINE::E_ENGINE_TYPE Engine::activeEngine() {
    if(CORE_INSTANCE)
    {
        Debug::debug() << "CORE_INSTANCE->type()" << CORE_INSTANCE->type();
        return CORE_INSTANCE->type();
    }

    return ENGINE::NO_ENGINE;
}

/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2014 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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
#include "info_system.h"
#include "info_system_worker.h"
#include "debug.h"

#include <QtCore>
#include <QCryptographicHash>
/*
********************************************************************************
*                                                                              *
*    namespace INFO                                                            *
*                                                                              *
********************************************************************************
*/ 
static quint64 s_infosystemRequestId = 0;
static QMutex s_infosystemRequestIdMutex;

INFO::InfoRequestData::InfoRequestData()
{
}

INFO::InfoRequestData::InfoRequestData( const INFO::InfoType t, INFO::InfoStringHash hash)
{
    requestId = INFO::infosystemRequestId();
    type      = t;
    data      = QVariant::fromValue< INFO::InfoStringHash >( hash );
}


quint64
INFO::infosystemRequestId()
{
    QMutexLocker locker( &s_infosystemRequestIdMutex );
    quint64 result = s_infosystemRequestId;
    s_infosystemRequestId++;
    return result;
}

QString
INFO::requestMd5( const INFO::InfoRequestData &request)
{
    QCryptographicHash md5( QCryptographicHash::Md5 );

    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();

    /* add request type */
    md5.addData( QString::number( (int)request.type ).toUtf8() );
    
    /* add request data key/value */
    QStringList keys = hash.keys();
    keys.sort();
    foreach( QString key, keys )
    {
       /* skip optionnal request parameter */
       if(key.startsWith("#"))
         continue;
    
        md5.addData( key.toUtf8() );
        md5.addData( hash[key].toUtf8() );
    }

    return md5.result().toHex();
}

QString INFO::albumKey(const QString& artist, const QString& album)
{
      QCryptographicHash hash(QCryptographicHash::Sha1);
      hash.addData(artist.toUtf8().constData());
      hash.addData(album.toUtf8().constData());

      return QString(hash.result().toHex());
}

/*
********************************************************************************
*                                                                              *
*    Class InfoSystem                                                          *
*                                                                              *
********************************************************************************
*/ 
InfoSystem* InfoSystem::INSTANCE = 0;

InfoSystem*
InfoSystem::instance()
{
    if ( !INSTANCE )
        INSTANCE = new InfoSystem( 0 );

    return INSTANCE;
}

InfoSystem::InfoSystem( QObject* parent ) : QObject( parent )
{
    INSTANCE  = this;

    m_inited       = false;
    m_active_cache = true;

    m_workerThread = new InfoSystemWorkerThread( this );
    m_workerThread->start();

    QTimer::singleShot( 0, this, SLOT( slot_init() ) );
}


InfoSystem::~InfoSystem()
{
    Debug::debug() << Q_FUNC_INFO;

    if ( m_workerThread )
    {
        m_workerThread->quit();
        m_workerThread->wait( 60000 );

        delete m_workerThread;
        m_workerThread = 0;
    }
    Debug::debug() << Q_FUNC_INFO << "done deleting worker";
}



void
InfoSystem::slot_init()
{
    if ( m_inited ) 
      return;
    
    
    if ( !m_workerThread->worker() )
    {
        QTimer::singleShot( 0, this, SLOT( slot_init() ) );
        return;
    }

    InfoSystemWorker* worker = m_workerThread->worker();

    qRegisterMetaType< INFO::InfoRequestData>();
    
    connect( worker, SIGNAL( info( INFO::InfoRequestData, QVariant ) ),
             this,       SIGNAL( info( INFO::InfoRequestData, QVariant ) ), Qt::UniqueConnection );

    
//     connect( worker, SIGNAL( finished( INFO::InfoRequestData) ),
//              this,       SIGNAL( finished( INFO::InfoRequestData ) ), Qt::UniqueConnection );

    /* call worker init */ 
    QMetaObject::invokeMethod( worker, "init", Qt::QueuedConnection );

    m_inited = true;
}


bool
InfoSystem::getInfo( const INFO::InfoRequestData& requestData )
{
    if ( !m_inited || !m_workerThread->worker() )
    {
        slot_init();
        return false;
    }
    
    /* start worker */
    QMetaObject::invokeMethod( m_workerThread->worker(), "getInfo", Qt::QueuedConnection, Q_ARG( INFO::InfoRequestData, requestData ) );
    return true;
}


/*
********************************************************************************
*                                                                              *
*    Class InfoSystemWorkerThread                                              *
*                                                                              *
********************************************************************************
*/ 
InfoSystemWorkerThread::InfoSystemWorkerThread( QObject* parent ) : QThread( parent )
{
    Debug::debug() << Q_FUNC_INFO;
}


InfoSystemWorkerThread::~InfoSystemWorkerThread()
{
    Debug::debug() << Q_FUNC_INFO;
}


void
InfoSystemWorkerThread::InfoSystemWorkerThread::run()
{
    m_worker = QPointer< InfoSystemWorker >( new InfoSystemWorker() );
    exec();
    if ( !m_worker.isNull() )
        delete m_worker.data();
}


InfoSystemWorker*
InfoSystemWorkerThread::worker() const
{
    if ( m_worker.isNull() )
        return 0;
    return m_worker.data();
} 


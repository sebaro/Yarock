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
#include "info_system_worker.h"
#include "debug.h"

#include "services/service_discogs.h"
#include "services/service_lastfm.h"
#include "services/service_musicbrainz.h"
#include "services/service_lyrics.h"
#include "services/service_spotify.h"

/*
********************************************************************************
*                                                                              *
*    Class InfoSystemWorker                                                    *
*                                                                              *
********************************************************************************
*/ 
InfoSystemWorker::InfoSystemWorker() : QObject()
{
    //Debug::debug() << Q_FUNC_INFO;
}

InfoSystemWorker::~InfoSystemWorker()
{
    //Debug::debug() << Q_FUNC_INFO;
    
    foreach( INFO::InfoService* service, m_services )
    {
        if( service )
          delete service;
    }
}


void
InfoSystemWorker::init(  )
{
    Debug::debug() << "  [InfoSystem] init";

    /* create all InfoServices */
    m_services.append(new ServiceSpotify());
    m_services.append(new ServiceLastFm());
    m_services.append(new ServiceMusicBrainz());
    m_services.append(new ServiceDiscogs());
    m_services.append(new ServiceLyrics());
    
    /* connect all InfoServices */
    foreach (INFO::InfoService* service, m_services) {
    
        service->moveToThread( this->thread() );
    
        connect(
            service,
            SIGNAL( checkCache( INFO::InfoRequestData ) ),
            this,
            SLOT( slot_checkCache( INFO::InfoRequestData ) ),
            Qt::QueuedConnection
        );

        connect(
            service,
            SIGNAL( info( INFO::InfoRequestData, QVariant) ),
            this,
            SLOT( slot_updateCache( INFO::InfoRequestData, QVariant) ),
            Qt::QueuedConnection
        );
                
        
        QMetaObject::invokeMethod( service, "init", Qt::QueuedConnection );
    }
} 


void InfoSystemWorker::getInfo( INFO::InfoRequestData requestData )
{
    bool foundOne = false;
    Debug::debug() << "  [InfoSystem] getInfo";
    
    foreach (INFO::InfoService* service, m_services) 
    {
        if( service->supportedInfoTypes().contains(requestData.type) ) 
        {
          QMetaObject::invokeMethod( service, "getInfo", Qt::QueuedConnection, Q_ARG( INFO::InfoRequestData, requestData ) );
          foundOne = true;
        }
    }

    if ( !foundOne )
    {
        emit info( requestData, QVariant() );
    }
}

void InfoSystemWorker::slot_checkCache(INFO::InfoRequestData requestData)
{
    //Debug::debug() << "  [InfoSystem] slot_checkCache";
  
    QObject* sendingObj = sender();
    
    /* check if info is already in cache */
    const QString md5Hash = INFO::requestMd5(requestData);
    
    if( m_dataCache.contains( md5Hash) ) 
    {
      /* notify info ready */
      emit info( requestData, QVariant( *( m_dataCache[ md5Hash ] ) ) ); 
    }
    else
    {
      /* trigger caller service fetch */
      QMetaObject::invokeMethod( sendingObj, "fetchInfo", Q_ARG( INFO::InfoRequestData, requestData ));
    }
}


void InfoSystemWorker::slot_updateCache(INFO::InfoRequestData requestData, QVariant output)
{
    //Debug::debug() << "  [InfoSystem] slot_updateCache";
    
    if( !output.isNull() ) 
    {
        /* check if info is already in cache */
        const QString md5Hash = INFO::requestMd5(requestData);
        if( !m_dataCache.contains( md5Hash) && InfoSystem::instance()->cacheActivated() )
        {
          m_dataCache.insert(md5Hash, new QVariant(output) );
        }

        /* notify info ready */
        emit info( requestData, output );
    }
}


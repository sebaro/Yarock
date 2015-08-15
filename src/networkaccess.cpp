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


#include "networkaccess.h"
#include "config.h"
#include "debug.h"

const QString USER_AGENT = QString( QString(APP_NAME) + "/" + QString(VERSION) );

static QMap< QThread*, QNetworkAccessManager* > s_threadNamHash;

/*
********************************************************************************
*                                                                              *
*    Class NetworkReply                                                        *
*                                                                              *
********************************************************************************
*/
NetworkReply::NetworkReply(QNetworkReply *networkReply) : QObject(networkReply)
{
    m_reply = networkReply;
    
    redirectCount = 0;

    connectReplySignals();
}

void NetworkReply::connectReplySignals()
{
    connect( m_reply, SIGNAL( finished() ), SLOT( networkLoadFinished() ) );
    connect( m_reply, SIGNAL( error( QNetworkReply::NetworkError ) ), SLOT( networkError( QNetworkReply::NetworkError ) ) );
    connect( m_reply, SIGNAL( destroyed( QObject* ) ), SLOT( deletedByParent() ) );
}



void
NetworkReply::deletedByParent()
{
    //Debug::debug() << Q_FUNC_INFO;

    if ( sender() == m_reply )
    {
        m_reply = 0;
        deleteLater();
    }
}


void NetworkReply::networkError(QNetworkReply::NetworkError code)
{
    Debug::debug() << Q_FUNC_INFO
             << m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
             << m_reply->errorString() << code;
     
    if ( m_reply->error() != QNetworkReply::NoError )
      emit error(m_reply);
}

void NetworkReply::networkLoadFinished() 
{
    //Debug::debug() << Q_FUNC_INFO;

    QUrl redirection = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    
    //Debug::debug() << Q_FUNC_INFO << " REDIRECTION " << redirection ;
    //Debug::debug() << Q_FUNC_INFO << " host() " << redirection.host() ;
    //Debug::debug() << Q_FUNC_INFO << " scheme() " << redirection.scheme() ;
    //Debug::debug() << Q_FUNC_INFO << " request url " << m_reply->request().url() ;
    
    if (redirection.isValid() && redirectCount < MAX_REDIRECTS) 
    {
        if ( m_reply->operation() == QNetworkAccessManager::GetOperation ||
             m_reply->operation() == QNetworkAccessManager::HeadOperation) 
        {            
            Debug::debug() << "NETWORK: Start redirect";
            // Because the redirection url can be relative, 
            // we have to use the previous one to resolve it 
            QUrl newUrl = m_reply->url().resolved(redirection);
    
            
            QNetworkReply *redirectReply = HTTP()->request(newUrl, m_reply->operation());
            setParent(redirectReply);
            m_reply->deleteLater();
            m_reply = redirectReply;
            
            connectReplySignals();
            redirectCount++;
            return;
        }
        else 
        {
          Debug::warning() << "NETWORK: Redirection not supported" << m_reply->url().toEncoded();
        }
    }
    else if (redirectCount >= MAX_REDIRECTS)
    {
      Debug::debug() << "NETWORK: Max redirection reached !";
      emit finished(m_reply);
    }
    else 
    {
      Debug::debug() << "NETWORK: finished";
      emit finished(m_reply);
      emit data(m_reply->readAll());
    }
}


/*
********************************************************************************
*                                                                              *
*    Class NetworkAccess                                                       *
*                                                                              *
********************************************************************************
*/
NetworkAccess* NetworkAccess::INSTANCE = 0;

NetworkAccess::NetworkAccess(QObject* parent) : QObject( parent ) {}


NetworkAccess*
NetworkAccess::http()
{
    if ( !INSTANCE )
        INSTANCE = new NetworkAccess( 0 );

    return INSTANCE;
}



QNetworkRequest NetworkAccess::buildRequest(QUrl url)
{
    //Debug::debug() << Q_FUNC_INFO << url.toString();
    
    QNetworkRequest request(url);
#if QT_VERSION < QT_VERSION_CHECK( 5, 0, 0 )
    // fix issue for radionomy website access
    if(url.toString().contains("radionomy"))
        request.setRawHeader("User-Agent", "Wget/1.16.0");
    else
        request.setRawHeader("User-Agent", USER_AGENT.toUtf8());        
#else
    request.setRawHeader("User-Agent", USER_AGENT.toUtf8());    
#endif    
    request.setRawHeader("Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "en-us,en;q=0.5");
    request.setRawHeader("Connection", "Keep-Alive");

    return request;
}

QNetworkReply* NetworkAccess::request(QUrl url, int operation, QByteArray data) 
{
    QNetworkAccessManager *manager = accessManager();

    QNetworkRequest request = buildRequest(url);

    QNetworkReply *networkReply;
    switch (operation) 
    {
        case QNetworkAccessManager::GetOperation:
            networkReply = manager->get(request);
            Debug::debug() << "NETWORK: get " << url.toString();
            break;

        case QNetworkAccessManager::HeadOperation:
            networkReply = manager->head(request);
            break;

        case QNetworkAccessManager::PostOperation:
            request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
            networkReply = manager->post(request, data);
            break;

        default:
            Debug::warning() << "NETWORK: Unknown operation:" << operation;
            return 0;
    }

    /* QT4 issue with ssl handshake */
    networkReply->ignoreSslErrors();
    
    return networkReply;
}




NetworkReply* NetworkAccess::get(const QUrl url)
{
    QNetworkReply *networkReply = request(url);
    return new NetworkReply(networkReply);
}

/* GET directly with QNetworkRequest in input    */
/*   user shall provide request with filled header */
NetworkReply* NetworkAccess::get(QNetworkRequest request)
{
    QNetworkReply *networkReply = accessManager()->get(request);
    return new NetworkReply(networkReply);
}


NetworkReply* NetworkAccess::head(const QUrl url)
{
    QNetworkReply *networkReply = request(url, QNetworkAccessManager::HeadOperation);
    return new NetworkReply(networkReply);
}


NetworkReply* NetworkAccess::post(const QUrl url, QByteArray data)
{
    QNetworkReply *networkReply = request(url, QNetworkAccessManager::PostOperation,data);
    return new NetworkReply(networkReply);
}

/* POST directly with QNetworkRequest in input    */
/*   user shall provide request with filled header */
NetworkReply* NetworkAccess::post(QNetworkRequest request, QByteArray data)
{
    QNetworkReply *networkReply = accessManager()->post(request, data);
    return new NetworkReply(networkReply);
}

QNetworkAccessManager* NetworkAccess::accessManager() 
{
    if ( s_threadNamHash.contains(  QThread::currentThread() ) )
    {
        //Debug::debug() << Q_FUNC_INFO << "Found current thread in nam hash";
        return s_threadNamHash[ QThread::currentThread() ];
    }
    else 
    {
        //Debug::debug() << Q_FUNC_INFO << "create new access manager";
        s_threadNamHash.insert( QThread::currentThread() ,new QNetworkAccessManager());
        return s_threadNamHash[ QThread::currentThread() ];
    }  
}

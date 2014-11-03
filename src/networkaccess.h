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
#ifndef _NETWORKACCESS_H_
#define _NETWORKACCESS_H_

#include <QtNetwork>
#include <QHash>
#include <QThread>

#include "constants.h"

/*
********************************************************************************
*                                                                              *
*    Class NetworkReply                                                        *
*                                                                              *
********************************************************************************
*/
class NetworkReply : public QObject
{
Q_OBJECT

public:
    NetworkReply(QNetworkReply* networkReply);
    static const int MAX_REDIRECTS = 10;
    
signals:
    void data(QByteArray);
    void error(QNetworkReply*);
    void finished(QNetworkReply*);

private slots:
    void networkError(QNetworkReply::NetworkError);
    void networkLoadFinished();
    void deletedByParent();
  
private:
    void connectReplySignals();

private:
    QNetworkReply    *m_reply;
    int               redirectCount;
};

/*
********************************************************************************
*                                                                              *
*    Class NetworkAccess                                                       *
*                                                                              *
********************************************************************************
*/
#define HTTP() (NetworkAccess::http())


class NetworkAccess : public QObject
{
Q_OBJECT

public:
    NetworkAccess(QObject* parent = 0);
    static NetworkAccess* http();
  
    QNetworkReply* request(QUrl url,int operation = QNetworkAccessManager::GetOperation, QByteArray data = QByteArray());

    NetworkReply* get(QUrl url);
    NetworkReply* get(QNetworkRequest request);
    
    NetworkReply* head(QUrl url);
    NetworkReply* post(QUrl url, QByteArray data = QByteArray());
    NetworkReply* post(QNetworkRequest request, QByteArray data = QByteArray());

private:
    QNetworkRequest buildRequest(QUrl url);
  
    QNetworkAccessManager* accessManager();
    
    static NetworkAccess* INSTANCE;    
};


#endif // _NETWORKACCESS_H_

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
#ifndef _SERVICE_SPOTIFY_H_
#define _SERVICE_SPOTIFY_H_

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QtCore>
#include <QNetworkReply>

#include "info_service.h"


/*
********************************************************************************
*                                                                              *
*    Class ServiceSpotify                                                      *
*                                                                              *
********************************************************************************
*/
class ServiceSpotify : public INFO::InfoService
{
Q_OBJECT
public:
    ServiceSpotify();
    virtual ~ServiceSpotify();

protected slots:
    virtual void init() {}
    void getInfo( INFO::InfoRequestData requestData );
    void fetchInfo( INFO::InfoRequestData requestData );
    
private:
    void fetch_artist_image(INFO::InfoRequestData request);
    void fetch_artist_similar(INFO::InfoRequestData request);  
    void getAuthentification();

private slots:
    void slot_get_artist_images(QByteArray);
    void slot_get_artist_similar(QByteArray);
    void slot_image_received(QByteArray);
    void slot_request_error();
    void slot_getAuthentification(QNetworkReply*);
    //void slot_getAuthentification(QByteArray);

private:
    QMap<QObject*, INFO::InfoRequestData> m_requests;
    QMap<QString, QString> m_auth_info;
};

#endif // _SERVICE_SPOTIFY_H_


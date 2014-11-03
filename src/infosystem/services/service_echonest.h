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
#ifndef _SERVICE_ECHONEST_H_
#define _SERVICE_ECHONEST_H_

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QtCore>

#include "info_service.h"


/*
********************************************************************************
*                                                                              *
*    Class ServiceEchonest                                                     *
*                                                                              *
********************************************************************************
*/
class ServiceEchonest : public INFO::InfoService
{
Q_OBJECT
public:
    ServiceEchonest();
    virtual ~ServiceEchonest();

protected slots:
    virtual void init() {}
    void getInfo( INFO::InfoRequestData requestData );
    void fetchInfo( INFO::InfoRequestData requestData );
    
private:
    void fetch_artist_bio(INFO::InfoRequestData request);    
    void fetch_artist_image(INFO::InfoRequestData request);
    void fetch_artist_similar(INFO::InfoRequestData request);  
    void fetch_image_uri( INFO::InfoRequestData request );

private slots:
    void slot_get_artist_biography(QByteArray);
    void slot_get_artist_images(QByteArray);
    void slot_get_artist_similar(QByteArray);
    void slot_image_received(QByteArray);
    void slot_request_error();

private:
    QMap<QObject*, INFO::InfoRequestData> m_requests;
};

#endif // _SERVICE_ECHONEST_H_


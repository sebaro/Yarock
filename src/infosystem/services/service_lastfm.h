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
#ifndef _SERVICE_LASTFM_H_
#define _SERVICE_LASTFM_H_

#include "info_service.h"
#include "info_system.h"

#include <QObject>
#include <QString>
#include <QMap>
#include <QByteArray>

/*
********************************************************************************
*                                                                              *
*    Class ServiceLastFm                                                       *
*                                                                              *
********************************************************************************
*/
class ServiceLastFm  : public INFO::InfoService
{
Q_OBJECT
public:
    ServiceLastFm();
    virtual ~ServiceLastFm();

protected slots:
    virtual void init() {}
    void getInfo( INFO::InfoRequestData request );
    void fetchInfo( INFO::InfoRequestData request );

private:
    void fetch_artist_info( INFO::InfoRequestData request );
    void fetch_artist_similar( INFO::InfoRequestData request );
    void fetch_album_info( INFO::InfoRequestData request );
    void fetch_image_uri( INFO::InfoRequestData request );

private slots:    
    void slot_parse_artist_info( QByteArray bytes );
    void slot_parse_artist_similar( QByteArray bytes );
    void slot_parse_album_info(QByteArray);
    void slot_image_received(QByteArray);
    void slot_request_error();

private:
    QMap<QObject*, INFO::InfoRequestData> m_requests;
};

#endif // _SERVICE_LASTFM_H_


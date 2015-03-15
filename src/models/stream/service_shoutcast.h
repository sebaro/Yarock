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

#ifndef _SERVICE_SHOUTCAST_H_
#define _SERVICE_SHOUTCAST_H_

#include "service_base.h"
#include "core/mediaitem/mediaitem.h"

#include <QList>
#include <QMap>
#include <QByteArray>
#include <QObject>
/*
********************************************************************************
*                                                                              *
*    Class ShoutCast                                                           *
*                                                                              *
********************************************************************************
*/
class ShoutCast : public Service
{
Q_OBJECT
public:
    ShoutCast();
    virtual void load();
    virtual void reload();
    virtual QList<MEDIA::TrackPtr> streams();
    virtual QList<MEDIA::LinkPtr> links();
    
public slots:
    virtual void slot_activate_link(MEDIA::LinkPtr link=MEDIA::LinkPtr(0));

private:
    void browseLink(MEDIA::LinkPtr link);
    void browseStation(MEDIA::LinkPtr link);
        
private slots:
    void slot_error();
    void slotBrowseLinkDone(QByteArray);
    void slotBrowseStationDone(QByteArray);
    
private:
    QMap<QObject*, MEDIA::LinkPtr>  m_requests;
    MEDIA::LinkPtr        m_active_link;
    MEDIA::LinkPtr        m_root_link;    
};

#endif // _SERVICE_SHOUTCAST_H_

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

#ifndef _SERVICE_TUNEIN_H_
#define _SERVICE_TUNEIN_H_

#include "service_base.h"
#include "core/mediaitem/mediaitem.h"

#include <QMap>
#include <QByteArray>
#include <QObject>

/*
********************************************************************************
*                                                                              *
*    Class TuneIn                                                              *
*                                                                              *
********************************************************************************
*/
class TuneIn : public Service
{
Q_OBJECT
public:
    TuneIn();
    virtual void load();
    virtual void reload();
    virtual QList<MEDIA::TrackPtr> streams();
    virtual QList<MEDIA::LinkPtr> links();

public slots:
    virtual void slot_activate_link(MEDIA::LinkPtr link=MEDIA::LinkPtr(0));
   
private:
    void browseLink(MEDIA::LinkPtr link);
    void parseTuneInJsonElement(QVariantMap map, MEDIA::LinkPtr link);

    void loadGenres();
    
private slots:
    void slotBrowseLinkDone(QByteArray bytes);
    void slotBrowseLinkError();
    void slot_stream_image_received(QByteArray);

private:
    QMap<QObject*, MEDIA::LinkPtr>   m_requests;
    QMap<QObject*, MEDIA::TrackPtr>  m_image_requests;
    QMap<QString, QString>  m_genres;
};

#endif // _SERVICE_TUNEIN_H_

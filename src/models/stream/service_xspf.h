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

#ifndef _XSPF_STREAMS_H_
#define _XSPF_STREAMS_H_

#include "service_base.h"
#include "core/mediaitem/mediaitem.h"

#include <QList>
#include <QString>

/*
********************************************************************************
*                                                                              *
*    Class XspfStreams                                                         *
*                                                                              *
********************************************************************************
*/
class XspfStreams : public Service
{
Q_OBJECT  
public:
    XspfStreams();
    ~XspfStreams();

    virtual void load();
    virtual void reload();
    virtual QList<MEDIA::TrackPtr> streams();
    virtual QList<MEDIA::LinkPtr> links();
    
    void saveToFile();
    void updateItem(MEDIA::TrackPtr stream);
    
public slots:
    virtual void slot_activate_link(MEDIA::LinkPtr link=MEDIA::LinkPtr(0));
   
private:
    QString                 m_filename;
    QList<MEDIA::TrackPtr>  m_streams;
    
    MEDIA::LinkPtr        m_active_link;
    MEDIA::LinkPtr        m_root_link;
};


#endif // _XSPF_STREAMS_H_

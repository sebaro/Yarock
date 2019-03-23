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

#ifndef _FAVORITE_STREAMS_H_
#define _FAVORITE_STREAMS_H_

#include "service_base.h"
#include "core/mediaitem/mediaitem.h"

#include <QList>
#include <QString>

/*
********************************************************************************
*                                                                              *
*    Class FavoriteStreams                                                     *
*                                                                              *
********************************************************************************
*/
class FavoriteStreams : public Service
{
Q_OBJECT  
public:
    FavoriteStreams();
    ~FavoriteStreams();

    virtual void load();
    virtual void reload();
    virtual QList<MEDIA::TrackPtr> streams();
    virtual QList<MEDIA::LinkPtr> links();
    
    void addOrRemovetoFavorite(MEDIA::TrackPtr stream);
    void updateStreamFavorite(MEDIA::TrackPtr stream);
    
private:
    bool findStream(MEDIA::TrackPtr stream);

public slots:
    virtual void slot_activate_link(MEDIA::LinkPtr link=MEDIA::LinkPtr(0));

private slots:
    void slot_dbBuilder_stateChange();
    
private:
    QString                 m_filename;
    QList<MEDIA::TrackPtr>  m_streams;
};


#endif // _FAVORITE_STREAMS_H_

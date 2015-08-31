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

#ifndef _MEDIAMIMEDATA_H
#define _MEDIAMIMEDATA_H


#include <QMimeData>
#include <QStringList>
#include <QString>


#include "core/mediaitem/mediaitem.h"

static const QString MEDIA_MIME = "application/x-yarock-media";

enum MEDIA_SOURCE   {SOURCE_COLLECTION, SOURCE_PLAYQUEUE};

/*
********************************************************************************
*                                                                              *
*    Class MediaMimeData                                                       *
*                                                                              *
********************************************************************************
*/
class MediaMimeData : public QMimeData
{
  public:
    MediaMimeData(MEDIA_SOURCE s);

    QStringList formats() const;

    bool hasFormat( const QString &mimeType ) const;

    QList<MEDIA::TrackPtr> getTracks() const { return m_tracks; }

    void addTrack(const MEDIA::TrackPtr track);
    void addTracks(QList<MEDIA::TrackPtr> tracks);

    MEDIA_SOURCE source() const {return m_source;} 
    void setSource(MEDIA_SOURCE  so) {m_source = so;}

    MEDIA::MediaPtr parent() const {return m_parent;}
    void setParent(MEDIA::MediaPtr parent) {m_parent = parent;}
    
    MediaMimeData* copy() const;
    
  private:
    MEDIA_SOURCE           m_source;
    QList<MEDIA::TrackPtr> m_tracks;
    MEDIA::MediaPtr        m_parent;
};

#endif // _MEDIAMIMEDATA_H

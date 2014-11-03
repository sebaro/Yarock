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

#include "mediamimedata.h"

/*
********************************************************************************
*                                                                              *
*    Class MediaMimeData                                                       *
*                                                                              *
********************************************************************************
*/
MediaMimeData::MediaMimeData(MEDIA_SOURCE s) : QMimeData()
{
    m_source = s;
}

QStringList MediaMimeData::formats() const
{
    QStringList formats( QMimeData::formats() );

    formats.append(MEDIA_MIME);

    if( !formats.contains( "text/uri-list" ) )
      formats.append( "text/uri-list" );

    return formats;
}

bool MediaMimeData::hasFormat( const QString &mimeType ) const
{
    if(m_source == SOURCE_COLLECTION) 
    {
      if( mimeType == MEDIA_MIME )
        return true;
    }
    else if(m_source == SOURCE_PLAYQUEUE)
    {
      if( mimeType == MEDIA_MIME || mimeType == "text/uri-list")
        return true;
    }
    return false;    
}

void MediaMimeData::addTrack(const MEDIA::TrackPtr track)
{
    m_tracks.append(track);
}

void MediaMimeData::addTracks(QList<MEDIA::TrackPtr> tracks)
{
    m_tracks.append(tracks);
}

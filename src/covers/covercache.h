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

#ifndef _COVER_CACHE_H_
#define _COVER_CACHE_H_


#include "core/mediaitem/mediaitem.h"

#include <QImage>
#include <QPixmap>
#include <QPixmapCache>
#include <QHash>
/*
********************************************************************************
*                                                                              *
*    Class CoverCache                                                          *
*                                                                              *
********************************************************************************
*/
class CoverCache
{
public:
    CoverCache();
    static CoverCache* instance();
    void invalidate( const MEDIA::MediaPtr album );

    QPixmap image( MEDIA::ArtistPtr artist, QList<MEDIA::AlbumPtr> albums=QList<MEDIA::AlbumPtr>());
    QPixmap cover( const MEDIA::AlbumPtr album);
    QPixmap cover( const MEDIA::TrackPtr track);

    void addStreamCover( const QString& url, QImage image);
    bool hasCover(MEDIA::TrackPtr );
    
private:
    static CoverCache* INSTANCE;
    ~CoverCache();
    
    /* hash from media Object pointer to QPixmapCache:key internal key */
    QHash< const MEDIA::MediaPtr, QPixmapCache::Key > m_keys;
    QHash< const QString, QPixmapCache::Key > m_keys_urls;

    QPixmap get_default_pixmap(bool isStream=false);
    
    Q_DISABLE_COPY( CoverCache )
};

#endif // _COVER_CACHE_H_

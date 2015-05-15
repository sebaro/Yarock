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

#include "covercache.h"
#include "database.h"
#include "utilities.h"
#include "debug.h"

#include <QFile>
#include <QPixmap>
#include <QPainter>

static const QString noCoverKey  = QString(":/images/default-cover-120x120.png");
static const QString urlCoverKey = QString(":/images/media-url-110x110.png");
/*
********************************************************************************
*                                                                              *
*    Class CoverCache                                                          *
*                                                                              *
********************************************************************************
*/
CoverCache* CoverCache::INSTANCE = 0;

CoverCache* CoverCache::instance()
{
    return INSTANCE;
}

CoverCache::CoverCache()
{
    QPixmapCache::insert( noCoverKey, get_default_pixmap(false) );
    QPixmapCache::insert( urlCoverKey, get_default_pixmap(true) );
      
    INSTANCE = this;
}

CoverCache::~CoverCache()
{
}


/* ---------------------------------------------------------------------------*/
/* CoverCache::image for artist                                               */
/* ---------------------------------------------------------------------------*/
QPixmap CoverCache::image( MEDIA::ArtistPtr artist, QList<MEDIA::AlbumPtr> albums)
{
    QPixmap pixmap;

    if(!Database::instance()->param()._option_artist_image) 
    {
      pixmap = QPixmap( 120, 120 );
      {
        pixmap.fill( Qt::transparent );
        QPainter pt( &pixmap );

        //! affichage des covers
        int i=0;      
        foreach(MEDIA::AlbumPtr album, albums)
        {
          QPixmap pix = CoverCache::instance()->cover(album);
          pix = pix.scaled(QSize(85,85), Qt::KeepAspectRatio, Qt::SmoothTransformation);
  
          QTransform transform = QTransform().rotate(15*i++).translate(-pix.width()/2,-pix.height()/2);
          pix = pix.transformed(transform, Qt::SmoothTransformation);

          pt.drawPixmap(pixmap.rect().center() - QPoint(pix.width() / 2, pix.height()/2), pix);
        } 
        pt.end();
      }
    }
    else
    {
      QPixmapCache::Key key = m_keys.value( artist );

      if( key != QPixmapCache::Key() && QPixmapCache::find( key, &pixmap ) )
        return pixmap;

      /* no pixmap in cache */
      pixmap = artist->pixmap();

      if(!pixmap.isNull())
      {
        m_keys[artist] = QPixmapCache::insert( pixmap );
      }
      else
      {
        if (QPixmapCache::find( noCoverKey, &pixmap ) )
          return pixmap;
        else
          return get_default_pixmap(false);
      }
    }

    return pixmap;  
}
    
/* ---------------------------------------------------------------------------*/
/* CoverCache::cover                                                          */
/* ---------------------------------------------------------------------------*/
QPixmap CoverCache::cover( const MEDIA::AlbumPtr album )
{
    QPixmap pixmap;

    QPixmapCache::Key key = m_keys.value( album );

    if( key != QPixmapCache::Key() && QPixmapCache::find( key, &pixmap ) )
      return pixmap;

    /* no pixmap in cache */
    pixmap = album->pixmap();

    if(!pixmap.isNull())
    {
        m_keys[album] = QPixmapCache::insert( pixmap );
    }
    else
    {
        if (QPixmapCache::find( noCoverKey, &pixmap ) )
          return pixmap;
        else
          return get_default_pixmap(false);
    }

    return pixmap;
}

/* ---------------------------------------------------------------------------*/
/* CoverCache::cover                                                          */
/* ---------------------------------------------------------------------------*/
QPixmap CoverCache::cover(const MEDIA::TrackPtr track )
{
    QPixmap pixmap = QPixmap();

    if(!track)
      return pixmap;

    if(track->type() != TYPE_TRACK)
    {
        QString s_url = track->url;

        /* check if cover exist for stream */
        QPixmapCache::Key key = m_keys_urls.value( s_url );

        if( key != QPixmapCache::Key() && QPixmapCache::find( key, &pixmap ) )
          return pixmap;

        /* check if parent have cover, need to BE a stream !!! not a playlist ITEM*/
        if(track->parent() && track->parent()->type() == TYPE_STREAM)
        {
           s_url = MEDIA::TrackPtr::staticCast(track->parent())->url;
      
           QPixmapCache::Key key = m_keys_urls.value( s_url );

           if( key != QPixmapCache::Key() && QPixmapCache::find( key, &pixmap ) )
             return pixmap;          
        }
      
        if (QPixmapCache::find( urlCoverKey, &pixmap ) )
          return pixmap;
        else
          return get_default_pixmap(true);
    }
    else if (track->id == -1)
    {
        QPixmap pixmap = MEDIA::LoadCoverFromFile( track->url );
        if(!pixmap.isNull())
          return pixmap;

        if (QPixmapCache::find( noCoverKey, &pixmap ) )
          return pixmap;
        else
          return get_default_pixmap(false);
    }
    else // track exist in collection
    {
        //! 1: first check cover for parent album
        if(track->parent() &&  track->parent()->type() == TYPE_ALBUM) {
          MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(track->parent());
          if(album)
            return cover(album);
        }

        //! 2: check cover path in /<config fir>/albums/
        QString path = UTIL::CONFIGDIR + "/albums/" + track->coverName();
        if( QFile(path).exists() )
          return QPixmap(path);

        if (QPixmapCache::find( noCoverKey, &pixmap ) )
          return pixmap;
        else
          return get_default_pixmap(false);
    }

    return pixmap;
}

/* ---------------------------------------------------------------------------*/
/* CoverCache::invalidate                                                     */
/* ---------------------------------------------------------------------------*/
void CoverCache::invalidate( const MEDIA::MediaPtr media )
{
    if( !m_keys.contains( media ) )
        return;

    QPixmapCache::Key key = m_keys.value( media );
    QPixmapCache::remove( key );
}


/* ---------------------------------------------------------------------------*/
/* CoverCache::hasCover                                                       */
/* ---------------------------------------------------------------------------*/
bool CoverCache::hasCover(MEDIA::TrackPtr track)
{
    return m_keys.contains( track );
}


void CoverCache::addStreamCover( const QString& url, QImage image)
{
    Debug::debug() << "    [CoverCache] addStreamCover:" << url;
    QImage i = image.scaledToHeight(120, Qt::SmoothTransformation);
  
    QPixmap pixTemp(QSize(120,120));
    {
      pixTemp.fill(Qt::transparent);
      QPainter p;
      p.begin(&pixTemp);
      
      p.drawPixmap( (120 - i.width())/2,0, QPixmap::fromImage(i));
      p.end();
    }
    
    QPixmapCache::Key key = m_keys_urls.value( url );
    m_keys_urls[url] = QPixmapCache::insert( pixTemp );
}

/* ---------------------------------------------------------------------------*/
/* CoverCache::get_default_pixmap                                             */
/* ---------------------------------------------------------------------------*/  
QPixmap CoverCache::get_default_pixmap(bool isStream/*=false*/)
{
    QPixmap p_in;
    if(isStream)
        p_in = QPixmap(":/images/media-url-110x110.png");
    else
        p_in = QPixmap(":/images/default-cover-120x120.png");
    

    QPixmap p_out(p_in.size());
    p_out.fill(Qt::transparent);
    QPainter p(&p_out);
    p.setOpacity(0.2);
    p.drawPixmap(0, 0, p_in);
    p.end();      
      
    if(isStream)
      QPixmapCache::insert( urlCoverKey, p_out );
    else
      QPixmapCache::insert( noCoverKey, p_out );
    
    return p_out;
}


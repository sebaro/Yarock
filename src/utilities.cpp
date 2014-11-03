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

#include "utilities.h"

#include <stdlib.h>

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#include <QtCore>
#include <QPainter>

/*
********************************************************************************
*                                                                              *
*    namespace UTIL                                                            *
*                                                                              *
********************************************************************************
*/
QString UTIL::getConfigDir()
{
    QString configdir;

#ifdef TEST_FLAG
    if (getenv("XDG_CONFIG_HOME")==NULL) {
        configdir = getenv("HOME");
        configdir.append("/.config/yarockTEST");
    } else {
        configdir = getenv("XDG_CONFIG_HOME");
        configdir.append("/.yarockTEST");
    }
#else
    if (getenv("XDG_CONFIG_HOME")==NULL) {
        configdir = getenv("HOME");
        configdir.append("/.config/yarock");
    } else {
        configdir = getenv("XDG_CONFIG_HOME");
        configdir.append("/.yarock");
    }
#endif
  return configdir;
}



QString UTIL::getConfigFile()
{
    return QString(UTIL::getConfigDir() + QDir::separator() + "yarock-1.conf");
}


int UTIL::randomInt(int low, int high)
{
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}


QString UTIL::durationToString(int duration_second)
{
    int seconds = qAbs(duration_second);
    int hours   = seconds / (60*60);
    int minutes = (seconds / 60) % 60;
    seconds %= 60;

    QString ret;
    if (hours)
      ret.sprintf("%d:%02d:%02d", hours, minutes, seconds);
    else
      ret.sprintf("%d:%02d", minutes, seconds);

    return ret;
}

void UTIL::urlAddQueryItem( QUrl& url, const QString& key, const QString& value )
{
  #if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
      QUrlQuery urlQuery( url );
      urlQuery.addQueryItem( key, value );
      url.setQuery( urlQuery );
  #else
      url.addQueryItem( key, value );
  #endif
}

bool UTIL::urlHasQueryItem( const QUrl& url, const QString& key )
{
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    return QUrlQuery( url ).hasQueryItem( key );
#else
    return url.hasQueryItem( key );
#endif
}

QPixmap
UTIL::createRoundedImage( const QPixmap& pixmap, const QSize& size, float frameWidthPct )
{
    int height;
    int width;

    if ( !size.isEmpty() )
    {
        height = size.height();
        width = size.width();
    }
    else
    {
        height = pixmap.height();
        width = pixmap.width();
    }

    if ( !height || !width )
        return QPixmap();

    QPixmap scaledAvatar = pixmap.scaled( width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    if ( frameWidthPct == 0.00 )
        return scaledAvatar;

    QPixmap frame( width, height );
    frame.fill( Qt::transparent );

    QPainter painter( &frame );
    painter.setRenderHint( QPainter::Antialiasing );

    QRect outerRect( 0, 0, width, height );
    QBrush brush( scaledAvatar );
    QPen pen;
    pen.setColor( Qt::transparent );
    pen.setJoinStyle( Qt::RoundJoin );

    painter.setBrush( brush );
    painter.setPen( pen );
    painter.drawRoundedRect( outerRect, frameWidthPct * 100.0, frameWidthPct * 100.0, Qt::RelativeSize );

    return frame;
}

QPixmap
UTIL::squareCenterPixmap( const QPixmap& sourceImage )
{
    if ( sourceImage.width() != sourceImage.height() )
    {
        const int sqwidth = qMin( sourceImage.width(), sourceImage.height() );
        const int delta = abs( sourceImage.width() - sourceImage.height() );

        if ( sourceImage.width() > sourceImage.height() )
        {
            return sourceImage.copy( delta / 2, 0, sqwidth, sqwidth );
        }
        else
        {
            return sourceImage.copy( 0, delta / 2, sqwidth, sqwidth );
        }
    }

    return sourceImage;
}


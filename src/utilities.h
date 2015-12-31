/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2016 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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
#ifndef _UTIL_H_
#define _UTIL_H_

#include <QString>
#include <QStyle>
#include <QApplication>
#if QT_VERSION < 0x050000
#include <QCleanlooksStyle>
#endif

#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QSize>
#include <QUrl>

static QStyle *custom_style = 0;

namespace UTIL 
{
/* ---------------------------------------------------------------------------*/
/*      common math method                                                    */
/* ---------------------------------------------------------------------------*/        
    int randomInt(int low, int high);

/* ---------------------------------------------------------------------------*/
/*      configuration static method                                           */
/* ---------------------------------------------------------------------------*/    
    QString getConfigDir();
    QString getConfigFile();

    static const QString CONFIGDIR  = getConfigDir();

    static const QString CONFIGFILE = getConfigFile();


/* ---------------------------------------------------------------------------*/
/*      string helper method                                                  */
/* ---------------------------------------------------------------------------*/    
    QString deltaTimeToString(int seconds);
    QString durationToString(int seconds);

    static inline QString prettyTrackNumber(int number)
    {
        QString ret;
        ret.sprintf("%02d", number);
        return ret;
    }

/* ---------------------------------------------------------------------------*/
/*      url common method                                                     */
/* ---------------------------------------------------------------------------*/
    void urlAddQueryItem( QUrl& url, const QString& key, const QString& value );
    bool urlHasQueryItem( const QUrl& url, const QString& key );
    
/* ---------------------------------------------------------------------------*/
/*      image and pixmap method                                               */
/* ---------------------------------------------------------------------------*/
    QPixmap createRoundedImage( const QPixmap& avatar, const QSize& size=QSize(), float frameWidthPct = 0.10 );  
   
    QPixmap squareCenterPixmap( const QPixmap& sourceImage );
   
    QImage artistImageFromByteArray(QByteArray array);

   
/* ---------------------------------------------------------------------------*/
/*      painting method                                                       */
/* ---------------------------------------------------------------------------*/
    void drawPlayingIcon(QPainter* painter, int size, int margin, QPoint pos);
    
/* ---------------------------------------------------------------------------*/
/*      style helper method                                                   */
/* ---------------------------------------------------------------------------*/    
    static inline QStyle* getStyle()
    {
      if(!custom_style) 
      {
        #if QT_VERSION >= 0x050000
          custom_style = QApplication::style();
        #else
          if(QApplication::style()->inherits("QGtkStyle"))
            custom_style = new QCleanlooksStyle();
          else
            custom_style = QApplication::style();
        #endif
      }
      return  custom_style;
    }    
}

#endif // _UTIL_H_

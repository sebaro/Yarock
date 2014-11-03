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
#ifndef _UTIL_H_
#define _UTIL_H_

#include <QString>
#include <QStyle>
#include <QApplication>
#if QT_VERSION < 0x050000
#include <QCleanlooksStyle>
#endif

#include <QPixmap>
#include <QSize>
#include <QUrl>

static QStyle *custom_style = 0;

namespace UTIL 
{
  QString getConfigDir();
  QString getConfigFile();

  static const QString CONFIGDIR  = getConfigDir();

  static const QString CONFIGFILE = getConfigFile();

  int randomInt(int low, int high);

  QString durationToString(int duration_second);

  void urlAddQueryItem( QUrl& url, const QString& key, const QString& value );
  bool urlHasQueryItem( const QUrl& url, const QString& key );
    
  static inline QStyle* getStyle()
  {

    if(!custom_style) {
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
  
   QPixmap createRoundedImage( const QPixmap& avatar, const QSize& size=QSize(), float frameWidthPct = 0.10 );  
   
   QPixmap squareCenterPixmap( const QPixmap& sourceImage );
}

#endif // _UTIL_H_

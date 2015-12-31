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

//! this file is extract from Clementine Player 0.7.1
//! thanks for all good works for Clementine developers ;)

#ifndef _ULTIMATE_LYRICS_READER_H_
#define _ULTIMATE_LYRICS_READER_H_

#include <QObject>
#include <QXmlStreamReader>

#include "ultimatelyricsprovider.h"


class QIODevice;

/*
********************************************************************************
*                                                                              *
*    Class UltimateLyricsReader                                                *
*                                                                              *
********************************************************************************
*/

class UltimateLyricsReader : public QObject {
  Q_OBJECT

public:
  UltimateLyricsReader(QObject* parent = 0);

  QList<UltimateLyricsProvider*> Parse(const QString& filename) const;
  QList<UltimateLyricsProvider*> ParseDevice(QIODevice* device) const;

private:
  UltimateLyricsProvider* ParseProvider(QXmlStreamReader* reader) const;
  UltimateLyricsProvider::Rule ParseRule(QXmlStreamReader* reader) const;
  QString ParseInvalidIndicator(QXmlStreamReader* reader) const;
};

#endif // _ULTIMATE_LYRICS_READER_H_

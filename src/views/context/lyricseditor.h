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

#ifndef _LYRICS_EDITOR_H_
#define _LYRICS_EDITOR_H_

#include "info_system.h"

#include <QDialog>
#include <QString>
#include <QLabel>
#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QPlainTextEdit>


/*
********************************************************************************
*                                                                              *
*    Class LyricsEditor                                                        *
*                                                                              *
********************************************************************************
*/
class LyricsEditor : public QDialog
{
Q_OBJECT
  public:
    LyricsEditor(QWidget *parent = 0);
    void setInfo( const QString & artistName,
                  const QString & songName
                );
    void setLyrics(const QString& lyrics);

  private :
    QString            _artistName;
    QLabel             *_songName;
    QDialogButtonBox   *_buttonBox;
    QPlainTextEdit     *_lyricsText;
    void saveLyrics();

  private slots:
    void clicked(QAbstractButton*);

  signals:
    void lyricsChanged(INFO::InfoRequestData, QVariant);
};

#endif // _LYRICS_EDITOR_H_

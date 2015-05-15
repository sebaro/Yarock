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

#ifndef _TRACKSLIDERPOPUP_H_
#define _TRACKSLIDERPOPUP_H_

#include <QWidget>

class SeekSliderPopup : public QWidget {
  Q_OBJECT

public:
  SeekSliderPopup(QWidget* parent);

public slots:
  void SetText(const QString& text);
  void SetSmallText(const QString& small_text);
  void SetPopupPosition(const QPoint& pos);

protected:
  void paintEvent(QPaintEvent*);

private:
  void UpdatePixmap();
  void UpdatePosition();

private:
  QString         m_text;
  QString         m_small_text;
  QPoint          m_pos;

  QFont           m_font;
  QFont           m_small_font;
  QFontMetrics    m_font_metrics;
  QFontMetrics    m_small_font_metrics;

  QPixmap         m_pixmap;
  QPixmap         m_background_cache;
};

#endif // _TRACKSLIDERPOPUP_H_

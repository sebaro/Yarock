/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2018 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

#ifndef TAGWIDGET_H
#define TAGWIDGET_H


#include <QWidget>

class TagWidget : public QWidget 
{
  Q_OBJECT

public:
  TagWidget(const QString& text, QWidget* parent);

  static const int kIconSize;
  static const int kIconTextSpacing;
  static const int kHPadding;
  static const int kVPadding;

  QSize sizeHint() const;
  QSize minimumSize() const;
  
  QString text() const { return m_text; }

signals:
  void Clicked();

protected:
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);
  void paintEvent(QPaintEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void contextMenuEvent(QContextMenuEvent*);

private:
    QString     m_text;
    qreal       m_alpha;
    QColor      m_brush_color;
};

#endif // TAGWIDGET_H

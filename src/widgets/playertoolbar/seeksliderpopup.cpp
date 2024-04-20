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

#include "seeksliderpopup.h"
#include "settings.h"

#include <QBitmap>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QWheelEvent>
#include <QtDebug>

#define kTextMargin   4
#define kPointLength  16
#define kPointWidth   4
#define kBorderRadius 4


SeekSliderPopup::SeekSliderPopup(QWidget* parent) : QWidget(parent),
    m_font_metrics(fontMetrics()),
    m_small_font_metrics(fontMetrics())
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setMouseTracking(false);

    m_font.setPointSizeF(8);
    m_font.setBold(true);
    m_small_font.setPointSizeF(8);
    m_font_metrics = QFontMetrics(m_font);
    m_small_font_metrics = QFontMetrics(m_small_font);
}

void SeekSliderPopup::SetText(const QString& text)
{
    m_text = text;
    UpdatePixmap();
}

void SeekSliderPopup::SetSmallText(const QString& text)
{
    m_small_text = text;
    UpdatePixmap();
}

void SeekSliderPopup::SetPopupPosition(const QPoint& pos)
{
    m_pos = pos;
    UpdatePosition();
}

void SeekSliderPopup::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, m_pixmap);
}

void SeekSliderPopup::UpdatePixmap()
{
  const int text_width = qMax(m_font_metrics.horizontalAdvance(m_text), m_small_font_metrics.horizontalAdvance(m_small_text));

  const QRect text_rect1(kTextMargin, kTextMargin,
                         text_width + 2, m_font_metrics.height());
  const QRect text_rect2(kTextMargin, text_rect1.bottom(),
                         text_width, m_small_font_metrics.height());

  const int bubble_bottom = text_rect2.bottom() + kTextMargin;

  const QRect total_rect(0, 0, text_rect1.right() + kTextMargin, bubble_bottom + kPointLength);

  const QRect bubble_rect(0, 0,total_rect.width(), bubble_bottom);

  const QColor brush_color = SETTINGS()->_baseColor;

  if (m_background_cache.size() != total_rect.size())
  {
    QPolygon pointy;
    pointy << QPoint(total_rect.width()/2 - kPointWidth, bubble_bottom)
           << QPoint(total_rect.width()/2, total_rect.bottom())
           << QPoint(total_rect.width()/2 + kPointWidth, bubble_bottom);

    m_background_cache = QPixmap(total_rect.size());
    m_background_cache.fill(Qt::transparent);

    QPainter p(&m_background_cache);
    p.setRenderHint(QPainter::Antialiasing);
    //p.setRenderHint(QPainter::HighQualityAntialiasing);

    /* draw bubble */
    p.setPen(Qt::NoPen);
    p.setBrush(brush_color);
    p.drawRoundedRect(bubble_rect, kBorderRadius, kBorderRadius);

    /* draw pointy */
    p.drawPolygon(pointy);
  }

  m_pixmap = QPixmap(total_rect.size());
  m_pixmap.fill(Qt::transparent);

  QPainter p(&m_pixmap);
  p.setRenderHint(QPainter::Antialiasing);
  //p.setRenderHint(QPainter::HighQualityAntialiasing);

  /* draw background */
  p.drawPixmap(total_rect.topLeft(), m_background_cache);

  /* draw text */
  if(brush_color.lightness() > 200)
      p.setPen(QColor(Qt::black));
  else
      p.setPen(QColor(Qt::white));

  p.setFont(m_font);
  p.drawText(text_rect1, Qt::AlignHCenter, m_text);

  p.setFont(m_small_font);
  p.setOpacity(0.65);
  p.drawText(text_rect2, Qt::AlignHCenter, m_small_text);

  p.end();

  resize(m_pixmap.size());
  UpdatePosition();
  update();
}

void SeekSliderPopup::UpdatePosition()
{
    move( m_pos.x() - m_pixmap.width() / 2,
          m_pos.y() - m_pixmap.height() );
}


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

#include "tagwidget.h"
#include "settings.h"

#include <QPainter>
#include <QPalette>
#include <QApplication>

const int TagWidget::kIconSize = 16;
const int TagWidget::kIconTextSpacing = 8;
const int TagWidget::kHPadding = 6;
const int TagWidget::kVPadding = 2;

TagWidget::TagWidget(const QString& text, QWidget* parent) : QWidget(parent)
{
    m_text = text;

    m_alpha = 0.6;

    m_brush_color = SETTINGS()->_baseColor;
    
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QSize TagWidget::sizeHint() const 
{
  QSize text = fontMetrics().size(0, m_text);
    
  return QSize(kHPadding + + text.width() + kHPadding,
               kVPadding + text.height() + kVPadding);
}

QSize TagWidget::minimumSize() const
{
    return sizeHint();
}


void TagWidget::enterEvent(QEvent*) 
{
    m_alpha = 1.0;
    update();
}

void TagWidget::leaveEvent(QEvent*) 
{
    m_alpha = 0.6;
    update();
}

void TagWidget::paintEvent(QPaintEvent*) 
{
    QPainter p(this);

    const QRect tag_rect( rect() );
    const QRect text_rect( QPoint(kHPadding, 0), QSize(tag_rect.width() - 2*kHPadding, tag_rect.height()));

    /* ----- Background ----- */
    m_brush_color.setAlphaF(m_alpha);
  
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen( m_brush_color, 0.1, Qt::SolidLine, Qt::RoundCap));
    p.setBrush( m_brush_color );
    p.drawRoundedRect(tag_rect, 5, 5);

    /* ----- Text ----- */
    p.setOpacity(1.0);
    if(m_brush_color.lightness() > 200)
      p.setPen(QColor(Qt::black));
    else
      p.setPen(QColor(Qt::white));
    
    p.drawText(text_rect, m_text);
}


void TagWidget::mouseReleaseEvent(QMouseEvent*) 
{
  emit Clicked();
}


void TagWidget::contextMenuEvent(QContextMenuEvent*) 
{
  emit Clicked();
}


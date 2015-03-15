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

#include "item_button.h"
#include "settings.h"

#include <QApplication>
#include <QPainter>
/*
********************************************************************************
*                                                                              *
*    Class ButtonItem                                                          *
*                                                                              *
********************************************************************************
*/
ButtonItem::ButtonItem()
{
#if QT_VERSION < 0x050000
    setAcceptsHoverEvents(true);
#else
    setAcceptHoverEvents(true);
#endif    
    setAcceptDrops(false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

    setCursor(Qt::PointingHandCursor);
    m_pixmap = QPixmap();

    setGraphicsItem(this);

    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

    m_brush_color = SETTINGS()->_baseColor;     
    m_alpha = 0.6;
}

void ButtonItem::setPixmap(const QPixmap &p )
{
    m_pixmap = p.scaled(QSize(18,18), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void ButtonItem::setText(const QString& t)
{
    m_text = t;
}

void ButtonItem::setData(QVariant v)
{
    m_data = v;
}

void ButtonItem::setColor(QColor color)
{
    m_brush_color = color;
    update();
}

QRectF ButtonItem::boundingRect() const
{
    QFont font = QApplication::font();
    font.setBold( true );

    QFontMetrics fm( font );
    int width = fm.width(m_text);

    int margin = width == 0 ? 8 : 12;
    
    return QRectF(0, 0, width + m_pixmap.width()  + margin, 26);
}

void ButtonItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
Q_UNUSED(widget)

    m_brush_color.setAlphaF(m_alpha);

    QRect rect = boundingRect().toRect();
    painter->setPen(QPen( m_brush_color, 0.1, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(QBrush( m_brush_color ,Qt::SolidPattern));
    painter->drawRoundedRect(rect, 4.0, 4.0);

    //! draw pixmap
    painter->drawPixmap(4,4,m_pixmap);

    //! draw text
    QFont font = QApplication::font();
    font.setBold( true );
    painter->setFont(font);

    if(m_brush_color.lightness() > 200)
      painter->setPen(QColor(Qt::black));
    else
      painter->setPen(QColor(Qt::white));
    painter->drawText(rect.adjusted(m_pixmap.width(), 0, 0, 0), Qt::AlignCenter | Qt::AlignVCenter, m_text);
}

// Inherited from QGraphicsLayoutItem
void ButtonItem::setGeometry(const QRectF &geom)
{
    setPos(geom.topLeft());
}

// Inherited from QGraphicsLayoutItem
QSizeF ButtonItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which);
Q_UNUSED(constraint);
    return boundingRect().size();
}


void ButtonItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
    emit clicked();
}


void ButtonItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    m_alpha = 0.8;
    this->update();
}

void ButtonItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    m_alpha = 0.6;
    this->update();
}



/*
********************************************************************************
*                                                                              *
*    Class ButtonStateItem                                                     *
*                                                                              *
********************************************************************************
*/
ButtonStateItem::ButtonStateItem() : ButtonItem()
{
    m_checked     = false;
}

void ButtonStateItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
Q_UNUSED(widget)
    m_brush_color = SETTINGS()->_baseColor;

    if(m_checked)
      m_brush_color.setAlphaF(0.8);
    else
      m_brush_color.setAlphaF(m_alpha);


    QRect rect = boundingRect().toRect();
    painter->setPen(QPen( m_brush_color, 0.1, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(QBrush( m_brush_color ,Qt::SolidPattern));
    painter->drawRoundedRect(rect, 4.0, 4.0);

    //! draw pixmap
    painter->drawPixmap(4,4,m_pixmap);

    //! draw text
    QFont font = QApplication::font();
    font.setBold( true );
    painter->setFont(font);
    painter->setPen(QColor(Qt::white));
    painter->drawText(rect.adjusted(m_pixmap.width(), 0, 0, 0), Qt::AlignCenter | Qt::AlignVCenter, m_text);
}

void ButtonStateItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
    m_checked = true;
    ButtonItem::mousePressEvent(event);
}

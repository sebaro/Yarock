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

#ifndef _STREAM_ITEM_H_
#define _STREAM_ITEM_H_

#include "item_common.h"

#include "core/mediaitem/mediaitem.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>


/*
********************************************************************************
*                                                                              *
*    Class StreamGraphicItem                                                   *
*                                                                              *
********************************************************************************
*/
class StreamGraphicItem : public QGraphicsItem
{
  public:
    StreamGraphicItem();
    int type() const { return GraphicsItem::StreamType; }
    MEDIA::TrackPtr    media;

    void startDrag(QWidget*);

  protected:
    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *e);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *e);
    void mousePressEvent(QGraphicsSceneMouseEvent *e);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    
    int   m_coverSize;
  public:
    #if QT_VERSION >= 0x050000
    QStyleOptionViewItem opt;
    #else
    QStyleOptionViewItemV4 opt;
    #endif
};
#endif // _STREAM_ITEM_H_

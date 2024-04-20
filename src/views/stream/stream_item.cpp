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

#include "stream_item.h"
#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/mediamimedata.h"
#include "covers/covercache.h"
#include "utilities.h"

#include "global_actions.h"
#include "settings.h"
#include "debug.h"

//! Qt
#include <QtCore>
#include <QPainter>
#include <QDrag>
#include <QGraphicsSceneEvent>


/*
********************************************************************************
*                                                                              *
*    StreamGraphicItem                                                         *
*      -> with image                                                           *
*                                                                              *
********************************************************************************
*/
StreamGraphicItem::StreamGraphicItem()
{
#if QT_VERSION < 0x050000
    setAcceptsHoverEvents(true);
#else
    setAcceptHoverEvents(true);
#endif    
   setAcceptDrops(false);
   setFlag(QGraphicsItem::ItemIsSelectable, true);
   setFlag(QGraphicsItem::ItemIsMovable, false);
   setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

   //! option configuration
   opt.widget = 0;
   opt.palette = QApplication::palette();
   opt.font = QApplication::font();
   opt.font.setStyleStrategy(QFont::PreferAntialias);    
   opt.fontMetrics = QFontMetrics(opt.font);

   opt.showDecorationSelected = true;
   opt.decorationPosition  = QStyleOptionViewItem::Left;
   opt.decorationAlignment = Qt::AlignCenter;
   opt.displayAlignment    = Qt::AlignLeft|Qt::AlignVCenter;

   opt.locale.setNumberOptions(QLocale::OmitGroupSeparator);
   opt.state &= ~ QStyle::State_Active;
   opt.state |= QStyle::State_Enabled;
   opt.state &= ~QStyle::State_Selected;
    
    
   m_coverSize = SETTINGS()->_coverSize;
   
   opt.rect = boundingRect().toRect();
   opt.palette.setColor(QPalette::Active, QPalette::Highlight, SETTINGS()->_baseColor);
   opt.palette.setColor(QPalette::Inactive, QPalette::Highlight, QApplication::palette().color(QPalette::Normal,QPalette::Highlight));   
}

QRectF StreamGraphicItem::boundingRect() const
{
    return QRectF(0, 0, m_coverSize*1.25, m_coverSize + opt.fontMetrics.height()*2 + 4);
}


void StreamGraphicItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget *widget)
{
Q_UNUSED(option)
   //! get Model data
   if(!media) return;
   
   
    /* Get color for state */
    if(media->isPlaying)
    {
        opt.state |= QStyle::State_Selected;
        opt.state |= QStyle::State_Active;
    }    
    else 
    {
        opt.state &= ~QStyle::State_Active;
        opt.state &= ~QStyle::State_Selected;
        if(isSelected())
        {
            opt.state |= QStyle::State_Selected;
        }
    }

    /* Draw frame for State_HasFocus item */
    UTIL::getStyle()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    painter->drawPixmap(QPointF((opt.rect.size().width()- m_coverSize)/2,2),CoverCache::instance()->cover(media));


    
    /* paint stream/radio title */
    painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText) );
    painter->setFont(opt.font);

    const QString elided_name = opt.fontMetrics.elidedText ( media->extra["station"].toString(), Qt::ElideRight, (m_coverSize*1.25)-20);
    painter->drawText(QRect (10,m_coverSize,(m_coverSize*1.25)-20, 25), Qt::AlignTop | Qt::AlignHCenter,elided_name );
    
    
    /* paint bitrate */
    painter->setFont( UTIL::alternateFont() );
    painter->setPen(opt.palette.color ( QPalette::Disabled, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));
    painter->drawText(QRect(0, m_coverSize +3 + opt.fontMetrics.height() + 2, m_coverSize*1.25, 25),  Qt::AlignTop | Qt::AlignHCenter, "#" + media->extra["bitrate"].toString());

    
    /* paint playing or favorite attibute */
    if(media->isBroken)
        painter->drawPixmap(2, 1, QPixmap(":/images/media-broken-18x18.png"));

    if(media->isFavorite)
        painter->drawPixmap(0, 32, QPixmap(":/images/favorites-18x18.png"));
}

void StreamGraphicItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- StreamGraphicItem::hoverEnterEvent";
    opt.state |= QStyle::State_MouseOver;
    this->update();
}

void StreamGraphicItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- StreamGraphicItem::hoverLeaveEvent";
    opt.state &= ~QStyle::State_MouseOver;
    this->update();
}

void StreamGraphicItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}


void StreamGraphicItem::startDrag(QWidget* w)
{
    setCursor(Qt::OpenHandCursor);

    MediaMimeData* mimedata = new MediaMimeData(SOURCE_COLLECTION);
    mimedata->addTrack(this->media);

    QDrag *drag = new QDrag(w);
    drag->setMimeData(mimedata);
    drag->exec();
}

void StreamGraphicItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
    QVariant v;
    v.setValue(static_cast<QGraphicsItem*>(this));

    (ACTIONS()->value(BROWSER_STREAM_ITEM_MOUSE_MOVE))->setData(v);
    (ACTIONS()->value(BROWSER_STREAM_ITEM_MOUSE_MOVE))->trigger();
}


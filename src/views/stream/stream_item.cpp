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

#include "stream_item.h"
#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/mediamimedata.h"
#include "covers/covercache.h"
#include "utilities.h"

#include "global_actions.h"
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
   opt.state |= QStyle::State_Active;
   opt.state |= QStyle::State_Enabled;
   _width = 530;
}

QRectF StreamGraphicItem::boundingRect() const
{
    int height  = opt.fontMetrics.height()*2 + 4 ;

    if(_width < 530)
      return QRectF(0, 0, 530, height + 4 * 2 );
    
    
    return QRectF(0, 0, _width, height + 4 * 2 );
}


void StreamGraphicItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget *widget)
{
Q_UNUSED(option)
   //! get Model data
   if(!media) return;
   const int width       = _width < 530 ? 530 : _width;
   const int height      = boundingRect().toRect().height();
    
   //! Get color for state
   QColor c = QApplication::palette().color(QPalette::Normal,QPalette::Highlight);

   if(isSelected())
   {
     opt.state |= QStyle::State_Selected;
     opt.palette.setColor(QPalette::Normal, QPalette::Highlight, c);
   }
   else if (opt.state & QStyle::State_MouseOver) {
     opt.state |= QStyle::State_Selected;
     c.setAlpha(100);
     opt.palette.setColor(QPalette::Normal, QPalette::Highlight, c);
   }
   else {
     opt.state &= ~QStyle::State_Selected;
   }

   //! Draw frame for State_HasFocus item
   opt.rect = boundingRect().toRect().adjusted(0,0,-15,0);
   UTIL::getStyle()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

   //! paint stream name
   QFont bold_font = opt.font;
   bold_font.setBold(true);
   QFontMetrics fm = QFontMetrics(bold_font);

   painter->setFont(bold_font);
   painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));

   const QString name_elided = fm.elidedText ( media->name, Qt::ElideRight, width-20);
   painter->drawText(QRect(30, height/2-fm.height(), width-20, fm.height()), Qt::AlignLeft | Qt::AlignVCenter,name_elided);

   //! paint stream url
   painter->setPen(opt.palette.color ( QPalette::Disabled, isSelected() ? QPalette::HighlightedText : QPalette::WindowText) );
   painter->setFont(opt.font);

   const QString url_elided = opt.fontMetrics.elidedText ( media->url, Qt::ElideRight, width-20);
   painter->drawText(QRect(30, height/2+1, width-20, height/2), Qt::AlignLeft | Qt::AlignVCenter,url_elided);
   
   //! paint activated item
   if(media->isPlaying)
     painter->drawPixmap(0, height/2-10,QPixmap(":/images/media-playing.png"));
   else if(media->isBroken)
     painter->drawPixmap(2, height/2-10, QPixmap(":/images/media-broken-18x18.png"));
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




/*
********************************************************************************
*                                                                              *
*    StreamGraphicItem_v2                                                      *
*                                                                              *
********************************************************************************
*/
StreamGraphicItem_v2::StreamGraphicItem_v2()
{

}

QRectF StreamGraphicItem_v2::boundingRect() const
{
    if(_width < 530)
      return QRectF(0, 0, 530, 70);

    return QRectF(0, 0, _width, 70);
}

void StreamGraphicItem_v2::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
   //! get Model data
   if(!media) return;
   const int width       = _width < 530 ? 530 : _width;
   const int height      = boundingRect().toRect().height();
   
   //! Get color for state
   QColor c = QApplication::palette().color(QPalette::Normal,QPalette::Highlight);

   if(isSelected())
   {
     //! paint stream name
     painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));
     painter->setFont(opt.font);

     const QString stream_elided = opt.fontMetrics.elidedText ( media->name, Qt::ElideRight, width-90);
     painter->drawText(QRect(75, 22, width-90, 22), Qt::AlignLeft | Qt::AlignVCenter,stream_elided);
      
     opt.state |= QStyle::State_Selected;
     opt.palette.setColor(QPalette::Normal, QPalette::Highlight, c);
   }
   else if (opt.state & QStyle::State_MouseOver) {
     opt.state |= QStyle::State_Selected;
     c.setAlpha(100);
     opt.palette.setColor(QPalette::Normal, QPalette::Highlight, c);
   }
   else {
     opt.state &= ~QStyle::State_Selected;
   }


   //! Draw frame for State_HasFocus item
   opt.rect = boundingRect().toRect().adjusted(0,0,-15,0);
   UTIL::getStyle()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

   //! paint stream name
   QFont bold_font = opt.font;
   bold_font.setBold(true);
   QFontMetrics fm = QFontMetrics(bold_font);

   painter->setFont(bold_font);
   painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));

   const QString name_elided = fm.elidedText ( media->name, Qt::ElideRight, width-115);
   painter->drawText(QRect(95, height/2-fm.height(), width-90, fm.height()), Qt::AlignLeft | Qt::AlignVCenter,name_elided);

   //! paint stream url
   painter->setPen(opt.palette.color ( QPalette::Disabled, isSelected() ? QPalette::HighlightedText : QPalette::WindowText) );
   painter->setFont(opt.font);

   const QString url_elided = opt.fontMetrics.elidedText ( media->url, Qt::ElideRight, width-115);
   painter->drawText(QRect(95, height/2+1, width-90, fm.height()), Qt::AlignLeft | Qt::AlignVCenter,url_elided);
      
  
   //! paint image   
   if(media->isPlaying)
     painter->drawPixmap(70, 22,QPixmap(":/images/media-playing.png"));
   else if(media->isBroken)
     painter->drawPixmap(70, 22, QPixmap(":/images/media-broken-18x18.png"));
   else
   {
     QPixmap pix = CoverCache::instance()->cover(media).scaled(QSize(64, 64), Qt::KeepAspectRatio, Qt::SmoothTransformation);
     painter->drawPixmap(3,3, pix);
   }
}


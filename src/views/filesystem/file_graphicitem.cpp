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

#include "file_graphicitem.h"
#include "utilities.h"
#include "global_actions.h"
#include "debug.h"

#include <QtCore>
#include <QPainter>    
/*
********************************************************************************
*                                                                              *
*    DirectoryGraphicItem                                                      *
*                                                                              *
********************************************************************************
*/
DirectoryGraphicItem::DirectoryGraphicItem()
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
    opt.fontMetrics = QFontMetrics(opt.font);

    opt.showDecorationSelected = true;
    opt.decorationPosition = QStyleOptionViewItem::Top;
    opt.displayAlignment = Qt::AlignCenter;

    opt.locale.setNumberOptions(QLocale::OmitGroupSeparator);
    opt.state |= QStyle::State_Active;
    opt.state |= QStyle::State_Enabled;
    opt.state &= ~QStyle::State_Selected;
}


QRectF DirectoryGraphicItem::boundingRect() const
{
    return QRectF(0, 0, 130, 130);
}

void DirectoryGraphicItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)

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
    opt.rect = boundingRect().toRect();
    UTIL::getStyle()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    QPixmap pixTemp(QSize(130,130));
    {
      pixTemp.fill(Qt::transparent);
      QPainter p;
      p.begin(&pixTemp);

      //! paint cover art
      p.drawPixmap(20,2, pixmap);
      p.end();
    }
    painter->drawPixmap(0, 0, pixTemp);
    
   //! paint directory name
   painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText) );
   painter->setFont(opt.font);    
   
   const QString elided_dirname = opt.fontMetrics.elidedText ( m_dirname, Qt::ElideRight, 130);
   
   painter->drawText(QRect (0,pixmap.height()+5,130, opt.fontMetrics.height()+4), Qt::AlignTop | Qt::AlignHCenter,elided_dirname );   
}


void DirectoryGraphicItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- DirectoryGraphicItem::hoverEnterEvent";
    opt.state |= QStyle::State_MouseOver;
    this->update();
}

void DirectoryGraphicItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- DirectoryGraphicItem::hoverLeaveEvent";
    opt.state &= ~QStyle::State_MouseOver;
    this->update();
}

void DirectoryGraphicItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- DirectoryGraphicItem::mousePressEvent ";
}

void DirectoryGraphicItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
    QVariant v;
    v.setValue(static_cast<QGraphicsItem*>(this));

    (ACTIONS()->value(BROWSER_DIR_ITEM_MOUSE_MOVE))->setData(v);
    (ACTIONS()->value(BROWSER_DIR_ITEM_MOUSE_MOVE))->trigger();
}



/*
********************************************************************************
*                                                                              *
*    Class TrackGraphicItem_v4                                                 *
*      -> copy of TrackGraphicItem_v3                                          *
*      -> work for local playlist or local track                               *
*      -> draw track artist/album/title or track/playlist filepath             *
********************************************************************************
*/
TrackGraphicItem_v4::TrackGraphicItem_v4()
{
    _width        = 530;
}

QRectF TrackGraphicItem_v4::boundingRect() const
{
    if(_width < 510)
      return QRectF(0, 0, 510, 22);

    return QRectF(0, 0, _width, 22);
}

void TrackGraphicItem_v4::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
   //! get data
   const int width       = _width < 530 ? 530 : _width;
   const int title_width = _width < 530 ? 150 : (_width - 150)/3;

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
   opt.rect = boundingRect().toRect();
   UTIL::getStyle()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

   if(!media)
   {
     painter->setPen( opt.palette.color ( isSelected() ? QPalette::Normal : QPalette::Disabled, isSelected() ? QPalette::HighlightedText : QPalette::WindowText) );
     painter->setFont(opt.font);

     const QString name_elided = opt.fontMetrics.elidedText ( m_path, Qt::ElideRight, width -20);
     painter->drawText(QRect(30, 0,  width -20, 22), Qt::AlignLeft | Qt::AlignVCenter, name_elided);
   }    
   else
   {
      bool isTrack       = (media->type() == TYPE_TRACK) ? true : false;
      //! TRACK in collection
      if(isTrack && media->id != -1)
      {
        painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));

        painter->setFont(opt.font);

        //! paint artist name
        const QString artist_elided = opt.fontMetrics.elidedText ( media->artist, Qt::ElideRight, title_width);
        painter->drawText(QRect(30, 0, title_width-50, 22), Qt::AlignLeft | Qt::AlignVCenter,artist_elided);

        //! paint album name
        const QString album_elided = opt.fontMetrics.elidedText ( media->album, Qt::ElideRight, title_width);
        painter->drawText(QRect(60+title_width-50, 0, title_width, 22), Qt::AlignLeft | Qt::AlignVCenter,album_elided);

        //! paint track name
        const QString title_elided = opt.fontMetrics.elidedText ( media->title, Qt::ElideRight, title_width);
        painter->drawText(QRect(60+title_width*2-50, 0, title_width, 22), Qt::AlignLeft | Qt::AlignVCenter,title_elided);

        //! paint track duration
        const QString duree_elided = opt.fontMetrics.elidedText ( media->durationToString(), Qt::ElideRight, 50);
        painter->drawText(QRect(60+title_width*3-50, 0, 50, 22), Qt::AlignRight | Qt::AlignVCenter,duree_elided);

        //! paint track rating
        const float rating_ = media->rating;
        RatingPainter::instance()->Paint(painter, QRect(title_width*3-50 + 115, 0, 80, 22), rating_ , true);
      }
      else {
        painter->setPen( opt.palette.color ( isSelected() ? QPalette::Normal : QPalette::Disabled, isSelected() ? QPalette::HighlightedText : QPalette::WindowText) );
        painter->setFont(opt.font);

        const QString name_elided = opt.fontMetrics.elidedText ( media->url, Qt::ElideRight, width -20);
        painter->drawText(QRect(30, 0,  width -20, 22), Qt::AlignLeft | Qt::AlignVCenter, name_elided);
      }
      
      //! paint activated item
      if(media->isPlaying)
        painter->drawPixmap(0, 0,QPixmap(":/images/media-playing.png"));
      else if(media->isBroken)
        painter->drawPixmap(1, 1, QPixmap(":/images/media-broken-18x18.png"));
      else if (!isTrack)
        painter->drawPixmap(1, 1, QPixmap(":/images/files-18x18.png"));      
   }



}


void TrackGraphicItem_v4::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
    QVariant v;
    v.setValue(static_cast<QGraphicsItem*>(this));

    (ACTIONS()->value(BROWSER_DIR_ITEM_MOUSE_MOVE))->setData(v);
    (ACTIONS()->value(BROWSER_DIR_ITEM_MOUSE_MOVE))->trigger();
}


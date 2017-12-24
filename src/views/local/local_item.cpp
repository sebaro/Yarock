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

#include "local_item.h"

#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/mediamimedata.h"
#include "covers/covercache.h"
#include "utilities.h"

#include "qpixmapfilter.h"

#include "models/local/local_track_model.h"
#include "models/local/local_playlist_model.h"
#include "views/local/local_scene.h"

#include "smartplaylist/smartplaylist.h"

#include "widgets/ratingwidget.h"
#include "global_actions.h"

#include "settings.h"
#include "debug.h"

//! Qt
#include <QPen>
#include <QStyle>
#include <QGraphicsSceneEvent>
#include <QDrag>
#include <QtCore>

/*
********************************************************************************
*                                                                              *
*    AlbumGraphicItem                                                          *
*      -> with artist name/album name                                          *
********************************************************************************
*/
AlbumGraphicItem::AlbumGraphicItem()
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
    opt.decorationPosition = QStyleOptionViewItem::Top;
    opt.displayAlignment = Qt::AlignCenter;

    opt.locale.setNumberOptions(QLocale::OmitGroupSeparator);
    opt.state &= ~ QStyle::State_Active;
    opt.state |= QStyle::State_Enabled;
    opt.state &= ~QStyle::State_Selected;
    
    m_coverSize = SETTINGS()->_coverSize;
    
    opt.rect = boundingRect().toRect();
    opt.palette.setColor(QPalette::Active, QPalette::Highlight, SETTINGS()->_baseColor);
    opt.palette.setColor(QPalette::Inactive, QPalette::Highlight, QApplication::palette().color(QPalette::Normal,QPalette::Highlight));
}


QRectF AlbumGraphicItem::boundingRect() const
{
    return QRectF(0, 0, m_coverSize*1.25, m_coverSize + opt.fontMetrics.height()*2 + 4);
}

void AlbumGraphicItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
    MEDIA::ArtistPtr artist_ptr = MEDIA::ArtistPtr::staticCast(media->parent());

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

    QPixmap pixTemp(opt.rect.size());
    {
       pixTemp.fill(Qt::transparent);
       QPainter p;
       p.begin(&pixTemp);
 
       //! paint cover art
       QPixmap pix = CoverCache::instance()->cover(media);
       p.drawPixmap((opt.rect.size().width()- m_coverSize)/2,2, pix);
       p.end();
    }
 
    /* manual graphics shadow (Qgraphics Effect cause refresh pb) */
    drawShadow(painter,QPointF(0,0),pixTemp,boundingRect());
    

    /* paint album title */
    painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText) );
    painter->setFont(opt.font);

    const QString elided_album = opt.fontMetrics.elidedText ( media->name, Qt::ElideRight, m_coverSize*1.25);
    painter->drawText(QRect (0,m_coverSize+3,m_coverSize*1.25, 25), Qt::AlignTop | Qt::AlignHCenter,elided_album );

    /* album number */
    if(media->disc_number != 0) {
        QColor brush_color = QColor( 200, 200, 255 );
        painter->save();
        painter->setPen(QPen( brush_color, 0.1, Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(QBrush( brush_color ,Qt::SolidPattern));
        painter->drawRoundedRect(QRect(1,36,16,16), 8.0, 8.0);
        painter->restore();
     
        painter->drawText(QRect(0, 35, 18, 18), Qt::AlignTop | Qt::AlignCenter, QString::number(media->disc_number));
    }
   
    /* paint artist name */
    painter->setFont( UTIL::alternateFont() );
    painter->setPen(opt.palette.color ( QPalette::Disabled, isSelected() ? QPalette::HighlightedText : QPalette::WindowText) );

    const QString elided_artist = UTIL::alternateFontMetric().elidedText ( artist_ptr->name, Qt::ElideRight, m_coverSize*1.25, Qt::TextWrapAnywhere);
    painter->drawText(QRect(0, m_coverSize+3 + opt.fontMetrics.height() + 2, m_coverSize*1.25, 25), Qt::AlignTop | Qt::AlignHCenter, elided_artist);

    /* paint favorite attibute */
    if(media->isFavorite)
      painter->drawPixmap(0, 60, QPixmap(":/images/favorites-18x18.png"));
}


void AlbumGraphicItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- AlbumGraphicItem::hoverEnterEvent";
    opt.state |= QStyle::State_MouseOver;
    this->update();
}

void AlbumGraphicItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- AlbumGraphicItem::hoverLeaveEvent";
    opt.state &= ~QStyle::State_MouseOver;
    this->update();
}

void AlbumGraphicItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- AlbumGraphicItem::mousePressEvent ";
}


void AlbumGraphicItem::startDrag(QWidget* w)
{
    //Debug::debug() << " AlbumGraphicItem::startDrag";
    setCursor(Qt::OpenHandCursor);
    MediaMimeData* mimedata = new MediaMimeData(SOURCE_COLLECTION);

    for(int i =0; i < this->media->childCount();i++) {
      if( LocalTrackModel::instance()->isMediaMatch(this->media->child(i)) )
        mimedata->addTrack( MEDIA::TrackPtr::staticCast( this->media->child(i) ) );
    }
    QDrag *drag = new QDrag(w);
    drag->setMimeData(mimedata);

    //! set pixmap
    QPixmap pix = CoverCache::instance()->cover(media);
    drag->setPixmap(pix);
    drag->exec();
}

void AlbumGraphicItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
    QVariant v;
    v.setValue(static_cast<QGraphicsItem*>(this));

    (ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE))->setData(v);
    (ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE))->trigger();
}

/*
********************************************************************************
*                                                                              *
*    AlbumGraphicItem_v2                                                       *
*      -> with album year/album name                                           *
********************************************************************************
*/
AlbumGraphicItem_v2::AlbumGraphicItem_v2()
{
    opt.rect = boundingRect().toRect();
}

void AlbumGraphicItem_v2::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)

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

    QPixmap pixTemp(opt.rect.size());
    {
       pixTemp.fill(Qt::transparent);
       QPainter p;
       p.begin(&pixTemp);
 
       //! paint cover art
       QPixmap pix = CoverCache::instance()->cover(media);
       p.drawPixmap((opt.rect.size().width()- m_coverSize)/2,2, pix);
       p.end();
    }
 
    /* manual graphics shadow (Qgraphics Effect cause refresh pb) */
    drawShadow(painter,QPointF(0,0),pixTemp,boundingRect());

    /* paint album title */
    painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));
    painter->setFont(opt.font);

    const QString elided_album = opt.fontMetrics.elidedText ( media->name, Qt::ElideRight, m_coverSize*1.25);
    painter->drawText(QRect (0,m_coverSize+3,m_coverSize*1.25, 25),  Qt::AlignTop | Qt::AlignHCenter,elided_album );

    /* album number */
    if(media->disc_number != 0) {
        QColor brush_color = QColor( 200, 200, 255 );
        painter->save();
        painter->setPen(QPen( brush_color, 0.1, Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(QBrush( brush_color ,Qt::SolidPattern));
        painter->drawRoundedRect(QRect(1,36,16,16), 8.0, 8.0);
        painter->restore();
     
        painter->drawText(QRect(0, 35, 18, 18), Qt::AlignTop | Qt::AlignCenter, QString::number(media->disc_number));
    }
  
    /* paint year */
    painter->setFont( UTIL::alternateFont() );
    painter->setPen(opt.palette.color ( QPalette::Disabled, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));
    painter->drawText(QRect(0, m_coverSize +3 + opt.fontMetrics.height() + 2, m_coverSize*1.25, 25),  Qt::AlignTop | Qt::AlignHCenter, media->yearToString());

    /* paint favorite attibute */
    if(media->isFavorite)
      painter->drawPixmap(0, 60, QPixmap(":/images/favorites-18x18.png"));
}
/*
********************************************************************************
*                                                                              *
*    AlbumGenreGraphicItem                                                     *
*                                                                              *
********************************************************************************
*/
AlbumGenreGraphicItem::AlbumGenreGraphicItem()
{

}

void AlbumGenreGraphicItem::startDrag(QWidget* w)
{
    //Debug::debug() << " AlbumGenreGraphicItem::startDrag";
    setCursor(Qt::OpenHandCursor);
    MediaMimeData* mimedata = new MediaMimeData(SOURCE_COLLECTION);

    for(int i =0; i < this->media->childCount();i++)
    {
      MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast( this->media->child(i) );
      
      if(!LocalTrackModel::instance()->isMediaMatch(track)) continue;
      if(track->genre == _genre)
        mimedata->addTrack(MEDIA::TrackPtr::staticCast( this->media->child(i) ));
    }
    QDrag *drag = new QDrag(w);
    drag->setMimeData(mimedata);

    //! set pixmap
    QPixmap pix = CoverCache::instance()->cover(media);
    drag->setPixmap(pix);
    drag->exec();
}

void AlbumGenreGraphicItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
    QVariant v;
    v.setValue(static_cast<QGraphicsItem*>(this));

    (ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE))->setData(v);
    (ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE))->trigger();
}


/*
********************************************************************************
*                                                                              *
*    AlbumGraphicItem_v3                                                       *
*      -> album playcount                                                      *
********************************************************************************
*/
AlbumGraphicItem_v3::AlbumGraphicItem_v3()
{
    opt.rect = boundingRect().toRect();
}

QRectF AlbumGraphicItem_v3::boundingRect() const
{
    return QRectF(0, 0, m_coverSize*1.25, /*cover*/m_coverSize + /*text*/opt.fontMetrics.height()*2 + /*playcount*/25 + /*padding*/4);
}

void AlbumGraphicItem_v3::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
    MEDIA::ArtistPtr artist_ptr = MEDIA::ArtistPtr::staticCast(media->parent());
    const QString artist_name   =  !artist_ptr.isNull() ? artist_ptr->name : "";

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

    QPixmap pixTemp( opt.rect.size() );
    {
      pixTemp.fill(Qt::transparent);
      QPainter p;
      p.begin(&pixTemp);

      QPixmap pix = CoverCache::instance()->cover(media);

      p.drawPixmap((opt.rect.size().width()- m_coverSize)/2,21, pix);
      p.end();
    }

    /* manual graphics shadow (Qgraphics Effect cause refresh pb) */
    drawShadow(painter,QPointF(0,0),pixTemp,boundingRect());

    /* paint playcount */
    QColor m_brush_color = QColor(0x4a82dd);
    m_brush_color.setAlphaF(0.6);
    QRect rect = QRect(opt.rect.size().width()/2 - 25,2,50,18);
    painter->setPen(QPen( m_brush_color, 0.1, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(QBrush( m_brush_color ,Qt::SolidPattern));
    painter->drawRoundedRect(rect, 4.0, 4.0);

    painter->setFont(QFont("Arial", 8, QFont::Bold));
    painter->setPen( Qt::white );
    painter->drawText(rect, Qt::AlignCenter, QString::number(media->playcount) );

    /* paint album title */
    painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));
    painter->setFont(opt.font);

    const QString elided_album = opt.fontMetrics.elidedText ( media->name, Qt::ElideRight, m_coverSize*1.25);
    painter->drawText(QRect (0,m_coverSize+22,m_coverSize*1.25,25), Qt::AlignTop | Qt::AlignHCenter,elided_album );

    /* paint artist name */
    painter->setFont( UTIL::alternateFont() );
    painter->setPen(opt.palette.color ( QPalette::Disabled, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));

    const QString elided_artist = UTIL::alternateFontMetric().elidedText ( artist_name, Qt::ElideRight, m_coverSize*1.25);
    painter->drawText(QRect(0, m_coverSize+22+opt.fontMetrics.height(), m_coverSize*1.25, 25), Qt::AlignTop | Qt::AlignHCenter, elided_artist);

    /* paint favorite attibute */
    if(media->isFavorite)
      painter->drawPixmap(0, 60, QPixmap(":/images/favorites-18x18.png"));
}


/*
********************************************************************************
*                                                                              *
*    AlbumGraphicItem_v4                                                       *
*      -> album rating                                                         *
********************************************************************************
*/
AlbumGraphicItem_v4::AlbumGraphicItem_v4()
{
    opt.rect = boundingRect().toRect();
    hover_rating_ = -1.0;
}

QRectF AlbumGraphicItem_v4::boundingRect() const
{
    return QRectF(0, 0, m_coverSize*1.25, /*cover*/m_coverSize + /*text*/opt.fontMetrics.height()*2 + /*rating*/25 + /*padding*/4);
}

void AlbumGraphicItem_v4::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)

    MEDIA::ArtistPtr artist_ptr = MEDIA::ArtistPtr::staticCast(media->parent());
    const QString artist_name   =  !artist_ptr.isNull() ? artist_ptr->name : "";

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

    QPixmap pixTemp(opt.rect.size());
    {
      pixTemp.fill(Qt::transparent);
      QPainter p;
      p.begin(&pixTemp);

      //! paint cover art
      QPixmap pix = CoverCache::instance()->cover(media);
      p.drawPixmap((opt.rect.size().width()- m_coverSize)/2,21,pix);
      p.end();
    }

    //! manual graphics shadow (Qgraphics Effect cause refresh pb)
    drawShadow(painter,QPointF(0,0),pixTemp,boundingRect());

    //! paint album rating
    const float rating_ = media->rating;
    RatingPainter::instance()->Paint(painter, QRect(0, 1, opt.rect.size().width(), 22), hover_rating_ == -1.0 ? rating_ : hover_rating_, media->isUserRating);

    //! paint album title
    painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));
    painter->setFont( opt.font );

    const QString elided_album = opt.fontMetrics.elidedText ( media->name, Qt::ElideRight, m_coverSize*1.25);
    painter->drawText(QRect (0,m_coverSize+22,m_coverSize*1.25, 25), Qt::AlignTop | Qt::AlignHCenter,elided_album );

    //! paint artist name
    painter->setFont( UTIL::alternateFont() );
    painter->setPen(opt.palette.color ( QPalette::Disabled, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));

    const QString elided_artist = UTIL::alternateFontMetric().elidedText ( artist_name, Qt::ElideRight, m_coverSize*1.25);
    painter->drawText(QRect(0, m_coverSize+22+opt.fontMetrics.height()+2, m_coverSize*1.25, 18), Qt::AlignTop | Qt::AlignHCenter, elided_artist);

    /* paint favorite attibute */
    if(media->isFavorite)
      painter->drawPixmap(0, 60, QPixmap(":/images/favorites-18x18.png"));
}

void AlbumGraphicItem_v4::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if(event->pos().toPoint().y() <= 22)
    {
      hover_rating_ = RatingPainter::RatingForPos(event->pos().toPoint(), QRect(0, 1, m_coverSize*1.25, 22));
      this->update();

      // update all track hover rating if selected
      QGraphicsScene* scene = this->scene();
      if(scene->selectedItems().contains(this)) {
        foreach(QGraphicsItem* gi, scene->selectedItems()) {
          AlbumGraphicItem_v4 *item = static_cast<AlbumGraphicItem_v4*>(gi);
          item->setHoverRating(hover_rating_);
          item->update();
        }
      }
    }
}

void AlbumGraphicItem_v4::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->pos().toPoint().y() <= 22)
    {
      media->rating       = RatingPainter::RatingForPos(event->pos().toPoint(), QRect(0, 1, m_coverSize*1.25, 22));
      media->isUserRating = true;

      // update all track hover rating if selected
      QGraphicsScene* scene = this->scene();
      if(scene->selectedItems().contains(this)) {
        foreach(QGraphicsItem* gi, scene->selectedItems()) {
          AlbumGraphicItem_v4 *item = static_cast<AlbumGraphicItem_v4*>(gi);
          item->media->rating = media->rating;
          item->media->isUserRating = true;
        }
      }

      QVariant v;
      v.setValue(static_cast<QGraphicsItem*>(this));

      (ACTIONS()->value(BROWSER_ITEM_RATING_CLICK))->setData(v);
      (ACTIONS()->value(BROWSER_ITEM_RATING_CLICK))->trigger();
    }
}

void AlbumGraphicItem_v4::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //Debug::debug() << " ---- AlbumGraphicItem_v4::hoverLeaveEvent";
    hover_rating_ = -1.0;

    // update all track hover rating if selected
    QGraphicsScene* scene = this->scene();
    if(scene->selectedItems().contains(this)) {
      foreach(QGraphicsItem* gi, scene->selectedItems()) {
        AlbumGraphicItem_v4 *item = static_cast<AlbumGraphicItem_v4*>(gi);
        item->setHoverRating(-1.0);
        item->update();
      }
    }

    AlbumGraphicItem::hoverLeaveEvent(event);
}

/*
********************************************************************************
*                                                                              *
*    ArtistGraphicItem                                                         *
*                                                                              *
********************************************************************************
*/
ArtistGraphicItem::ArtistGraphicItem()
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
    opt.decorationPosition = QStyleOptionViewItem::Top;
    opt.displayAlignment = Qt::AlignCenter;

    opt.locale.setNumberOptions(QLocale::OmitGroupSeparator);
    opt.state &= ~ QStyle::State_Active;
    opt.state |= QStyle::State_Enabled;
    opt.state &= ~QStyle::State_Selected;
    
    m_coverSize = SETTINGS()->_coverSize;

    opt.rect = boundingRect().toRect();
    opt.palette.setColor(QPalette::Active, QPalette::Highlight, SETTINGS()->_baseColor);
    opt.palette.setColor(QPalette::Inactive, QPalette::Highlight, QApplication::palette().color(QPalette::Normal,QPalette::Highlight));
}

QRectF ArtistGraphicItem::boundingRect() const
{
    return QRectF(0, 0, m_coverSize*1.17, m_coverSize + opt.fontMetrics.height()*2 + 4);
}



void ArtistGraphicItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)

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


    /* draw artist image 200x200 */
    QPixmap pixTemp( opt.rect.size() );
    {
      pixTemp.fill(Qt::transparent);
      QPainter p;
      p.begin(&pixTemp);

      //! paint cover art
      QPixmap pix = CoverCache::instance()->image(media, albums_covers);
      
      p.drawPixmap((opt.rect.size().width()-m_coverSize)/2,2,pix);
      p.end();
    }

    /* manual graphics shadow (Qgraphics Effect cause refresh pb) */
    drawShadow( painter, QPointF(0,0), pixTemp, boundingRect());

    /* affichage nom de l'artist */
    painter->setFont(opt.font);
    painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));

    const QString elided_artist = opt.fontMetrics.elidedText ( media->name, Qt::ElideMiddle, m_coverSize*1.17*2, Qt::TextWrapAnywhere);

    painter->drawText(QRect(0, m_coverSize+4, m_coverSize*1.17, opt.fontMetrics.height()*2+4), Qt::AlignTop | Qt::AlignHCenter | Qt::TextWrapAnywhere, elided_artist);

    /* paint favorite attibute */
    if(media->isFavorite)
      painter->drawPixmap(0, 40, QPixmap(":/images/favorites-18x18.png"));
}

void ArtistGraphicItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- ArtistGraphicItem::hoverEnterEvent";
    opt.state |= QStyle::State_MouseOver;
    this->update();
}

void ArtistGraphicItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- ArtistGraphicItem::hoverLeaveEvent";
    opt.state &= ~QStyle::State_MouseOver;
    this->update();
}

void ArtistGraphicItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void ArtistGraphicItem::startDrag(QWidget *w)
{
    setCursor(Qt::OpenHandCursor);

    MediaMimeData* mimedata = new MediaMimeData(SOURCE_COLLECTION);

    mimedata->addTracks (
      LocalTrackModel::instance()->getItemChildrenTracks(this->media)
    );


    QDrag *drag = new QDrag(w);
    drag->setMimeData(mimedata);

    drag->setPixmap( CoverCache::instance()->image(media, albums_covers) );

    drag->exec();
}

void ArtistGraphicItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
    QVariant v;
    v.setValue(static_cast<QGraphicsItem*>(this));

    (ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE))->setData(v);
    (ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE))->trigger();
}

/*
********************************************************************************
*                                                                              *
*    ArtistGraphicItem_v2                                                      *
*      -> artist playcount                                                     *
********************************************************************************
*/
ArtistGraphicItem_v2::ArtistGraphicItem_v2()
{
    opt.rect = boundingRect().toRect();
}

QRectF ArtistGraphicItem_v2::boundingRect() const
{
    return QRectF(0, 0, m_coverSize*1.25, m_coverSize + opt.fontMetrics.height()*2 + 4 +  /*playcount*/25 );
}

void ArtistGraphicItem_v2::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)

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

    /* draw artist image 200x200 */
    QPixmap pixTemp( opt.rect.size() );
    {
      pixTemp.fill(Qt::transparent);
      QPainter p;
      p.begin(&pixTemp);

      //! paint cover art
      QPixmap pix = CoverCache::instance()->image(media, albums_covers);
      p.drawPixmap((opt.rect.size().width()-m_coverSize)/2,21, pix);
      p.end();
    }

    /* manual graphics shadow (Qgraphics Effect cause refresh pb) */
    drawShadow( painter, QPointF(0,0), pixTemp, boundingRect());


    /* paint playcount */
    QColor m_brush_color = QColor(0x4a82dd);
    m_brush_color.setAlphaF(0.6);
    QRect rect = QRect(opt.rect.size().width()/2-25,2,50,18);
    painter->setPen(QPen( m_brush_color, 0.1, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(QBrush( m_brush_color ,Qt::SolidPattern));
    painter->drawRoundedRect(rect, 4.0, 4.0);

    painter->setFont(QFont("Arial", 8, QFont::Bold));
    painter->setPen( Qt::white );
    painter->drawText(rect, Qt::AlignCenter, QString::number(media->playcount) );

    /* paint artist name */
    painter->setFont(opt.font);
    painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));

    const QString elided_artist = opt.fontMetrics.elidedText ( media->name, Qt::ElideRight, m_coverSize*1.25);
    painter->drawText(QRect(0, m_coverSize+22, m_coverSize*1.25, 25), Qt::AlignTop | Qt::AlignHCenter, elided_artist);
    
    /* paint favorite attibute */
    if(media->isFavorite)
      painter->drawPixmap(0, 60, QPixmap(":/images/favorites-18x18.png"));
}



/*
********************************************************************************
*                                                                              *
*    ArtistGraphicItem_v3                                                      *
*      -> artist rating                                                        *
********************************************************************************
*/
ArtistGraphicItem_v3::ArtistGraphicItem_v3()
{
    opt.rect = boundingRect().toRect();
    hover_rating_ = -1.0;
}

QRectF ArtistGraphicItem_v3::boundingRect() const
{
    return QRectF(0, 0, m_coverSize*1.25, m_coverSize + opt.fontMetrics.height()*2 + 4 +  /*rating*/25 );

}

void ArtistGraphicItem_v3::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)

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
    
    /* draw artist image 200x200 */
    QPixmap pixTemp( opt.rect.size() );
    {
      pixTemp.fill(Qt::transparent);
      QPainter p;
      p.begin(&pixTemp);

      //! paint cover art
      QPixmap pix = CoverCache::instance()->image(media, albums_covers);
      p.drawPixmap((opt.rect.size().width()-m_coverSize)/2,21, pix);
      p.end();
    }

    /* manual graphics shadow (Qgraphics Effect cause refresh pb) */
    drawShadow( painter, QPointF(0,0), pixTemp, boundingRect());

    /* paint artist rating */
    const float rating_ = media->rating;
    RatingPainter::instance()->Paint(painter, QRect(0, 1, m_coverSize*1.25, 22), hover_rating_ == -1.0 ? rating_ : hover_rating_, media->isUserRating);

    /* paint artist name */
    painter->setFont(opt.font);
    painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));

    const QString elided_artist = opt.fontMetrics.elidedText ( media->name, Qt::ElideRight, m_coverSize*1.25);
    painter->drawText(QRect(0, m_coverSize+22, m_coverSize*1.25, 25), Qt::AlignTop | Qt::AlignHCenter, elided_artist);

    /* paint favorite attibute */
    if(media->isFavorite)
      painter->drawPixmap(0, 60, QPixmap(":/images/favorites-18x18.png"));
}

void ArtistGraphicItem_v3::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if(event->pos().toPoint().y() <= 22)
    {
      hover_rating_ = RatingPainter::RatingForPos(event->pos().toPoint(), QRect(0, 1, m_coverSize*1.25, 22));
      this->update();

      // update all track hover rating if selected
      QGraphicsScene* scene = this->scene();
      if(scene->selectedItems().contains(this)) {
        foreach(QGraphicsItem* gi, scene->selectedItems()) {
          ArtistGraphicItem_v3 *item = static_cast<ArtistGraphicItem_v3*>(gi);
          item->setHoverRating(hover_rating_);
          item->update();
        }
      }
    }
}

void ArtistGraphicItem_v3::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->pos().toPoint().y() <= 22)
    {
      media->rating       = RatingPainter::RatingForPos(event->pos().toPoint(), QRect(0, 1, m_coverSize*1.25, 22));
      media->isUserRating = true;

      // update all track hover rating if selected
      QGraphicsScene* scene = this->scene();
      if(scene->selectedItems().contains(this)) {
        foreach(QGraphicsItem* gi, scene->selectedItems()) {
          ArtistGraphicItem_v3 *item = static_cast<ArtistGraphicItem_v3*>(gi);
          item->media->rating = media->rating;
          item->media->isUserRating = true;
        }
      }

      QVariant v;
      v.setValue(static_cast<QGraphicsItem*>(this));

      (ACTIONS()->value(BROWSER_ITEM_RATING_CLICK))->setData(v);
      (ACTIONS()->value(BROWSER_ITEM_RATING_CLICK))->trigger();
    }
}

void ArtistGraphicItem_v3::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //Debug::debug() << " ---- ArtistGraphicItem_v3::hoverLeaveEvent";
    hover_rating_ = -1.0;

    // update all track hover rating if selected
    QGraphicsScene* scene = this->scene();
    if(scene->selectedItems().contains(this)) {
      foreach(QGraphicsItem* gi, scene->selectedItems()) {
        ArtistGraphicItem_v3 *item = static_cast<ArtistGraphicItem_v3*>(gi);
        item->setHoverRating(-1.0);
        item->update();
      }
    }

    ArtistGraphicItem::hoverLeaveEvent(event);
}

/*
********************************************************************************
*                                                                              *
*    Class TrackGraphicItem                                                    *
*      -> used in PLAYLIST VIEW                                                *
*      -> draw only number/track name                                          *
********************************************************************************
*/
TrackGraphicItem::TrackGraphicItem()
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
    opt.decorationPosition = QStyleOptionViewItem::Left;
    opt.decorationAlignment = Qt::AlignCenter;
    opt.displayAlignment = Qt::AlignLeft|Qt::AlignVCenter;

    opt.locale.setNumberOptions(QLocale::OmitGroupSeparator);
    opt.state |= QStyle::State_Active;
    opt.state |= QStyle::State_Enabled;

    _width = 510;
}

QRectF TrackGraphicItem::boundingRect() const
{
    if(_width < 510)
      return QRectF(0, 0, 510, 22);

    return QRectF(0, 0, _width, 22);
}

void TrackGraphicItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
   //! get data
   bool isTrack       = (media->type() == TYPE_TRACK) ? true : false;

   const int width       = _width < 530 ? 530 : _width;
   const int title_width = _width < 530 ? 150 : (_width - 140)/3;

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
   opt.rect = boundingRect().toRect().adjusted(0,0,-10,0);
   UTIL::getStyle()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);


   //! TRACK in collection
   if(isTrack && media->id != -1)
   {
        painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));
        painter->setFont(opt.font);

        //! paint artist name
        const QString artist_elided = opt.fontMetrics.elidedText ( media->artist, Qt::ElideRight, title_width);
        painter->drawText(QRect(30, 0, title_width, 22), Qt::AlignLeft | Qt::AlignVCenter,artist_elided);

        //! paint album name
        const QString album_elided = opt.fontMetrics.elidedText ( media->album, Qt::ElideRight, title_width);
        painter->drawText(QRect(60+title_width, 0, title_width, 22), Qt::AlignLeft | Qt::AlignVCenter,album_elided);

        //! paint track name
        const QString title_elided = opt.fontMetrics.elidedText ( media->title, Qt::ElideRight, title_width);
        painter->drawText(QRect(60+title_width*2, 0, title_width, 22), Qt::AlignLeft | Qt::AlignVCenter,title_elided);

        //! paint track duration
        const QString duree_elided = opt.fontMetrics.elidedText ( media->durationToString(), Qt::ElideRight, 50);
        painter->drawText(QRect(60+title_width*3, 0, 50, 22), Qt::AlignRight | Qt::AlignVCenter,duree_elided);
   }
   else 
   {
        painter->setPen( opt.palette.color ( isSelected() ? QPalette::Normal : QPalette::Disabled, isSelected() ? QPalette::HighlightedText : QPalette::WindowText) );
        painter->setFont( opt.font );

        if(isTrack) {
          const QString name_elided = opt.fontMetrics.elidedText ( media->url, Qt::ElideRight, width -20);
          painter->drawText(QRect(30, 0,  width -20, 22), Qt::AlignLeft | Qt::AlignVCenter, name_elided);
        }
        else
        {
          QString info = !media->extra["station"].toString().isEmpty() ? media->extra["station"].toString() : media->url;
          
          const QString name_elided = opt.fontMetrics.elidedText ( info, Qt::ElideRight, width -20);
          painter->drawText(QRect(30, 0,  width -20, 22), Qt::AlignLeft | Qt::AlignVCenter, name_elided);
        }
   }

   //! paint activated item
   if(media->isPlaying)
      UTIL::drawPlayingIcon(painter,18, 0, QPoint(12,1));
   else if(media->isBroken)
     painter->drawPixmap(2, 1, QPixmap(":/images/media-broken-18x18.png"));
   else if (!isTrack)
     painter->drawPixmap(1, 2, QPixmap(":/images/media-url-18x18.png"));
}


void TrackGraphicItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- TrackGraphicItem::hoverEnterEvent";
    opt.state |= QStyle::State_MouseOver;
    this->update();
}

void TrackGraphicItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- TrackGraphicItem::hoverLeaveEvent";
    opt.state &= ~QStyle::State_MouseOver;
    this->update();
}

void TrackGraphicItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void TrackGraphicItem::startDrag(QWidget *w)
{
    //Debug::debug() << " ---- TrackGraphicItem::mouseMoveEvent";
    setCursor(Qt::OpenHandCursor);

    MediaMimeData* mimedata = new MediaMimeData(SOURCE_COLLECTION);
    mimedata->addTrack(this->media);

    QDrag *drag = new QDrag(w);
    drag->setMimeData(mimedata);
    drag->exec();
}

void TrackGraphicItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
    QVariant v;
    v.setValue(static_cast<QGraphicsItem*>(this));

    (ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE))->setData(v);
    (ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE))->trigger();
}

/*
********************************************************************************
*                                                                              *
*    Class TrackGraphicItem_v2                                                 *
*      -> used in TRACKS VIEW                                                  *
*      -> draw number/track name/album/duration/rating                         *
*                                                                              *
********************************************************************************
*/
TrackGraphicItem_v2::TrackGraphicItem_v2() : TrackGraphicItem()
{
    _width        = 530;
    hover_rating_ = -1.0;
}

QRectF TrackGraphicItem_v2::boundingRect() const
{
    if(_width < 530)
      return QRectF(0, 0, 530, 22);

    return QRectF(0, 0, _width, 22);
}

/* TrackGraphicItem_v2 is used in VIEW_TRACKS :  */
/*  assume only media is TYPE_TRACK              */
void TrackGraphicItem_v2::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
   //! get data
   const int title_width = _width < 530 ? 150 : (_width - 210)/2;

   if(media->type() != TYPE_TRACK) {
     TrackGraphicItem::paint(painter, option, widget);
     return;
   }

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
   //QStyle *style = widget ? widget->style() : QApplication::style();
   opt.rect = boundingRect().toRect().adjusted(0,0,-10,0);
   UTIL::getStyle()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

   //! paint track number
   painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));
   painter->setFont(opt.font);
   painter->drawText(QRect(30, 0, 30, 22), Qt::AlignLeft | Qt::AlignVCenter, QString::number(media->num));

   //! paint track name
   const QString title_elided = opt.fontMetrics.elidedText ( media->title, Qt::ElideRight, title_width);
   painter->drawText(QRect(60, 0, title_width, 22), Qt::AlignLeft | Qt::AlignVCenter,title_elided);

   //! paint album name
   const QString album_elided = opt.fontMetrics.elidedText ( media->album, Qt::ElideRight, title_width);
   painter->drawText(QRect(60+title_width, 0, title_width, 22), Qt::AlignLeft | Qt::AlignVCenter,album_elided);

   //! paint track duration
   const QString duree_elided = opt.fontMetrics.elidedText ( media->durationToString(), Qt::ElideRight, 50);
   painter->drawText(QRect(60+title_width*2, 0, 50, 22), Qt::AlignRight | Qt::AlignVCenter,duree_elided);


   //! paint activated item
   if(media->isPlaying)
      UTIL::drawPlayingIcon(painter,18, 0, QPoint(12,1));
   else if(media->isBroken)
     painter->drawPixmap(0, 0, QPixmap(":/images/media-broken-18x18.png"));

   //! paint track rating
   const float rating_ = media->rating;
   RatingPainter::instance()->Paint(painter, QRect(title_width*2 + 115, 0, 80, 22), hover_rating_ == -1.0 ? rating_ : hover_rating_, true);
}

void TrackGraphicItem_v2::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    const int title_width = _width < 530 ? 150 : (_width - 210)/2;
    if(event->pos().toPoint().x() >= title_width*2 + 115)
    {
      hover_rating_ = RatingPainter::RatingForPos(event->pos().toPoint(), QRect(title_width*2 + 115, 0, 80, 22));

      this->update();

      // update all track hover rating if selected
      QGraphicsScene* scene = this->scene();
      if(scene->selectedItems().contains(this)) {
        foreach(QGraphicsItem* gi, scene->selectedItems()) {
          TrackGraphicItem_v2 *item = static_cast<TrackGraphicItem_v2*>(gi);
          item->setHoverRating(hover_rating_);
          item->update();
        }
      }
    }
}

void TrackGraphicItem_v2::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    const int title_width = _width < 530 ? 150 : (_width - 210)/2;

    if(event->pos().toPoint().x() >= title_width*2 + 115)
    {
      media->rating = RatingPainter::RatingForPos(event->pos().toPoint(), QRect(title_width*2 + 115, 0, 80, 22));

      // update all track hover rating if selected
      QGraphicsScene* scene = this->scene();
      if(scene->selectedItems().contains(this)) {
        foreach(QGraphicsItem* gi, scene->selectedItems()) {
          TrackGraphicItem_v2 *item = static_cast<TrackGraphicItem_v2*>(gi);
          item->media->rating = media->rating;
        }
      }

      QVariant v;
      v.setValue(static_cast<QGraphicsItem*>(this));

      (ACTIONS()->value(BROWSER_ITEM_RATING_CLICK))->setData(v);
      (ACTIONS()->value(BROWSER_ITEM_RATING_CLICK))->trigger();
    }
}

void TrackGraphicItem_v2::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //Debug::debug() << " ---- TrackGraphicItem_v2::hoverLeaveEvent";
    hover_rating_ = -1.0;

    // update all track hover rating if selected
    QGraphicsScene* scene = this->scene();
    if(scene->selectedItems().contains(this)) {
      foreach(QGraphicsItem* gi, scene->selectedItems()) {
        TrackGraphicItem_v2 *item = static_cast<TrackGraphicItem_v2*>(gi);
        item->setHoverRating(-1.0);
        item->update();
      }
    }

    TrackGraphicItem::hoverLeaveEvent(event);
}
/*
********************************************************************************
*                                                                              *
*    Class TrackGraphicItem_v3                                                 *
*      -> used in HISTORY VIEW                                                 *
*      -> only draw track or stream name                                       *
*      -> add different color depending on sql id                              *
********************************************************************************
*/
TrackGraphicItem_v3::TrackGraphicItem_v3()
{
    _width        = 530;
    hover_rating_ = -1.0;
}

QRectF TrackGraphicItem_v3::boundingRect() const
{
    if(_width < 510)
      return QRectF(0, 0, 510, 22);

    return QRectF(0, 0, _width, 22);
}

void TrackGraphicItem_v3::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
   //! get data
   bool isTrack       = (media->type() == TYPE_TRACK) ? true : false;

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
     RatingPainter::instance()->Paint(painter, QRect(title_width*3-50 + 115, 0, 80, 22), hover_rating_ == -1.0 ? rating_ : hover_rating_, true);
   }
   else 
   {
     painter->setPen( opt.palette.color ( isSelected() ? QPalette::Normal : QPalette::Disabled, isSelected() ? QPalette::HighlightedText : QPalette::WindowText) );
     painter->setFont(opt.font);

     if(isTrack) 
     {
         const QString name_elided = opt.fontMetrics.elidedText ( media->url, Qt::ElideRight, width -20);
         painter->drawText(QRect(30, 0,  width -20, 22), Qt::AlignLeft | Qt::AlignVCenter, name_elided);
     }
     else
     {
         QString info = !media->extra["station"].toString().isEmpty() ? media->extra["station"].toString() : media->url;
          
         const QString name_elided = opt.fontMetrics.elidedText ( info, Qt::ElideRight, width -20);
         painter->drawText(QRect(30, 0,  width -20, 22), Qt::AlignLeft | Qt::AlignVCenter, name_elided);
     }
   }

   //! paint activated item
   if(media->isPlaying)
      UTIL::drawPlayingIcon(painter,18, 0, QPoint(12,1));
   else if(media->isBroken)
     painter->drawPixmap(1, 1, QPixmap(":/images/media-broken-18x18.png"));
   else if (!isTrack)
     painter->drawPixmap(1, 2, QPixmap(":/images/media-url-18x18.png"));
}


void TrackGraphicItem_v3::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    const int title_width = _width < 530 ? 150 : (_width - 150)/3;
    if( event->pos().toPoint().x() >= (title_width*3-50 + 115) )
    {
      hover_rating_ = RatingPainter::RatingForPos(event->pos().toPoint(), QRect(title_width*3-50 + 115, 0, 80, 22));
      this->update();
    }
}

void TrackGraphicItem_v3::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    const int title_width = _width < 530 ? 150 : (_width - 150)/3;

    if( event->pos().toPoint().x() >= (title_width*3-50 + 115) )
    {
      media->rating = RatingPainter::RatingForPos(event->pos().toPoint(), QRect(title_width*3-50 + 115, 0, 80, 22));

      QVariant v;
      v.setValue(static_cast<QGraphicsItem*>(this));

      (ACTIONS()->value(BROWSER_ITEM_RATING_CLICK))->setData(v);
      (ACTIONS()->value(BROWSER_ITEM_RATING_CLICK))->trigger();
    }
}

void TrackGraphicItem_v3::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //Debug::debug() << " ---- TrackGraphicItem_v3::hoverLeaveEvent";
    hover_rating_ = -1.0;
    TrackGraphicItem::hoverLeaveEvent(event);
}


/*
********************************************************************************
*                                                                              *
*    Class TrackGraphicItem_v5                                                 *
*      -> used in TRACKS VIEW popup menu                                       *
*      -> draw number/track name/duration/rating                               *
*                                                                              *
********************************************************************************
*/
TrackGraphicItem_v5::TrackGraphicItem_v5() : TrackGraphicItem()
{
    _width        = 530;
    hover_rating_ = -1.0;
}

QRectF TrackGraphicItem_v5::boundingRect() const
{
    if(_width < 530)
      return QRectF(0, 0, 530, 22);

    return QRectF(0, 0, _width, 22);
}

/* TrackGraphicItem_v5 is used in popup menu   */
/*  assume only media is TYPE_TRACK            */
void TrackGraphicItem_v5::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
   //! get data
   const int title_width = _width < 530 ? 120 : (_width - 210);

   if(media->type() != TYPE_TRACK) {
     TrackGraphicItem::paint(painter, option, widget);
     return;
   }

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
   //QStyle *style = widget ? widget->style() : QApplication::style();
   opt.rect = boundingRect().toRect().adjusted(0,0,-10,0);
   UTIL::getStyle()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

   //! paint track number
   painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));
   painter->setFont(opt.font);
   painter->drawText(QRect(30, 0, 30, 22), Qt::AlignLeft | Qt::AlignVCenter, QString::number(media->num));

   //! paint track name
   const QString title_elided = opt.fontMetrics.elidedText ( media->title, Qt::ElideRight, title_width);
   painter->drawText(QRect(60, 0, title_width, 22), Qt::AlignLeft | Qt::AlignVCenter,title_elided);

   //! paint track duration
   const QString duree_elided = opt.fontMetrics.elidedText ( media->durationToString(), Qt::ElideRight, 50);
   painter->drawText(QRect(60+title_width, 0, 50, 22), Qt::AlignRight | Qt::AlignVCenter,duree_elided);


   //! paint activated item
   if(media->isPlaying)
      UTIL::drawPlayingIcon(painter,18, 0, QPoint(12,1));
   else if(media->isBroken)
      painter->drawPixmap(0, 0, QPixmap(":/images/media-broken-18x18.png"));

   //! paint track rating
   const float rating_ = media->rating;
   RatingPainter::instance()->Paint(painter, QRect(title_width + 115, 0, 80, 22), hover_rating_ == -1.0 ? rating_ : hover_rating_, true);
}

void TrackGraphicItem_v5::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    const int title_width = _width < 530 ? 120 : (_width - 210);
    if(event->pos().toPoint().x() >= title_width + 115)
    {
      hover_rating_ = RatingPainter::RatingForPos(event->pos().toPoint(), QRect(title_width + 115, 0, 80, 22));

      this->update();

      // update all track hover rating if selected
      QGraphicsScene* scene = this->scene();
      if(scene->selectedItems().contains(this)) {
        foreach(QGraphicsItem* gi, scene->selectedItems()) {
          TrackGraphicItem_v5 *item = static_cast<TrackGraphicItem_v5*>(gi);
          item->setHoverRating(hover_rating_);
          item->update();
        }
      }
    }
}

void TrackGraphicItem_v5::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    const int title_width = _width < 530 ? 120 : (_width - 210);

    if(event->pos().toPoint().x() >= title_width + 115)
    {
      media->rating = RatingPainter::RatingForPos(event->pos().toPoint(), QRect(title_width + 115, 0, 80, 22));

      // update all track hover rating if selected
      QGraphicsScene* scene = this->scene();
      if(scene->selectedItems().contains(this)) {
        foreach(QGraphicsItem* gi, scene->selectedItems()) {
          TrackGraphicItem_v5 *item = static_cast<TrackGraphicItem_v5*>(gi);
          item->media->rating = media->rating;
        }
      }

      QVariant v;
      v.setValue(static_cast<QGraphicsItem*>(this));

      (ACTIONS()->value(BROWSER_ITEM_RATING_CLICK))->setData(v);
      (ACTIONS()->value(BROWSER_ITEM_RATING_CLICK))->trigger();
    }
}

void TrackGraphicItem_v5::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //Debug::debug() << " ---- TrackGraphicItem_v5::hoverLeaveEvent";
    hover_rating_ = -1.0;

    // update all track hover rating if selected
    QGraphicsScene* scene = this->scene();
    if(scene->selectedItems().contains(this)) {
      foreach(QGraphicsItem* gi, scene->selectedItems()) {
        TrackGraphicItem_v5 *item = static_cast<TrackGraphicItem_v5*>(gi);
        item->setHoverRating(-1.0);
        item->update();
      }
    }

    TrackGraphicItem::hoverLeaveEvent(event);
}


/*
********************************************************************************
*                                                                              *
*    PlaylistGraphicItem                                                       *
*                                                                              *
********************************************************************************
*/
PlaylistGraphicItem::PlaylistGraphicItem()
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
   opt.decorationPosition = QStyleOptionViewItem::Top;
   opt.displayAlignment = Qt::AlignCenter;

   opt.locale.setNumberOptions(QLocale::OmitGroupSeparator);
   opt.state &= ~ QStyle::State_Active;
   opt.state |= QStyle::State_Enabled;
   opt.state &= ~QStyle::State_Selected;
   
   opt.palette.setColor(QPalette::Active, QPalette::Highlight, SETTINGS()->_baseColor);
   opt.palette.setColor(QPalette::Inactive, QPalette::Highlight, QApplication::palette().color(QPalette::Normal,QPalette::Highlight));
}

QRectF PlaylistGraphicItem::boundingRect() const
{
    return QRectF(0, 0, 170, 170);
}


void PlaylistGraphicItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
   //! get data
   const QString name    = media->name;
   const QString icon    = media->icon;

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

   
   //! Draw frame for State_HasFocus item
   opt.rect = boundingRect().toRect().adjusted(0,0,0,0);
   UTIL::getStyle()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

   painter->setPen(opt.palette.color ( QPalette::Normal, isSelected() ? QPalette::HighlightedText : QPalette::WindowText));
   painter->setFont(opt.font);

   //! paint media playlist mime
   painter->drawPixmap(opt.rect.adjusted(10,6,-10,-10),QPixmap(icon));

   //! paint playlist date
   if( media->p_type == T_FILE || media->p_type== T_DATABASE)
   {
      QDateTime date        =  QDateTime::fromTime_t(media->date);
      painter->drawText(boundingRect().toRect().adjusted(0, 0, 0, -150), Qt::AlignCenter, date.toString("dd.MM.yyyy"));
   }

   //! paint playlist name
   const QString elided_p_name = opt.fontMetrics.elidedText ( name, Qt::ElideMiddle, 260, Qt::TextWrapAnywhere);

   painter->drawText(QRect(0, 140, 170, 30), Qt::AlignTop | Qt::AlignCenter | Qt::TextWrapAnywhere, elided_p_name);

   /* paint favorite attibute */
   if(media->isFavorite)
     painter->drawPixmap(10, 60, QPixmap(":/images/favorites-18x18.png"));

   //! paint media playlist type (internal/user)
   if(media->p_type == T_FILE)
     painter->drawPixmap(10, 90, QPixmap(":/images/files-18x18.png"));
}

void PlaylistGraphicItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- AlbumGraphicItem::hoverEnterEvent";
    opt.state |= QStyle::State_MouseOver;
    this->update();
}

void PlaylistGraphicItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    //Debug::debug() << " ---- AlbumGraphicItem::hoverLeaveEvent";
    opt.state &= ~QStyle::State_MouseOver;
    this->update();
}

void PlaylistGraphicItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void PlaylistGraphicItem::startDrag(QWidget *w)
{
    setCursor(Qt::OpenHandCursor);

    MediaMimeData* mimedata = new MediaMimeData(SOURCE_COLLECTION);

    //! rechercher dynamique pour les smart playlists
    if(media->p_type == T_SMART)
    {
      mimedata->addTracks( SmartPlaylist::mediaItem( media->rules ) );
    }
    //! playliste normale -> lecture dans le modele
    else
    {
       mimedata->setParent( media );
       mimedata->addTracks (
         LocalPlaylistModel::instance()->getItemChildrenTracks(media)
       );
    }

    QDrag *drag = new QDrag(w);
    drag->setMimeData(mimedata);

    //! set pixmap
    drag->setPixmap( QPixmap(media->icon) );
    drag->exec();
}

void PlaylistGraphicItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
    QVariant v;
    v.setValue(static_cast<QGraphicsItem*>(this));

    (ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE))->setData(v);
    (ACTIONS()->value(BROWSER_LOCAL_ITEM_MOUSE_MOVE))->trigger();
}


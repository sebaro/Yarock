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


#include "context_item.h"

#include "covers/covercache.h"
#include "core/mediaitem/mediaitem.h"
#include "core/player/engine.h"
#include "settings.h"
#include "debug.h"

#include <QtCore>
#include <QDesktopServices>
#include <QTextDocument>
#include <QApplication>



/*
********************************************************************************
*                                                                              *
*    Class WebLinkItem                                                         *
*       -> use with    webLinkItem->setHtml("<a href='text'>http://url</a>");  *
*                                                                              *
********************************************************************************
*/
WebLinkItem::WebLinkItem()
{
#if QT_VERSION < 0x050000
    setAcceptsHoverEvents(true);
#else
    setAcceptHoverEvents(true);
#endif    
    setAcceptDrops(false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsFocusable, false);
    setCursor(Qt::PointingHandCursor);

    setGraphicsItem(this);

    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
}

void WebLinkItem::clear()
{
    m_link.clear();
    m_text.clear();
}

void WebLinkItem::setText(const QString& text)
{
    m_text = text;
}
void WebLinkItem::setLink(const QString& link)
{
    m_link = link;
}

QRectF WebLinkItem::boundingRect() const
{
    if(m_text.isEmpty())
      return QRectF(0,0,0,0);
    
    QFont font;
    font.setPointSize( 8 );

    QFontMetrics fm( font );
    
    return QRectF(0, 0, fm.width(m_text) + 12, fm.height() + 4);
}

// Inherited from QGraphicsLayoutItem
void WebLinkItem::setGeometry(const QRectF &geom)
{
    setPos(geom.topLeft());
}

// Inherited from QGraphicsLayoutItem
QSizeF WebLinkItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which);
Q_UNUSED(constraint);
    return boundingRect().size();
}

void WebLinkItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
Q_UNUSED(widget)
    QColor brush_color = QColor( 63, 28, 83 );

    QRect rect = boundingRect().toRect();
    painter->setPen(QPen( brush_color, 0.1, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(QBrush( brush_color ,Qt::SolidPattern));
    painter->drawRoundedRect(rect, 4.0, 4.0);

    //! draw text
    QFont font = QApplication::font();
    font.setPointSize( 8 );
    painter->setFont(font);

    painter->setPen(QColor(Qt::white));
    painter->drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, m_text);
}

void WebLinkItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
Q_UNUSED(event)
    if(!m_link.isEmpty())
      QDesktopServices::openUrl(QUrl(m_link));
}



/*
********************************************************************************
*                                                                              *
*    Class TextGraphicItem                                                     *
*                                                                              *
********************************************************************************
*/
TextGraphicItem::TextGraphicItem()
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

    setGraphicsItem(this);
    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );

    
    setFont( QApplication::font() );
    
    setDefaultTextColor(QApplication::palette().color(QPalette::Normal, QPalette::WindowText));
    setTextInteractionFlags(Qt::TextBrowserInteraction);
    setOpenExternalLinks(false);

    setFlag(QGraphicsItem::ItemIsFocusable, false);
    
    this->document()->setDefaultStyleSheet("p { margin: 4; }");
}

void TextGraphicItem::clear()
{
    this->document()->clear();
    updateItem();
}

void TextGraphicItem::updateItem()
{
     prepareGeometryChange();

     updateGeometry();

     update();
}

QRectF TextGraphicItem::boundingRect() const
{
  
  return QRectF(0, 0, document()->size().width(), document()->size().height());
}


// Inherited from QGraphicsLayoutItem
void TextGraphicItem::setGeometry(const QRectF &geom)
{
    QGraphicsLayoutItem::setGeometry(geom);
    setPos(geom.topLeft());
}

// Inherited from QGraphicsLayoutItem
QSizeF TextGraphicItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which)
Q_UNUSED(constraint)

    return QSizeF( boundingRect().size());
}

void TextGraphicItem::contextMenuEvent ( QGraphicsSceneContextMenuEvent * event )
{
  event->ignore();
}


/*
********************************************************************************
*                                                                              *
*    Class AlbumThumbGraphicItem                                               *
*                                                                              *
********************************************************************************
*/
AlbumThumbGraphicItem::AlbumThumbGraphicItem()
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

    setGraphicsItem(this);

    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

    m_pix = QPixmap(":/images/default-cover-120x120.png");
}


void AlbumThumbGraphicItem::clear()
{
    m_title.clear();
    m_year.clear();

    m_pix = QPixmap(":/images/default-cover-120x120.png");
}

void AlbumThumbGraphicItem::updateItem()
{
    update();
}

QRectF AlbumThumbGraphicItem::boundingRect() const
{
    return QRectF(0, 0, 130, 130);
}

// Inherited from QGraphicsLayoutItem
void AlbumThumbGraphicItem::setGeometry(const QRectF &geom)
{
    QGraphicsLayoutItem::setGeometry(geom);
    setPos(geom.topLeft());
}

// Inherited from QGraphicsLayoutItem
QSizeF AlbumThumbGraphicItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which)
Q_UNUSED(constraint)
    return QSizeF( boundingRect().size());
}

void AlbumThumbGraphicItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * )
{
Q_UNUSED(option)
    QPixmap pixTemp(QSize(130,130));
    {
      pixTemp.fill(Qt::transparent);
      QPainter p;
      p.begin(&pixTemp);

      p.drawPixmap(13,9, m_pix);
      p.end();
    }

   painter->drawPixmap(0, 0, pixTemp);

   //! paint album title
   painter->setFont(QFont("Arial", 8, QFont::Normal));
   painter->setPen(QApplication::palette().color(QPalette::Normal, QPalette::WindowText));

   QFontMetrics m1( painter->font());
   const QString elided_album = m1.elidedText ( m_title, Qt::ElideRight, 128);
   painter->drawText(QRect (0,120,128,20), Qt::AlignVCenter | Qt::AlignHCenter, elided_album );

   //! paint year
   painter->setPen(QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
   painter->drawText(QRect(0, 0, 130, 10), Qt::AlignTop | Qt::AlignCenter, m_year);
}



/*
********************************************************************************
*                                                                              *
*    Class ArtistThumbGraphicItem                                              *
*                                                                              *
********************************************************************************
*/
ArtistThumbGraphicItem::ArtistThumbGraphicItem()
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

    setGraphicsItem(this);

    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

    m_pix = QPixmap(":/images/view-artist.png");
}


void ArtistThumbGraphicItem::clear()
{
    m_name.clear();
    m_pix = QPixmap(":/images/view-artist.png");
}

void ArtistThumbGraphicItem::updateItem()
{
    update();
}

QRectF ArtistThumbGraphicItem::boundingRect() const
{
    return QRectF(0, 0, 95, 95);
}

// Inherited from QGraphicsLayoutItem
void ArtistThumbGraphicItem::setGeometry(const QRectF &geom)
{
    QGraphicsLayoutItem::setGeometry(geom);
    setPos(geom.topLeft());
}

// Inherited from QGraphicsLayoutItem
QSizeF ArtistThumbGraphicItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which)
Q_UNUSED(constraint)
    return QSizeF( boundingRect().size());
}

void ArtistThumbGraphicItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * )
{
Q_UNUSED(option)
    QSize size = m_pix.size();
    
    int offset_h = qMax(12, (95 - size.height())/2);
    
    QPixmap pixTemp(QSize(95,95));
    {
      pixTemp.fill(Qt::transparent);
      QPainter p;
      p.begin(&pixTemp);

      p.drawPixmap((95 - size.width())/2,offset_h, m_pix);
      p.end();
    }

   painter->drawPixmap(0, 0, pixTemp);

   /* paint artist name */
   painter->setFont(QFont("Arial", 8, QFont::Normal));
   painter->setPen(QApplication::palette().color(QPalette::Normal, QPalette::WindowText));
   
   QFontMetrics m1( painter->font());
   const QString elided_artist = m1.elidedText ( m_name, Qt::ElideRight, 92);
   
   painter->drawText(QRect(0, 0, 95, 10), Qt::AlignTop | Qt::AlignCenter, elided_artist);
}

/*
********************************************************************************
*                                                                              *
*    Class NowPlayingGraphicItem                                               *
*                                                                              *
********************************************************************************
*/
NowPlayingGraphicItem::NowPlayingGraphicItem()
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

    setGraphicsItem(this);

    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
}

void NowPlayingGraphicItem::updateItem()
{
    update();
}

QRectF NowPlayingGraphicItem::boundingRect() const
{
    return QRectF(0, 0, 500, 180);
}

// Inherited from QGraphicsLayoutItem
void NowPlayingGraphicItem::setGeometry(const QRectF &geom)
{
    QGraphicsLayoutItem::setGeometry(geom);
    setPos(geom.topLeft());
}

// Inherited from QGraphicsLayoutItem
QSizeF NowPlayingGraphicItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which)
Q_UNUSED(constraint)
    return QSizeF(boundingRect().size());
}

void NowPlayingGraphicItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * )
{
Q_UNUSED(option)

  const int width = 500;

  //! Get data
  MEDIA::TrackPtr track = Engine::instance()->playingTrack();
  QPixmap         pix   = CoverCache::instance()->cover(track);

  //! font
  QFont font_normal = QApplication::font();
  font_normal.setBold(false);
  font_normal.setPointSize(font_normal.pointSize()*1.1);

  QFont font_bold   = QApplication::font();
  font_bold.setBold(true);
  font_bold.setPointSize(font_bold.pointSize()*1.1);

  //! -- Playing State
  if(Engine::instance()->state() != ENGINE::STOPPED && Engine::instance()->playingTrack())
  {
    if(track->type() == TYPE_TRACK)
    {
        //! paint title
        painter->setPen(QApplication::palette().color(QPalette::Normal, QPalette::WindowText));
        painter->setFont(font_bold);
        QFontMetrics fm(font_bold);
        const QString titleTruncated = fm.elidedText ( track->title, Qt::ElideRight, width-135 );
        painter->drawText(QRect(130, 15, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,titleTruncated );

        //! paint album
        painter->setPen(QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
        painter->setFont(font_normal);
        QFontMetrics fm2(font_normal);
        const QString albumTruncated = fm2.elidedText ( track->album, Qt::ElideRight, width-135 );
        painter->drawText(QRect(130, 35, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,albumTruncated );

        //! paint artist
        const QString artistTruncated = fm2.elidedText ( track->artist, Qt::ElideRight, width-135 );
        painter->drawText(QRect(130, 55, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,artistTruncated );

        //! paint duration
        if(track->duration > 0) {
          const QString durationText = track->durationToString();
          const QString dureeTruncated = fm2.elidedText ( durationText, Qt::ElideRight, width-135 );
          painter->drawText(QRect(130, 75, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,dureeTruncated );
        }

/*        //! paint track rating
        ui_rating_widget->set_rating(media->data.rating);
        ui_rating_widget->show();
*/
        //! paint pixmap (coverart)
        painter->drawPixmap(10,10,110,110, pix);
    }
    else if(track->type() == TYPE_STREAM)
    {
        //! paint stream name
        painter->setPen(QApplication::palette().color(QPalette::Normal, QPalette::WindowText));
        painter->setFont(font_bold);
        QFontMetrics fm(font_bold);
        const QString streamTruncated = fm.elidedText ( track->name, Qt::ElideRight, width-135 );
        painter->drawText(QRect(130, 15, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,streamTruncated );

        //! paint title
        painter->setPen(QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
        painter->setFont(font_normal);
        QFontMetrics fm2(font_normal);
        const QString title_or_url = track->title.isEmpty() ? track->url : track->title;

        const QString titleTruncated = fm2.elidedText ( title_or_url, Qt::ElideRight, width-135 );
        painter->drawText(QRect(130, 35, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,titleTruncated );

        //! paint album
        const QString albumTruncated = fm2.elidedText ( track->album, Qt::ElideRight, width-135 );
        painter->drawText(QRect(130, 55, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,albumTruncated );

        //! paint artist
        const QString artistTruncated = fm2.elidedText ( track->artist, Qt::ElideRight, width-135 );
        painter->drawText(QRect(130, 75, width-135, 20), Qt::AlignVCenter | Qt::AlignLeft,artistTruncated );

        //! paint pixmap (cover art)
        painter->drawPixmap(8,8,110,110, pix);
    }
  }
  else 
  {
     painter->setPen(QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));

     const QRect rect = boundingRect().toRect();

     painter->setFont(font_normal);
     painter->drawText(QRect(70,6,250,22), Qt::AlignVCenter, QObject::tr("Player is stopped"));
     
     /* draw pixmap with transparency */
     QPixmap p_in = QPixmap(":/images/info-48x48.png");

     QPixmap p_out(p_in.size());
     p_out.fill(Qt::transparent);
     QPainter p(&p_out);
     p.setOpacity(0.2);
     p.drawPixmap(0, 0, p_in);
     p.end();
     painter->drawPixmap(rect.adjusted(10,-4,0,0).topLeft() ,p_out );

  }
}

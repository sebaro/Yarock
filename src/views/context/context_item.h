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

#ifndef _CONTEXT_GRAPHICS_ITEM_H_
#define _CONTEXT_GRAPHICS_ITEM_H_

#include <QGraphicsItem>
#include <QGraphicsLayoutItem>
#include <QGraphicsSceneEvent>
#include <QStyleOptionButton>
#include <QPainter>


/*
********************************************************************************
*                                                                              *
*    Class WebLinkItem                                                         *
*                                                                              *
********************************************************************************
*/
class WebLinkItem : public QGraphicsItem, public QGraphicsLayoutItem
{
  public:
    WebLinkItem();
    void  clear();
    void  updateItem();
    void  setText(const QString& text);
    void  setLink(const QString& link);

    int height() {return boundingRect().toRect().height();}
    int width() {return boundingRect().toRect().width();}

  private:
    QString     m_text;
    QString     m_link;

  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    // Inherited from QGraphicsLayoutItem
    void setGeometry(const QRectF &geom);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
};

/*
********************************************************************************
*                                                                              *
*    Class TextGraphicItem                                                     *
*                                                                              *
********************************************************************************
*/
class TextGraphicItem : public QGraphicsTextItem, public QGraphicsLayoutItem
{
  public:
    TextGraphicItem();
    void updateItem();
    void clear();

    int height() {return boundingRect().toRect().height();}
    int width() {return boundingRect().toRect().width();}

  protected:
    void contextMenuEvent ( QGraphicsSceneContextMenuEvent * event );

    // Implement QGraphicsItem method
    QRectF boundingRect() const;

    // Inherited from QGraphicsLayoutItem
    void setGeometry(const QRectF &geom);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
};


/*
********************************************************************************
*                                                                              *
*    Class AlbumThumbGraphicItem                                               *
*                                                                              *
********************************************************************************
*/
class AlbumThumbGraphicItem : public QGraphicsItem, public QGraphicsLayoutItem
{
  public:
    AlbumThumbGraphicItem();
    void updateItem();
    void clear();

    QString      m_title;
    QString      m_year;
    QPixmap      m_pix;

  protected:
    // Inherited from QGraphicsLayoutItem
    void setGeometry(const QRectF &geom);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
};


/*
********************************************************************************
*                                                                              *
*    Class ArtistThumbGraphicItem                                              *
*                                                                              *
********************************************************************************
*/
class ArtistThumbGraphicItem : public QGraphicsItem, public QGraphicsLayoutItem
{
  public:
    ArtistThumbGraphicItem();
    void updateItem();
    void clear();

    QString      m_name;
    QPixmap      m_pix;

  protected:
    // Inherited from QGraphicsLayoutItem
    void setGeometry(const QRectF &geom);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
};


/*
********************************************************************************
*                                                                              *
*    Class NowPlayingGraphicItem                                               *
*                                                                              *
********************************************************************************
*/
class NowPlayingGraphicItem : public QGraphicsItem, public QGraphicsLayoutItem
{
  public:
    NowPlayingGraphicItem();
    void updateItem();

  protected:
    // Inherited from QGraphicsLayoutItem
    void setGeometry(const QRectF &geom);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
};


#endif // _CONTEXT_GRAPHICS_ITEM_H_

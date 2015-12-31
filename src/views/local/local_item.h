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

#ifndef _LOCAL_ITEM_H_
#define _LOCAL_ITEM_H_


#include "views/item_common.h"

#include "core/mediaitem/mediaitem.h"
#include "widgets/ratingwidget.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>


/*
********************************************************************************
*                                                                              *
*    Class AlbumGraphicItem                                                    *
*      -> with artist name/album name                                          *
********************************************************************************
*/
class AlbumGraphicItem : public QGraphicsItem
{
  public:
    AlbumGraphicItem();
    MEDIA::AlbumPtr             media;
    int type() const { return GraphicsItem::AlbumType; }

    void startDrag(QWidget*);

  protected:
    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  public:
     QStyleOptionViewItemV4 opt;
};

/*
********************************************************************************
*                                                                              *
*    Class AlbumGraphicItem_v2                                                 *
*      -> with album year/album name                                           *
********************************************************************************
*/
class AlbumGraphicItem_v2 : public AlbumGraphicItem
{
  public:
    AlbumGraphicItem_v2();

  protected:
    // Implement QGraphicsItem method
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
};


/*
********************************************************************************
*                                                                              *
*    Class AlbumGenreGraphicItem                                               *
*                                                                              *
********************************************************************************
*/
class AlbumGenreGraphicItem : public AlbumGraphicItem
{
  public:
    AlbumGenreGraphicItem();
    QString                 _genre;
    int type() const { return GraphicsItem::AlbumGenreType; }

    void startDrag(QWidget*);

  protected:
    // Implement QGraphicsItem method
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
};

/*
********************************************************************************
*                                                                              *
*    Class AlbumGraphicItem_v3                                                 *
*         -> (with playcount)                                                  *
********************************************************************************
*/
class AlbumGraphicItem_v3 : public AlbumGraphicItem
{
  public:
    AlbumGraphicItem_v3();

  protected:
    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
};

/*
********************************************************************************
*                                                                              *
*    Class AlbumGraphicItem_v4                                                 *
*         -> (with rating)                                                     *
********************************************************************************
*/
class AlbumGraphicItem_v4 : public AlbumGraphicItem
{
  public:
    AlbumGraphicItem_v4();
    void setHoverRating(float hr) {hover_rating_ = hr;}

  protected:
    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

  private:
    float hover_rating_;
};

/*
********************************************************************************
*                                                                              *
*    Class ArtistGraphicItem                                                   *
*                                                                              *
********************************************************************************
*/
class ArtistGraphicItem : public QGraphicsItem
{
  public:
    ArtistGraphicItem();
    MEDIA::ArtistPtr             media;
    int type() const { return GraphicsItem::ArtistType; }

    void startDrag(QWidget*);

    QList<MEDIA::AlbumPtr>       albums_covers;
    
  protected:
    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  public:
     QStyleOptionViewItemV4 opt;
};

/*
********************************************************************************
*                                                                              *
*    Class ArtistGraphicItem_v2                                                *
*         -> (with playcount)                                                  *
********************************************************************************
*/
class ArtistGraphicItem_v2 : public ArtistGraphicItem
{
  public:
    ArtistGraphicItem_v2();

  protected:
    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
};

/*
********************************************************************************
*                                                                              *
*    Class ArtistGraphicItem_v3                                                *
*         -> (with rating)                                                     *
********************************************************************************
*/
class ArtistGraphicItem_v3 : public ArtistGraphicItem
{
  public:
    ArtistGraphicItem_v3();
    void setHoverRating(float hr) {hover_rating_ = hr;}

  protected:
    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

  private:
    float hover_rating_;
};

/*
********************************************************************************
*                                                                              *
*    Class TrackGraphicItem                                                    *
*      -> used in PLAYLIST VIEW                                                *
*      -> draw only number/track name                                          *
********************************************************************************
*/
class TrackGraphicItem : public QGraphicsItem
{
  public:
    TrackGraphicItem();
    int                      _width;
    MEDIA::TrackPtr          media;
    int type() const { return GraphicsItem::TrackType; }

    void startDrag(QWidget*);

  protected:
    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

  public:
     QStyleOptionViewItemV4 opt;
};

/*
********************************************************************************
*                                                                              *
*    Class TrackGraphicItem_v2                                                 *
*      -> used in TRACKS VIEW                                                  *
*      -> draw number/track name/album/duration/rating                         *
*                                                                              *
********************************************************************************
*/
class TrackGraphicItem_v2  : public TrackGraphicItem
{
  public:
    TrackGraphicItem_v2();
    void setHoverRating(float hr) {hover_rating_ = hr;}

  protected:
    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

  private:
    float hover_rating_;
};

/*
********************************************************************************
*                                                                              *
*    Class TrackGraphicItem_v3                                                 *
*      -> used in HISTORY VIEW                                                 *
*      -> work for stream or track                                             *
*      -> draw track artist/album/title/rating or stream url name              *
********************************************************************************
*/
class TrackGraphicItem_v3 : public TrackGraphicItem
{
  public:
    TrackGraphicItem_v3();

  protected:
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

  private:
    float hover_rating_;
};


/*
********************************************************************************
*                                                                              *
*    Class PlaylistGraphicItem                                                 *
*                                                                              *
********************************************************************************
*/
class PlaylistGraphicItem : public QGraphicsItem
{
  public:
    PlaylistGraphicItem();
    MEDIA::PlaylistPtr           media;
    int type() const { return GraphicsItem::PlaylistType; }

    void startDrag(QWidget*);

  protected:
    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

  private:
     QStyleOptionViewItemV4 opt;
};




#endif // _LOCAL_ITEM_H_

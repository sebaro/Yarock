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

#ifndef _ITEM_COMMON_H_
#define _ITEM_COMMON_H_

#include <QGraphicsItem>
#include <QGraphicsLayoutItem>
#include <QTimer>
#include <QWidget>


namespace GraphicsItem {
  enum GraphicsType {
      AlbumType             = QGraphicsItem::UserType + 1,
      ArtistType            = QGraphicsItem::UserType + 2,
      AlbumGenreType        = QGraphicsItem::UserType + 3,
      TrackType             = QGraphicsItem::UserType + 4,
      PlaylistType          = QGraphicsItem::UserType + 5,
      StreamType            = QGraphicsItem::UserType + 6,
      FileSystemType        = QGraphicsItem::UserType + 7,
      CategoryType          = QGraphicsItem::UserType + 99
  };
}




/*
********************************************************************************
*                                                                              *
*    Class InfoGraphicItem                                                     *
*                                                                              *
********************************************************************************
*/
class InfoGraphicItem : public QGraphicsItem
{
  public:
    InfoGraphicItem(QWidget* parentView = 0);
    QString     _text;

  protected:
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
    QWidget    *m_parent;    
};


/*
********************************************************************************
*                                                                              *
*    Class InfoLayoutItem                                                      *
*                                                                              *
********************************************************************************
*/
class InfoLayoutItem : public InfoGraphicItem, public QGraphicsLayoutItem
{
  public:
    InfoLayoutItem(QWidget* parentView = 0);

  protected:
    // Inherited from QGraphicsLayoutItem
    void setGeometry(const QRectF &geom);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
};


/*
********************************************************************************
*                                                                              *
*    Class LoadingGraphicItem                                                  *
*                                                                              *
********************************************************************************
*/
class LoadingGraphicItem : public QGraphicsObject
{
Q_OBJECT  
  public:
    LoadingGraphicItem(QWidget* parentView = 0);
    QString     _text;

  protected:
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
    
  private slots:
    void slot_timeout();
    
  private:
    static const int constSpinnerSteps=64;
    
    QWidget    *m_parent;      
    int         m_value;
    QTimer     *m_timer;
};

/*
********************************************************************************
*                                                                              *
*    Class CategorieGraphicItem                                                *
*                                                                              *
********************************************************************************
*/
class CategorieGraphicItem : public QGraphicsItem
{
  public:
    CategorieGraphicItem(QWidget* parentView = 0);
    QString     m_name;
    int type() const { return GraphicsItem::CategoryType; }

  protected:
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
    
    QWidget    *m_parent;
};

/*
********************************************************************************
*                                                                              *
*    Class CategorieLayoutItem                                                 *
*                                                                              *
********************************************************************************
*/
class CategorieLayoutItem : public CategorieGraphicItem, public QGraphicsLayoutItem
{
  public:
    CategorieLayoutItem(QWidget* parentView = 0);

  protected:
    // Inherited from QGraphicsLayoutItem
    void setGeometry(const QRectF &geom);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
};


#endif // _ITEM_COMMON_H_
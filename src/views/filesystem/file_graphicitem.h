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

#ifndef _FILE_GRAPHICS_ITEM_H_
#define _FILE_GRAPHICS_ITEM_H_

#include <QGraphicsItem>
#include <QStyleOptionViewItemV4>
#include <QIcon>

#include "item_common.h"
#include "views/local/local_item.h"

static QIcon icon_folder;
/*
********************************************************************************
*                                                                              *
*    Class DirectoryGraphicItem                                                *
*      ->                                                                      *
********************************************************************************
*/
class DirectoryGraphicItem : public QGraphicsItem
{
  public:
    DirectoryGraphicItem();

    int type() const { return GraphicsItem::FileSystemType; }

    void setPath(const QString & d) {m_path = d;}
    void setDirname(const QString & d) {m_dirname = d;}
    void setPixmap(const QPixmap & p) {pixmap = p;}

    QString dirname() {return m_dirname;}
    QString path() {return m_path;}
        
    void setSymbLink() { isSymbLink = true;}
        
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
     QString  m_dirname;
     QString  m_path;
     QPixmap  pixmap;
     bool     isSymbLink;
};

/*
********************************************************************************
*                                                                              *
*    Class TrackGraphicItem_v4                                                 *
*      -> copy of TrackGraphicItem_v3                                          *
*      -> work for stream or track                                             *
*      -> draw track artist/album/title or stream url name                     *
********************************************************************************
*/
class TrackGraphicItem_v4 : public TrackGraphicItem
{
  public:
    TrackGraphicItem_v4();
    void setPath(const QString & d) {m_path = d;}

  protected:
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    
  private:
     QString  m_path;
};


#endif // _FILE__GRAPHICS_ITEM_H_

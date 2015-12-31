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

#ifndef _ITEM_BUTTON_H_
#define _ITEM_BUTTON_H_

#include <QGraphicsItem>
#include <QGraphicsLayoutItem>


/*
********************************************************************************
*                                                                              *
*    Class ButtonItem                                                          *
*                                                                              *
********************************************************************************
*/
class ButtonItem : public QGraphicsObject, public QGraphicsLayoutItem
{
Q_OBJECT
Q_INTERFACES(QGraphicsLayoutItem)
  public:
    ButtonItem();

    void setPixmap(const QPixmap& p );
    void setText(const QString& t);

    void setData(QVariant);
    QVariant data() {return m_data;}

    void setColor(QColor color);

    int height() {return boundingRect().toRect().height();}
    int width() {return boundingRect().toRect().width();}

  protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    // Inherited from QGraphicsLayoutItem
    void setGeometry(const QRectF &geom);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

  protected:
    QPixmap     m_pixmap;
    QString     m_text;
    qreal       m_alpha;
    QVariant    m_data;
    QColor      m_brush_color;

  signals:
    void clicked();
};


/*
********************************************************************************
*                                                                              *
*    Class ButtonStateItem                                                     *
*                                                                              *
********************************************************************************
*/
class ButtonStateItem : public ButtonItem
{
Q_OBJECT
  public:
    ButtonStateItem();

    void setChecked(bool b) {m_checked = b;}
    bool isChecked() {return m_checked;}

  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

  private:
    bool        m_checked;
};

#endif //_ITEM_BUTTON_H_

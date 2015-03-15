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

#include "customsplitter.h"

#include <QPaintEvent>
#include <QPainter>
#include <QSplitterHandle>
#include <QApplication>

/*
********************************************************************************
*                                                                              *
*    Class CustomSplitterHandle                                                *
*                                                                              *
********************************************************************************
*/
class CustomSplitterHandle : public QSplitterHandle
{
public:
    CustomSplitterHandle(Qt::Orientation orientation, QSplitter *parent)
    : QSplitterHandle(orientation, parent)
    {
        setMask(QRegion(contentsRect()));
        setAttribute(Qt::WA_MouseNoMask, true);
        _alpha = 0.4;
    }

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void leaveEvent ( QEvent * event );
    void enterEvent ( QEvent * event );

private :
    qreal    _alpha;
};

void CustomSplitterHandle::leaveEvent ( QEvent * event )
{
Q_UNUSED(event);
    _alpha = 0.4;
}

void CustomSplitterHandle::enterEvent ( QEvent * event )
{
Q_UNUSED(event);
    _alpha = 1.0;
}


void CustomSplitterHandle::resizeEvent(QResizeEvent *event)
{
    setMask(QRegion(contentsRect()));
    QSplitterHandle::resizeEvent(event);
}

void CustomSplitterHandle::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect rect = event->rect();

    QColor outlineColor = QApplication::palette().color(QPalette::Inactive, QPalette::AlternateBase);
    outlineColor.setAlphaF(_alpha);

    QLinearGradient gradientTop;
    gradientTop.setStart(rect.left(), rect.height()/2);
    gradientTop.setFinalStop(rect.right(), rect.top()-30);
    gradientTop.setColorAt(0, outlineColor);
    gradientTop.setColorAt(1, Qt::transparent);
    painter.fillRect(rect.adjusted(0,0,0,-rect.height()/2), QBrush(gradientTop));

    QLinearGradient gradientBottom;
    gradientBottom.setStart(rect.left(), rect.height()/2 + 10);
    gradientBottom.setFinalStop(rect.right(), rect.bottom()+30);
    gradientBottom.setColorAt(0, outlineColor);
    gradientBottom.setColorAt(1, Qt::transparent);
    painter.fillRect(rect.adjusted(0,rect.height()/2,0,0), QBrush(gradientBottom));
}

/*
********************************************************************************
*                                                                              *
*    Class CustomSplitter                                                      *
*                                                                              *
********************************************************************************
*/
CustomSplitter::CustomSplitter(QWidget *parent) : QSplitter(parent)
{
    setHandleWidth(1);
    setChildrenCollapsible(false);

    QPalette palette;
    palette.setColor(QPalette::Background, QApplication::palette().color(QPalette::Normal, QPalette::Base));
    setPalette(palette);

    setAutoFillBackground(true);
    setOrientation(Qt::Horizontal);
    setOpaqueResize(true);
    setContentsMargins(0, 0, 0, 0);
}


QSplitterHandle *CustomSplitter::createHandle()
{
    return new CustomSplitterHandle(orientation(), this);
}


void CustomSplitter::paintEvent(QPaintEvent *event)
{
    QSplitter::paintEvent(event);
}

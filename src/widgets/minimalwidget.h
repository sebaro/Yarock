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
#ifndef _APP_MINIMAL_WIDGET_H_
#define _APP_MINIMAL_WIDGET_H_

#include <QWidget>
#include <QObject>
#include <QPaintEvent>
#include <QLabel>
#include <QToolBar>
#include <QObject>
#include <QEvent>


/*
********************************************************************************
*                                                                              *
*    Class MinimalWidget                                                       *
*                                                                              *
********************************************************************************
*/
class MinimalWidget : public QWidget
{
Q_OBJECT
public:
    MinimalWidget(QWidget *parent=0);

protected :
    void paintEvent( QPaintEvent *event );
    void showEvent ( QShowEvent * event );
    bool eventFilter( QObject *obj, QEvent *ev );
    
private slots:
    void slot_update();
    
private:    
    QLabel                *ui_image;
    QLabel                *ui_label_title;    
    QLabel                *ui_label_album;    
    QToolBar              *ui_toolbar;
    class RatingWidget    *ui_rating;
};


#endif // _APP_MINIMAL_WIDGET_H_

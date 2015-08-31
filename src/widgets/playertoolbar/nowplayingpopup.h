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

#ifndef _NOWPLAYING_POPUP_H_
#define _NOWPLAYING_POPUP_H_

//Qt
#include <QWidget>
#include <QToolBar>
#include <QLabel>
#include <QTimer>
/*
********************************************************************************
*                                                                              *
*    Class NowPlayingPopup                                                     *
*                                                                              *
********************************************************************************
*/
class NowPlayingPopup : public QWidget
{
Q_OBJECT
  public:
    NowPlayingPopup(QWidget *parent);
    void updateWidget();

  private slots:
    void slot_update_widget();
    void slot_rating_changed(float);
    void slot_on_lastfm_love();
    void slot_on_track_edit();

  private:
    QLabel                *ui_image;
    QLabel                *ui_label_title;    
    QLabel                *ui_label_album;    
    QToolBar              *ui_toolbar;
    class RatingWidget    *ui_rating;    
};


#endif // _NOWPLAYING_POPUP_H_
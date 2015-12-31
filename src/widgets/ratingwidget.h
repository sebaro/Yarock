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
// based on Clementine Player, thanks for the good work
#ifndef _RATINGWIDGET_H_
#define _RATINGWIDGET_H_

#include <QFrame>
#include <QPixmap>

/*
********************************************************************************
*                                                                              *
*    Class RatingPainter                                                       *
*                                                                              *
********************************************************************************
*/
class RatingPainter
{
static RatingPainter* INSTANCE;

public:
  RatingPainter();
  static RatingPainter* instance() { return INSTANCE; }

  static const int kStarCount = 5;
  static const int kStarSize = 15;
  static QRect Contents(const QRect& rect);
  static double RatingForPos(const QPoint& pos, const QRect& rect);
  void Paint(QPainter* painter, const QRect& rect, float rating, bool user_rating=false);

private:
  QPixmap stars_[kStarCount*2+1];
  QPixmap stars_2[kStarCount*2+1];
};
/*
********************************************************************************
*                                                                              *
*    Class RatingWidget                                                        *
*                                                                              *
********************************************************************************
*/
class RatingWidget : public QWidget {
  Q_OBJECT
  //Q_PROPERTY(float rating READ rating WRITE set_rating);

public:
  RatingWidget(QWidget* parent = 0);

  QSize sizeHint() const;

  float rating() const { return rating_; }
  void set_rating(float rating);

  void set_draw_frame(bool b) {draw_frame_ = b;}
  void set_enable(bool b) {enable_ = b;}
  void set_user_rating(bool b);

signals:
  void RatingChanged(float rating);

protected:
  void paintEvent(QPaintEvent*);
  void mousePressEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void leaveEvent(QEvent*);

private:
//  RatingPainter painter_;
  float rating_;
  float hover_rating_;
  bool  user_rating_;
  bool  draw_frame_;
  bool  enable_;
};

#endif // _RATINGWIDGET_H_

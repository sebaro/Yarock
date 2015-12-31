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

#include "ratingwidget.h"
//#include "debug.h"

#include <QMouseEvent>
#include <QStyleOptionFrameV3>
#include <QStylePainter>


const int RatingPainter::kStarCount;
const int RatingPainter::kStarSize;

RatingPainter* RatingPainter::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class RatingPainter                                                       *
*                                                                              *
********************************************************************************
*/
RatingPainter::RatingPainter()
{
    INSTANCE = this;

    // Load the base pixmaps
    QPixmap on(":/images/star-on.png");
    QPixmap on2(":/images/star-on-2.png");
    QPixmap off(":/images/star-off.png");

    // Generate the 10 states, better to do it now than on the fly
    for (int i=0 ; i<kStarCount*2+1 ; ++i)
    {
      const float rating = float(i) / 2.0;

      // Clear the pixmap
      stars_[i] = QPixmap(kStarSize * kStarCount, kStarSize);
      stars_[i].fill(Qt::transparent);
      QPainter p(&stars_[i]);

      stars_2[i] = QPixmap(kStarSize * kStarCount, kStarSize);
      stars_2[i].fill(Qt::transparent);
      QPainter p2(&stars_2[i]);

      // Draw the stars
      int x = 0;
      for (int i=0 ; i<kStarCount ; ++i, x+=kStarSize) {
        const QRect rect(x, 0, kStarSize, kStarSize);

        if (rating - 0.25 <= i) {
          // Totally empty
          p.drawPixmap(rect, off);
          p2.drawPixmap(rect, off);
        } else if (rating - 0.75 <= i) {
          // Half full
          const QRect target_left(rect.x(), rect.y(), kStarSize/2, kStarSize);
          const QRect target_right(rect.x() + kStarSize/2, rect.y(), kStarSize/2, kStarSize);
          const QRect source_left(0, 0, kStarSize/2, kStarSize);
          const QRect source_right(kStarSize/2, 0, kStarSize/2, kStarSize);
          p.drawPixmap(target_left, on, source_left);
          p.drawPixmap(target_right, off, source_right);
          p2.drawPixmap(target_left, on2, source_left);
          p2.drawPixmap(target_right, off, source_right);
        } else {
          // Totally full
          p.drawPixmap(rect, on);
          p2.drawPixmap(rect, on2);
        }
      }
    }
}


QRect RatingPainter::Contents(const QRect& rect)
{
  const int width = kStarSize * kStarCount;
  const int x = rect.x() + (rect.width() - width) / 2;

  return QRect(x, rect.y(), width, rect.height());
}

double RatingPainter::RatingForPos(const QPoint& pos, const QRect& rect)
{
  const QRect contents = Contents(rect);
  const double raw = double(pos.x() - contents.left()) / contents.width();

  // Round to the nearest 0.1
  return double(int(raw * kStarCount * 2 + 0.5)) / (kStarCount * 2);
}

void RatingPainter::Paint(QPainter* painter, const QRect& rect, float rating, bool user_rating)
{
  QSize size(qMin(kStarSize*kStarCount, rect.width()),
             qMin(kStarSize, rect.height()));
  QPoint pos(rect.center() - QPoint(size.width() / 2, size.height() / 2));

  rating *= kStarCount;

  // Draw the stars
  const int star = qBound(0, int(rating*2.0 + 0.5), kStarCount*2);
  if(user_rating)
    painter->drawPixmap(QRect(pos, size), stars_2[star], QRect(QPoint(0,0), size));
  else
    painter->drawPixmap(QRect(pos, size), stars_[star], QRect(QPoint(0,0), size));
}

/*
********************************************************************************
*                                                                              *
*    Class RatingWidget                                                        *
*                                                                              *
********************************************************************************
*/
RatingWidget::RatingWidget(QWidget* parent) : QWidget(parent)
{
    user_rating_    = false;
    draw_frame_     = true;
    enable_         = true;
    rating_         = -1.0;
    hover_rating_   = -1.0;

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setMouseTracking(true);
}

QSize RatingWidget::sizeHint() const
{
    int frame_width = 0;

    if(draw_frame_)
      frame_width = 1 + style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    return QSize(RatingPainter::kStarSize * (RatingPainter::kStarCount+2) + frame_width*2,
               RatingPainter::kStarSize + frame_width*2);
}

void RatingWidget::set_user_rating(bool b)
{
    user_rating_    = b;
    update();
}


void RatingWidget::set_rating(float rating)
{
    rating_ = rating;
    update();
}

void RatingWidget::paintEvent(QPaintEvent* e)
{
Q_UNUSED(e)
    QStylePainter p(this);

    // Draw the background
    if(draw_frame_)
    {
      QStyleOptionFrameV3 opt;
      opt.initFrom(this);
      opt.state |= QStyle::State_Sunken;
      opt.frameShape = QFrame::StyledPanel;
      opt.lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, this);
      opt.midLineWidth = 0;

      p.drawPrimitive(QStyle::PE_PanelLineEdit, opt);
    }

    // Draw the stars
    RatingPainter::instance()->Paint(&p, rect(), hover_rating_ == -1.0 ? rating_ : hover_rating_, user_rating_);
}

void RatingWidget::mousePressEvent(QMouseEvent* e)
{
    if(!enable_)
      return;

    rating_ = RatingPainter::RatingForPos(e->pos(), rect());
    emit RatingChanged(rating_);
    //Debug::debug() << "RatingWidget::emit RatingChanged";
}

void RatingWidget::mouseMoveEvent(QMouseEvent* e)
{
    if(!enable_)
      return;

    hover_rating_ = RatingPainter::RatingForPos(e->pos(), rect());
    update();
    //Debug::debug() << "RatingWidget::mouseMoveEvent hover_rating_" << hover_rating_;
}

void RatingWidget::leaveEvent(QEvent*)
{
    if(!enable_)
      return;

    hover_rating_ = -1.0;
    update();
    //Debug::debug() << "RatingWidget::leaveEvent hover_rating_" << hover_rating_;
}

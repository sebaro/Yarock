/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2014 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

#ifndef _SEEKSLIDER_H_
#define _SEEKSLIDER_H_

#include <QProgressBar>
#include <QMouseEvent>

/*
********************************************************************************
*                                                                              *
*    Class SeekSlider                                                          *
*                                                                              *
********************************************************************************
*/
class SeekSlider : public QProgressBar
{
Q_OBJECT
  public:
    explicit SeekSlider(QWidget *parent = 0);

  protected:
    virtual void mousePressEvent ( QMouseEvent * event );

  private :
    void _seek(int);
    void _stop();

  private slots:
    void slot_stateChanged();
    void slot_tick(qint64);
    void slot_length(qint64);
    void slot_seekableChanged(bool);
    void slot_currentSourceChanged();

  private:
    bool        m_enable;
    bool        m_ticking;
};

#endif // _SEEKSLIDER_H_

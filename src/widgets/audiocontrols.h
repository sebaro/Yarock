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

#ifndef _AUDIO_CONTROLS_H_
#define _AUDIO_CONTROLS_H_

#include <QToolButton>
#include <QWidget>


/*
********************************************************************************
*                                                                              *
*    Class RepeatControl                                                       *
*                                                                              *
********************************************************************************
*/
class RepeatControl : public QToolButton
{
Q_OBJECT
  public:
    RepeatControl( QWidget *);
    static RepeatControl         *INSTANCE;
    static RepeatControl* instance() { return INSTANCE; }

    void setState(int intState);
    int getState() {return (int)m_state;}

  private:
    enum RepeatMode {RepeatOff = 0,RepeatTrack = 1, RepeatAll = 2 };
    RepeatMode   m_state;

  private slots:
    void onButtonClicked();

  signals:
    void repeatStateChange(int);
};

/*
********************************************************************************
*                                                                              *
*    Class ShuffleControl                                                      *
*                                                                              *
********************************************************************************
*/
class ShuffleControl : public QToolButton
{
Q_OBJECT
  public:
    ShuffleControl( QWidget *);
    static ShuffleControl         *INSTANCE;
    static ShuffleControl* instance() { return INSTANCE; }

    void setState(int intState);
    int getState() {return (int)m_state;}

  private:
    enum ShuffleMode {ShuffleOff = 0 ,ShuffleOn = 1};
    ShuffleMode     m_state;

  private slots:
    void onButtonClicked();

  signals:
    void shuffleStateChange(int);
};

#endif // _AUDIO_CONTROLS_H_


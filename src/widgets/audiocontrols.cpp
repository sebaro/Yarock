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

#include "audiocontrols.h"
#include "settings.h"
//#include "debug.h"

#include <QSizePolicy>


RepeatControl* RepeatControl::INSTANCE = 0;
ShuffleControl* ShuffleControl::INSTANCE = 0;
/*
********************************************************************************
*                                                                              *
*    Class RepeatControl                                                       *
*                                                                              *
********************************************************************************
*/
RepeatControl::RepeatControl(QWidget *parent)  : QToolButton( parent )
{
    INSTANCE = this;
    this->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    this->setIconSize( QSize( 32, 32 ) );
    this->setIcon(QIcon(":/images/media-repeat-1-48x48.png"));
    this->setToolTip(tr("Repeat mode is off"));
    this->setAutoRaise(true);

    connect(this, SIGNAL(clicked()), this, SLOT(onButtonClicked()));

    // state restore
    setState( SETTINGS()->_repeatMode );
}

void RepeatControl::onButtonClicked()
{
    //Debug::debug() << "- RepeatControl -> onButtonClicked";
    switch(m_state) {
      case RepeatOff   : setState( (int) RepeatTrack ); break;
      case RepeatTrack : setState( (int) RepeatAll ); break;
      case RepeatAll   : setState( (int) RepeatOff ); break;
      default:break;
    }
}

void RepeatControl::setState(int intState)
{
    SETTINGS()->_repeatMode = intState;

    m_state = (RepeatMode) intState;

    switch(m_state) {
      case RepeatOff :
        this->setIcon(QIcon(":/images/media-repeat-1-48x48.png"));
        this->setToolTip(tr("Repeat mode is off"));
      break;
      case RepeatTrack :
         this->setIcon(QIcon(":/images/media-repeat-2-48x48.png"));
         this->setToolTip(tr("Repeat track"));
      break;
      case RepeatAll :
         this->setIcon(QIcon(":/images/media-repeat-3-48x48.png"));
         this->setToolTip(tr("Repeat all"));
      break;
      default:break;
    }

   emit repeatStateChange((int) m_state);
}


/*
********************************************************************************
*                                                                              *
*    Class ShuffleControl                                                      *
*                                                                              *
********************************************************************************
*/
ShuffleControl::ShuffleControl(QWidget *parent)  : QToolButton( parent )
{
    INSTANCE = this;
    this->setIconSize( QSize( 32, 32 ) );
    this->setIcon(QIcon(":/images/media-shuffle-1-48x48.png"));
    this->setToolTip(tr("Shuffle mode is off"));
    this->setAutoRaise(true);

    connect(this, SIGNAL(clicked()), this, SLOT(onButtonClicked()));

    // state restore
    setState( SETTINGS()->_shuffleMode );
}


void ShuffleControl::onButtonClicked()
{
    //Debug::debug() << "- RepeatControl -> onButtonClicked";
    switch(m_state) {
      case ShuffleOff : setState( (int) ShuffleOn ); break;
      case ShuffleOn  : setState( (int) ShuffleOff ); break;
      default:break;
    }
}

void ShuffleControl::setState(int intState)
{
    SETTINGS()->_shuffleMode = intState;

    m_state = (ShuffleMode) intState;

    switch(m_state) {
      case ShuffleOff :
         this->setIcon(QIcon(":/images/media-shuffle-1-48x48.png"));
         this->setToolTip(tr("Shuffle mode is off"));
      break;
      case ShuffleOn :
         this->setIcon(QIcon(":/images/media-shuffle-2-48x48.png"));
         this->setToolTip(tr("Shuffle mode is on"));
      break;
      default:break;
    }
   emit shuffleStateChange((int) m_state);
}



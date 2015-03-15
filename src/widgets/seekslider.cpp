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

#include "seekslider.h"
#include "core/mediaitem/mediaitem.h"
#include "core/player/engine.h"
#include "debug.h"

#include "settings.h"


#include <QApplication>

/*
********************************************************************************
*                                                                              *
*    Class SeekSlider                                                          *
*                                                                              *
********************************************************************************
*/

SeekSlider::SeekSlider(QWidget *parent) : QProgressBar(parent)
{
    //! build seekslider
    this->setOrientation(Qt::Horizontal);
    this->setEnabled(false);
    this->setTextVisible(false);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setMaximumHeight(6);
    this->setMinimumHeight(6);

    //! signals
    connect(Engine::instance(), SIGNAL(engineStateChanged()),this, SLOT(slot_stateChanged()));

    connect(Engine::instance(), SIGNAL(mediaTotalTimeChanged(qint64)), this, SLOT(slot_length(qint64)));
    connect(Engine::instance(), SIGNAL(mediaTick(qint64)), this, SLOT(slot_tick(qint64)));

    connect(Engine::instance(), SIGNAL(mediaSeekableChanged(bool)), this, SLOT(slot_seekableChanged(bool)));
    connect(Engine::instance(), SIGNAL(mediaChanged()), this, SLOT(slot_currentSourceChanged()));

    this->setEnabled(true);
    reset ();
    setCursor(Qt::PointingHandCursor);
    
   
    //! pretty style
    this->setStyleSheet(
      QString(
        QString( "QProgressBar {    border: 0px;background-color: %1 ;border-radius: 0px;}")
          .arg( QApplication::palette().color( QPalette::Window ).name() )
        +
        QString( " QProgressBar::chunk {  background-color: %1;}").arg(SETTINGS()->_baseColor.name())
      )
    );
}

void SeekSlider::_seek(int msec)
{
    if (!m_ticking) {
      Engine::instance()->seek(msec);
    }
}

void SeekSlider::mousePressEvent ( QMouseEvent * event )
{
     Debug::debug() << "-- SeekSlider -> mousePressEvent ";

    _seek((double)event->x()*maximum()/width());
}

void SeekSlider::slot_tick(qint64 msec)
{
    //Debug::debug() << "## SeekSlider -> slot_tick(qint64 msec) msec = " << msec;
    m_ticking = true;
    this->setValue(msec);
    m_ticking = false;
}

void SeekSlider::slot_length(qint64 msec)
{
    m_ticking = true;
    if(msec <= 0)
    {
      this->setRange(0, 1);
      this->setValue(0);
    }
    else
    {
      //Debug::debug() << "-- SeekSlider -> slot_length(qint64 msec) msec = " << msec;
      this->setRange(0, msec);
    }
    m_ticking = false;
}

void SeekSlider::slot_seekableChanged(bool isSeekable)
{
    //Debug::debug() << "-- SeekSlider -> slot_seekableChanged  isSeekable" << isSeekable;
    if (!isSeekable) {
        //_stop();
        this->setRange(0, 1);
        this->setValue(0);
        return;
    }
    
    switch (Engine::instance()->state()) {
      case ENGINE::PLAYING:
      case ENGINE::PAUSED:
        //this->setEnabled(true);
        m_enable = true;
        break;
      case ENGINE::STOPPED:
      case ENGINE::ERROR:
        //_stop();
        this->setRange(0, 1);
        this->setValue(0);
        break;
    }
}

void SeekSlider::slot_currentSourceChanged()
{
    //this releases the mouse and makes the seek slider stop seeking if the current source has changed
    QMouseEvent event(QEvent::MouseButtonRelease, QPoint(), Qt::LeftButton, 0, 0);
    QApplication::sendEvent(this, &event);
}

void SeekSlider::slot_stateChanged()
{
    Debug::debug() << "-- SeekSlider -> slot_stateChanged";
    switch (Engine::instance()->state()) {
      case ENGINE::PLAYING:
      case ENGINE::PAUSED:
        m_enable = true;
        break;
      case ENGINE::STOPPED:
      case ENGINE::ERROR:
        this->setRange(0, 1);
        this->setValue(0);
        break;
      default:break;
    }
}


void SeekSlider::_stop()
{
    //Debug::debug() << " ### SeekSlider -> STOP";
    m_enable = false;
    this->setRange(0, 1);
    this->setValue(0);
}

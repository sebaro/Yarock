/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2018 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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
#include "seeksliderpopup.h"
#include "core/mediaitem/mediaitem.h"
#include "core/player/engine.h"
#include "debug.h"

#include "utilities.h"
#include "settings.h"


#include <QApplication>
#include <QStyleOptionSlider>

/*
********************************************************************************
*                                                                              *
*    Class SeekSlider                                                          *
*                                                                              *
********************************************************************************
*/

SeekSlider::SeekSlider(QWidget *parent) : QProgressBar(parent)
{
    /* build seekslider */
    this->setOrientation(Qt::Horizontal);
    this->setTextVisible(false);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setMaximumHeight(6);
    this->setMinimumHeight(6);
    this->setMouseTracking(true);
    this->setCursor(Qt::PointingHandCursor);

    /* signals */
    connect(Engine::instance(), SIGNAL(mediaTotalTimeChanged(qint64)), this, SLOT(slot_length(qint64)));
    connect(Engine::instance(), SIGNAL(mediaTick(qint64)), this, SLOT(slot_tick(qint64)));

    connect(Engine::instance(), SIGNAL(mediaChanged()), this, SLOT(slot_stateChanged()));
    connect(Engine::instance(), SIGNAL(engineStateChanged()), this, SLOT(slot_stateChanged()));

    /* popup */
    m_popup = new SeekSliderPopup(window());

    /* pretty style */
    this->setStyleSheet(
      QString(
        QString( "QProgressBar {    border: 0px;background-color: %1 ;border-radius: 0px;}")
          .arg( QApplication::palette().color( QPalette::Window ).name() )
        +
        QString( " QProgressBar::chunk {  background-color: %1;}").arg(SETTINGS()->_baseColor.name())
      )
    );

    /* init state */
    stop();
}

void SeekSlider::seek(int msec)
{
    if (!m_ticking) {
      Engine::instance()->seek(msec);
    }
}

void SeekSlider::mousePressEvent ( QMouseEvent * event )
{
    //Debug::debug() << "      [SeekSlider] mousePressEvent ";

    seek((double)event->position().x()*maximum()/width());
}

void SeekSlider::slot_tick(qint64 msec)
{
    //Debug::debug() << "      [SeekSlider] slot_tick(qint64 msec) msec = " << msec;
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
      //Debug::debug() << "      [SeekSlider] slot_length(qint64 msec) msec = " << msec;
      this->setRange(0, msec);
    }
    m_ticking = false;
}


void SeekSlider::slot_stateChanged()
{
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();

    if( Engine::instance()->state() != ENGINE::STOPPED && track )
    {
      /* simple way : url no seekable/ local file seekable */
      /* replace with player real status of seekable ? */
      /* seekable signal is not used anymore removed because signal is not emitted */
      /* correctly for all engine (phonon, vlc, mpv) */
      bool local = MEDIA::isLocal( track->url );
      if( local )
      {
        setEnabled(true);
        m_enable = true;
      }
      else
      {
        stop();
      }
    }
    else
    {
      stop();
    }
}


void SeekSlider::stop()
{
    //Debug::debug() << "      [SeekSlider] STOP";
    setEnabled( false );
    m_enable = false;
    this->setRange(0, 1);
    this->setValue(0);
}


void SeekSlider::enterEvent(QEnterEvent* event)
{
    //Debug::debug() << "      [SeekSlider] enterEvent";
    QProgressBar::enterEvent(event);
    if (m_enable)
      m_popup->show();
}

void SeekSlider::leaveEvent(QEvent* event)
{
    QProgressBar::leaveEvent(event);
    m_popup->hide();
}


void SeekSlider::mouseReleaseEvent(QMouseEvent* event)
{
    QProgressBar::mouseReleaseEvent(event);
}


void SeekSlider::mouseMoveEvent(QMouseEvent* event)
{
    //Debug::debug() << "      [SeekSlider] mouseMoveEvent";

    QProgressBar::mouseMoveEvent(event);

    if( m_enable )
    {
      m_mouse_hover_sec = ((double)event->position().x()*maximum()/width()) / 1000;

      m_popup->SetText( UTIL::durationToString( m_mouse_hover_sec ) );

      int delta_seconds = m_mouse_hover_sec - value()/1000;

      m_popup->SetSmallText( UTIL::deltaTimeToString(delta_seconds) );

      m_popup->SetPopupPosition(mapTo(window(), QPoint(
         event->position().x(), rect().center().y()))
      );
    }
}


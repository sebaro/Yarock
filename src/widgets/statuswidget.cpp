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

#include "statuswidget.h"
#include "mainwindow.h"
#include "debug.h"

#include <QLayout>
#include <QApplication>
#include <QtGlobal>

/*
********************************************************************************
*                                                                              *
*    Class StatusWidget                                                        *
*                                                                              *
********************************************************************************
*/
StatusWidget* StatusWidget::INSTANCE = 0;

StatusWidget::StatusWidget(QWidget *parent) : QFrame(parent)
{
    INSTANCE = this;
    m_parent = parent;

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    this->setAttribute(Qt::WA_TransparentForMouseEvents);
    this->setContentsMargins( 0, 0, 0, 0 );
    this->setFixedHeight( 40 );
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    
    //! label
    message = new QLabel(this);
    message->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

    message->setFont(QApplication::font("QTipLabel"));
    message->setMargin(0);
    message->setIndent(1);
    message->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    message->setFrameStyle(QFrame::NoFrame);

    //! pixmap
    pixmap  = new QLabel(this);
    pixmap->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    //! layout
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins (4,4,4,4);

    layout->addWidget(pixmap);
    layout->addWidget(message);
    setLayout(layout);

    isShortMessage = false;
    
    MainWindow::instance()->installEventFilter(this);
    
    m_short_timer = new QTimer(this);
    m_short_timer->setSingleShot(true);

    connect(m_short_timer, SIGNAL(timeout()), this, SLOT(stopShortMessage()));
}



static uint messageId = 0;

/*******************************************************************************
   startProgressMessage
*******************************************************************************/
uint StatusWidget::startProgressMessage(const QString& action)
{
    //Debug::debug() << "- StatusWidget -> startProgressMessage :" << action;

    _longMessage.insert(++messageId, action);

    updateMessage();

    return messageId;
}

/*******************************************************************************
   stopProgressMessage
*******************************************************************************/
void StatusWidget::stopProgressMessage(uint id)
{
    //Debug::debug() << "- StatusWidget -> stopProgressMessage -> id "<< id;
    if(_longMessage.contains(id))
      _longMessage.remove(id);

    updateMessage();
}

/*******************************************************************************
   updateProgressMessage
*******************************************************************************/
void StatusWidget::updateProgressMessage(uint id, const QString & message)
{
    if(_longMessage.contains(id))
      _longMessage[id] = message;

    updateMessage();
}

/*******************************************************************************
   startShortMessage
*******************************************************************************/
void StatusWidget::startShortMessage(const QString& m, STATUS::T_MESSAGE type, int ms )
{
    //Debug::debug() << "- StatusWidget -> startShortMessage ->"<< m;

    //! only one short message at one time
    if(isShortMessage)
      stopShortMessage();
    isShortMessage = true;

    message->setText(m);
    m_current_type = type;

    updateMessage();

    m_short_timer->setInterval(ms);
    m_short_timer->start();
}


/*******************************************************************************
   stopShortMessage
*******************************************************************************/
void StatusWidget::stopShortMessage()
{
    m_short_timer->stop();
    isShortMessage = false;

    updateMessage();
}

/*******************************************************************************
   updateMessage
*******************************************************************************/
void StatusWidget::updateMessage()
{
    //Debug::debug() << "- StatusWidget -> updateMessage";

    /*--------------------------------*/
    /* No message                     */
    /* -------------------------------*/
    if(_longMessage.isEmpty() && !isShortMessage) {
      this->hide();
      return;
    }


    /*--------------------------------*/
    /* Long message                   */
    /* -------------------------------*/
    if(!_longMessage.isEmpty()) {
      m_current_type = STATUS::TYPE_INFO;

      QStringList list = _longMessage.values();
      message->setText(list.first());
    }
    /*--------------------------------*/
    /* Short message                  */
    /* -------------------------------*/
    else if(isShortMessage) {
      /* every thing is already done in startShortMessage*/
    }

    /* update pixmap */
    switch(m_current_type) {
      case STATUS::TYPE_INFO       : pixmap->setPixmap(QPixmap(":/images/info-48x48.png").scaled(QSize(28,28), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
      case STATUS::TYPE_PLAYQUEUE  : pixmap->setPixmap(QPixmap(":/images/info-48x48.png").scaled(QSize(28,28), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
      case STATUS::TYPE_WARNING    : pixmap->setPixmap(QPixmap(":/images/warning-48x48.png").scaled(QSize(28,28), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
      case STATUS::TYPE_ERROR      : pixmap->setPixmap(QPixmap(":/images/error-48x48.png").scaled(QSize(28,28), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
      default : pixmap->setPixmap(QPixmap());break;
    }

    
    /* update color */
    QColor fg_color = Qt::black;
    QColor in_color;
    switch(m_current_type) {
      case STATUS::TYPE_INFO       : in_color = QApplication::palette().highlight().color(); break;
      case STATUS::TYPE_PLAYQUEUE  : in_color = QApplication::palette().highlight().color(); break;
      case STATUS::TYPE_WARNING    : in_color = QColor(0xED, 0xC6, 0x62);break;
      case STATUS::TYPE_ERROR      : in_color = QColor(0xeb, 0xbb, 0xbb);break;
      default : break;
    }

    QColor out_color = in_color;
    qreal saturation = out_color.saturationF();
    saturation *= 0.8;
    qreal value = out_color.valueF();
    value *= 0.9;
    out_color.setHsvF( out_color.hueF(), saturation, value, out_color.alphaF() );


    this->setStyleSheet(
                QString( "QFrame { border: 1px ridge %1; " \
                         "background-color: %2; color: %3; border-radius: 4px; }" \
                         "QLabel { border: none;color: %3; }" )
                        .arg( QApplication::palette().color( QPalette::Window ).name() )
                        .arg( out_color.name() )
                        .arg( fg_color.name())
                    );

    /* resize/move and update */
    resize( sizeHint() );
    setPosition();
    update();
    this->show();
}


/*******************************************************************************
    eventFilter
*******************************************************************************/
bool StatusWidget::eventFilter(QObject *obj, QEvent *ev)
{
    //Debug::debug() << "StatusWidget eventFilter  obj" << obj;
    int type = ev->type();
    QWidget *wid = qobject_cast<QWidget*>(obj);

    if (obj == this)
    {
        return false;
    }

    // hide conditions of the SearchPopup
    if (wid && (wid == m_parent ))
    {
      if(type == QEvent::Resize || type == QEvent::Move) {
        if(isVisible())
          setPosition();
        return false;
      }
      else if(type == QEvent::Hide) {
          //this->hide();
      }
    }

    return QWidget::eventFilter(obj, ev);
}

/*******************************************************************************
   setPosition (private slot)
*******************************************************************************/
void StatusWidget::setPosition()
{
    //Debug::debug() << "- StatusWidget -> setPosition   m_parent->height() "  << m_parent->height();
    if(m_current_type == STATUS::TYPE_PLAYQUEUE)
    {

      move(m_parent->width()-this->width() - 3, m_parent->height() - this->height()-2);
    }
    else
    {
      move( 3, m_parent->height() - this->height()-2);
    }
}

/*******************************************************************************
   moveEvent
*******************************************************************************/
void StatusWidget::moveEvent ( QMoveEvent * event )
{
Q_UNUSED(event)
  if(isVisible())
     setPosition();
}

/*******************************************************************************
   resizeEvent
*******************************************************************************/
void StatusWidget::resizeEvent ( QResizeEvent * event )
{
Q_UNUSED(event)
   if(isVisible())
       setPosition();
}


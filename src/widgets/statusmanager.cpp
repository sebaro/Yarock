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

#include "statusmanager.h"
#include "debug.h"

#include <QLabel>
#include <QLayout>
#include <QApplication>
#include <QPushButton>

/*
********************************************************************************
*                                                                              *
*    Class StatusWidget                                                        *
*                                                                              *
********************************************************************************
*/
class StatusWidget : public QFrame
{
Q_OBJECT    
  public:
    StatusWidget(QWidget *parent = 0, STATUS::TYPE t = STATUS::INFO);

  public:
    QLabel        *message;
    QLabel        *pixmap;
    STATUS::TYPE   type;
    QPushButton   *close_button;
};


StatusWidget::StatusWidget(QWidget *parent, STATUS::TYPE t) : QFrame(parent), type(t)
{    
    setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    setContentsMargins( 0, 0, 0, 0 );
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

    /* label */
    message = new QLabel(this);
    message->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

    message->setFont(QApplication::font("QTipLabel"));
    message->setMargin(0);
    message->setIndent(1);
    message->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    message->setFrameStyle(QFrame::NoFrame);

    /* pixmap */
    pixmap  = new QLabel(this);
    pixmap->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    
    switch( type ) {
      case STATUS::INFO        : pixmap->setPixmap(QPixmap(":/images/info-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
      case STATUS::INFO_CLOSE  : pixmap->setPixmap(QPixmap(":/images/info-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
      case STATUS::PLAYQUEUE   : pixmap->setPixmap(QPixmap(":/images/info-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
      case STATUS::WARNING     : pixmap->setPixmap(QPixmap(":/images/warning-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
      case STATUS::ERROR       : pixmap->setPixmap(QPixmap(":/images/error-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
      case STATUS::ERROR_CLOSE : pixmap->setPixmap(QPixmap(":/images/error-48x48.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
      default : pixmap->setPixmap(QPixmap());break;
    }

    /* close button */
    if( type == STATUS::ERROR_CLOSE || type == STATUS::INFO_CLOSE )
    {
      close_button = new QPushButton(this);
      close_button->setIcon(QIcon(":/images/blue-cross-32x32.png"));
      close_button->setStyleSheet("QPushButton { border: none; padding: 0px; }");
    }
    
    /* update color */
    QColor fg_color = Qt::black;
    QColor in_color;
    switch( type ) {
      case STATUS::INFO        : in_color = QApplication::palette().highlight().color(); break;
      case STATUS::INFO_CLOSE  : in_color = QApplication::palette().highlight().color(); break;
      case STATUS::PLAYQUEUE   : in_color = QApplication::palette().highlight().color(); break;
      case STATUS::WARNING     : in_color = QColor(0xED, 0xC6, 0x62);break;
      case STATUS::ERROR       : in_color = QColor(0xeb, 0xbb, 0xbb);break;
      case STATUS::ERROR_CLOSE : in_color = QColor(0xeb, 0xbb, 0xbb);break;
      default : break;
    }

    QColor out_color = in_color;
    qreal saturation = out_color.saturationF();
    saturation *= 0.8;
    qreal value = out_color.valueF();
    value *= 0.9;
    out_color.setHsvF( out_color.hueF(), saturation, value, out_color.alphaF() );


    this->setStyleSheet(
                QString( "QFrame { border: none; " \
                         "background-color: %1; color: %2; border-radius: 2px; }" \
                         "QLabel { border: none;color: %2; }" )
                        .arg( out_color.name() )
                        .arg( fg_color.name() )
                    );
    
    /* layout */
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins (4,4,4,4);

    layout->addWidget(pixmap);
    layout->addWidget(message);
    if( type == STATUS::INFO_CLOSE || type == STATUS::ERROR_CLOSE )
      layout->addWidget(close_button);
    setLayout(layout);
}


/*
********************************************************************************
*                                                                              *
*    Class StatusManager                                                       *
*                                                                              *
********************************************************************************
*/
StatusManager* StatusManager::INSTANCE = 0;

StatusManager::StatusManager(QWidget *parent) : QObject(parent)
{
    INSTANCE = this;
    m_parent = parent;
}



static uint messageId = 0;

/*******************************************************************************
    StatusManager::newStatusWidget
*******************************************************************************/
/* private */
int StatusManager::newStatusWidget(const QString& text, STATUS::TYPE type)
{
    StatusWidget* w = new StatusWidget( m_parent, type );
    
    w->message->setText( text );
    
    int id = ++messageId;
    m_widgets.insert( id, w );
    
    return id;
}
    
/*******************************************************************************
   startMessage
*******************************************************************************/
uint StatusManager::startMessage(const QString& text, STATUS::TYPE type/*=INFO*/, int ms/*=-1*/ )
{
    if( text.isEmpty() )
        return 0;

    /*  present opening same shot message */
    foreach( uint key, m_widgets.keys() )
    {
        StatusWidget* widget  = m_widgets.value( key );
        if( widget->message->text() == text )
            return key;
    }
      
      
    uint id = newStatusWidget( text, type );
    doLayout();    
    
    /* short message */
    if( ms != -1 ) 
    {
      QTimer* timer = new QTimer();
      timer->setSingleShot(true);

      connect(timer, SIGNAL(timeout()), this, SLOT(onTimerStop()));

      m_timers.insert( id, timer );

      timer->setInterval(ms);
      timer->start();        
    }
    
    /* fatal message */
    if( type == STATUS::INFO_CLOSE || type == STATUS::ERROR_CLOSE )
    {
        StatusWidget* sw = m_widgets.value( id );
        connect(sw->close_button, SIGNAL(clicked()), this, SLOT(onCloseClicked()));
    }
    
    return id;
}


/*******************************************************************************
   stopMessage
*******************************************************************************/
void StatusManager::stopMessage(uint id)
{
    //Debug::debug() << "      [StatusManager] stopMessage id:" << id;
    StatusWidget* sw = m_widgets.take( id );
    if( sw )
    {
        sw->hide();
        delete sw;
        
        doLayout();
    }
}

/*******************************************************************************
   updateMessage
*******************************************************************************/
void StatusManager::updateMessage(uint id, const QString& text)
{
    StatusWidget* sw = m_widgets.value( id );
    sw->message->setText( text );

    doLayout();    
}

/*******************************************************************************
   onTimerStop
*******************************************************************************/
void StatusManager::onTimerStop()
{
    QTimer* timer = qobject_cast<QTimer*>(sender());
    if( timer )
    {
        uint key = m_timers.key( timer );
        timer->stop();
        delete m_timers.take( key );
        
        stopMessage(key);
    }
}
/*******************************************************************************
   onCloseClicked
*******************************************************************************/
void StatusManager::onCloseClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    StatusWidget* sw = qobject_cast<StatusWidget*>( button->parent() );
    
    if( sw )
    {
      uint key = m_widgets.key( sw );

      stopMessage( key );
    }    
}

/*******************************************************************************
    StatusManager::doLayout
      -> called from MainWindow resiseEvent
*******************************************************************************/
void StatusManager::onResize()
{
    doLayout();   
}

/*******************************************************************************
    StatusManager::doLayout
*******************************************************************************/
void StatusManager::doLayout()
{
    int XPos = 3;
    foreach( StatusWidget* sw, m_widgets.values() )
    {
       sw->resize( sw->sizeHint() );
       if(sw->type == STATUS::PLAYQUEUE)
       {
         sw->move( m_parent->width()-sw->width() - 3, m_parent->height() - sw->height()-2 );
       }
       else
       {
         sw->move( XPos, m_parent->height() - sw->height()-2 );
         XPos+= sw->width() + 2;
       }   
       sw->update();
       sw->show();
    }
}


#include "statusmanager.moc"    
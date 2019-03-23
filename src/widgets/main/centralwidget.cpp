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

#include "centralwidget.h"

#include "widgets/main/main_right.h"
#include "widgets/main/main_left.h"
#include "menuwidget.h"

#include "playertoolbarbase.h"
#include "playertoolbarcompact.h"
#include "playertoolbarfull.h"


#include "settings.h"
#include "global_actions.h"
#include "debug.h"

#include <QVBoxLayout>
/*
********************************************************************************
*                                                                              *
*    Class CentralWidget                                                       *
*                                                                              *
********************************************************************************
*/
CentralWidget::CentralWidget(QWidget *parent) : QFrame(parent)
{
    m_parent             = parent;

    /* UI setup */
    this->setObjectName(QString::fromUtf8("Central Widget"));
    this->setAutoFillBackground(true);

    this->setFrameShape(QFrame::StyledPanel);
    this->setFrameShadow(QFrame::Sunken);

    /* right widget */
    right_widget = new MainRightWidget(this);

    /* left widget */
    left_widget = new MainLeftWidget(this);

    /* toolbar */
    if(SETTINGS()->_isbottombarexpanded)
        new PlayerToolBarFull(this);
    else
        new PlayerToolBarCompact(this);
    
    
    /* content widget splitter population */
    m_viewsSplitter_1 = new CustomSplitter(this);
    m_viewsSplitter_1->setObjectName(QString::fromUtf8("viewsSplitter_1"));
    m_viewsSplitter_1->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    m_viewsSplitter_1->addWidget( left_widget->contentWidget() );
    m_viewsSplitter_1->addWidget( right_widget->contentWidget() );    
    
    QPalette p1;
    p1.setColor(QPalette::Background, QApplication::palette().color(QPalette::Normal, QPalette::Base));
    m_viewsSplitter_1->setPalette( p1 );
        
    /* header widget splitter population */
    m_viewsSplitter_1b = new CustomSplitter(this);
    m_viewsSplitter_1b->setObjectName(QString::fromUtf8("viewsSplitter_1b"));
    m_viewsSplitter_1b->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    m_viewsSplitter_1b->addWidget( left_widget->headerWidget() );
    m_viewsSplitter_1b->addWidget( right_widget->headerWidget() );    
    m_viewsSplitter_1b->handle(1)->setEnabled(false);
    m_viewsSplitter_1b->handle(1)->setAttribute( Qt::WA_TransparentForMouseEvents );
    m_viewsSplitter_1b->handle(1)->setFocusPolicy( Qt::NoFocus );


    QPalette p2;    
    p2.setColor(QPalette::Background,  QApplication::palette().color( QPalette::Window ) );
    m_viewsSplitter_1b->setPalette( p2 );
    
    /* final layout */
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);    
    layout->addWidget( m_viewsSplitter_1b );
    layout->addWidget( m_viewsSplitter_1 );
    layout->setStretch(1,1);    
    layout->addWidget( PlayerToolBarBase::instance() );

    /* signals connection */
    connect(ACTIONS()->value(APP_SHOW_PLAYQUEUE),   SIGNAL(triggered()), SLOT(slot_show_playlist()));
    connect(PlayerToolBarBase::instance(),   SIGNAL(switchToolBarType()), SLOT(slot_switch_playertoolbar()));

    
    /* event filter for splitter synchro */
    right_widget->contentWidget()->installEventFilter(this);
    
    
    PlayerToolBarBase::instance()->fullUpdate();
}

/*******************************************************************************
    eventFilter
*******************************************************************************/
bool CentralWidget::eventFilter(QObject *obj, QEvent *ev)
{
    //Debug::debug() << "CentralWidget eventFilter  obj" << obj;
    const int type = ev->type();
    const QWidget *wid = qobject_cast<QWidget*>(obj);

    if (obj == this)
    {
        return false;
    }

    // hide conditions of the SearchPopup
    if (wid && (wid == right_widget->contentWidget() ))
    {
      if(type == QEvent::Resize) 
      {
          //Debug::debug() << "CentralWidget eventFilter  RESIZE EVENT";
          m_viewsSplitter_1b->setSizes( m_viewsSplitter_1->sizes() );
          return false;
      }
      else if(type == QEvent::Hide)
      {
          //Debug::debug() << "CentralWidget eventFilter  HIDE EVENT";
          right_widget->headerWidget()->hide();
      }
      else if(type == QEvent::Show) 
      {
          //Debug::debug() << "CentralWidget eventFilter  SHOW EVENT";
          right_widget->headerWidget()->show();
      }
    }

    return QWidget::eventFilter(obj, ev);
}


/*******************************************************************************
    saveState
*******************************************************************************/
void CentralWidget::saveState()
{
      SETTINGS()->_splitterState_1   = m_viewsSplitter_1->saveState();
}

/*******************************************************************************
    restoreState
*******************************************************************************/
/* only called at startup through MainWindow */
void CentralWidget::restoreState()
{
    slot_show_playlist();

    // splitter state
    if(!SETTINGS()->_splitterState_1.isEmpty()) 
    {
      m_viewsSplitter_1->restoreState(SETTINGS()->_splitterState_1);
    }
    else 
    {
      QList<int> list;
      list << 800 << 350;
      m_viewsSplitter_1->setSizes (list);
    }
}


/*******************************************************************************
    slot_show_playlist
*******************************************************************************/
void CentralWidget::slot_show_playlist( )
{
    SETTINGS()->_showPlayQueuePanel = ACTIONS()->value(APP_SHOW_PLAYQUEUE)->isChecked();

    if(SETTINGS()->_showPlayQueuePanel)
      m_viewsSplitter_1->widget(1)->show();
    else
      m_viewsSplitter_1->widget(1)->hide();
}


/*******************************************************************************
    slot_switch_playertoolbar
*******************************************************************************/
void CentralWidget::slot_switch_playertoolbar( )
{
     Debug::debug() << "CentralWidget slot_switch_playertoolbar";
         
     PlayerToolBarBase* tbb = qobject_cast<PlayerToolBarBase*>(sender());
     
     this->layout()->takeAt(2);       
     
     disconnect(tbb, 0,this, 0);
     
     tbb->deleteLater();   

     if(qobject_cast<PlayerToolBarCompact*>(tbb))
     {
         SETTINGS()->_isbottombarexpanded = true;
         new PlayerToolBarFull(this);
     }   
     else 
     {
         SETTINGS()->_isbottombarexpanded = false;
         new PlayerToolBarCompact(this);
     }
     

     this->layout()->addWidget(PlayerToolBarBase::instance());
     
     PlayerToolBarBase::instance()->fullUpdate();

     connect(PlayerToolBarBase::instance(), SIGNAL(switchToolBarType()), SLOT(slot_switch_playertoolbar()));
}


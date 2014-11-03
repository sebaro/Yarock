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

#include "centralwidget.h"

#include "widgets/main/main_right.h"
#include "widgets/main/main_left.h"
#include "menuwidget.h"
#include "playlistwidget.h"
#include "playertoolbar.h"


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
CentralWidget::CentralWidget(
                       QWidget *parent,
                       PlaylistWidget     *playlistWidget
                       ) : QFrame(parent)
{
    m_parent             = parent;

    /* UI setup */
    this->setObjectName(QString::fromUtf8("Main widget"));
    this->setAutoFillBackground(true);

    this->setFrameShape(QFrame::StyledPanel);
    this->setFrameShadow(QFrame::Sunken);

    /* right widget */
    right_widget = new MainRightWidget(this);
    right_widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding );
    right_widget->addWidget(playlistWidget);
    
    /* left widget */
    left_widget = new MainLeftWidget(this);

    /* splitter population */
    m_viewsSplitter_1 = new CustomSplitter(this);
    m_viewsSplitter_1->setObjectName(QString::fromUtf8("viewsSplitter_1"));
    m_viewsSplitter_1->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    m_viewsSplitter_1->addWidget(left_widget);
    m_viewsSplitter_1->addWidget(right_widget);    
    
    /* final layout */
    QVBoxLayout* centralWidgetLayout = new QVBoxLayout(this);
    centralWidgetLayout->setSpacing(0);
    centralWidgetLayout->setContentsMargins(0, 0, 0, 0);    
    centralWidgetLayout->addWidget(m_viewsSplitter_1);
    centralWidgetLayout->setStretch(0,1);
    centralWidgetLayout->addWidget(new PlayerToolBar(this));

    /* signals connection */
    connect(ACTIONS()->value(APP_SHOW_PLAYQUEUE),   SIGNAL(triggered()), SLOT(slot_show_playlist()));
    connect(ACTIONS()->value(APP_SHOW_MENU),        SIGNAL(triggered()), SLOT(slot_show_menu()));
}

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
    slot_show_menu();

    // splitter state
    if(!SETTINGS()->_splitterState_1.isEmpty()) {
      m_viewsSplitter_1->restoreState(SETTINGS()->_splitterState_1);
    }
    else {
      QList<int> list;
      list << 800 << 350;
      m_viewsSplitter_1->setSizes (list); // 1200
    }
    
    MenuWidget::instance()->restoreState();
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
    slot_show_menu (Hide/Show Menu Widget)
*******************************************************************************/
void CentralWidget::slot_show_menu( )
{
    SETTINGS()->_showMenuPanel = ACTIONS()->value(APP_SHOW_MENU)->isChecked();

    if( SETTINGS()->_showMenuPanel )
      left_widget->splitter()->widget(0)->show();
    else
      left_widget->splitter()->widget(0)->hide();
}

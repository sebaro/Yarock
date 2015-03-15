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

//! local
#include "menuwidget.h"
#include "menumodel.h"
#include "global_actions.h"
#include "settings.h"
#include "debug.h"

//! Qt
#include <QtGui>
#include <QApplication>
#include <QPainter>


MenuWidget* MenuWidget::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class MenuWidget                                                          *
*                                                                              *
********************************************************************************
*/
MenuWidget::MenuWidget(QWidget * parent) : QWidget(parent)
{
    INSTANCE = this;
    m_parent = parent;
    
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    this->setPalette(palette);

    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    this->setMinimumWidth(50);

    /* global instance */
    new MenuModel();
    
    /*  navigator bar  */
    m_menuBar = new MenuBar(this);

    /*  navigator tree  */
      QScrollArea* area = new QScrollArea(this);
      area->setWidgetResizable(true);
      area->setFrameShape(QFrame::NoFrame);
    
      QWidget* w = new QWidget(this);
      QVBoxLayout* vl0 = new QVBoxLayout(w);
      vl0->setSpacing(2);
      vl0->setContentsMargins(4, 4, 4, 4);
      
      for (int i=0; i < MenuModel::instance()->rowCount(QModelIndex()); i++)
      {
        const QModelIndex &idx = MenuModel::instance()->index(i, 0, QModelIndex());

        if( !idx.isValid() ) continue;

        QString text   = idx.data(Qt::DisplayRole).toString();
      
        QLabel* l = new QLabel(text);
        QPalette palette;
        palette.setColor(QPalette::WindowText, SETTINGS()->_baseColor);
        l->setFont(QFont("Arial",12,QFont::Bold));
        l->setPalette(palette);

        vl0->addWidget( l );

        for (int j=0; j < MenuModel::instance()->rowCount(idx); j++)
        {
          const QModelIndex childIdx = MenuModel::instance()->index(j, 0, idx);
          QAction *a = childIdx.data(MenuActionRole).value<QAction*>();
    
          QToolButton* tb = new QToolButton(w);
          tb->setDefaultAction(a);
          tb->setAutoRaise(true);
          tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
          vl0->addWidget(tb);
        }
      }
      area->setWidget(w);
    
    /*  stacked widget */    
    m_stackedWidget = new QStackedWidget;
    m_stackedWidget->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::MinimumExpanding );    
    m_stackedWidget->addWidget(area);
    m_stackedWidget->addWidget(m_menuBar);
     
    /*  ui_control_button */    
    ui_control_button = new QPushButton(this);
    ui_control_button->setFlat(true);
    ui_control_button->setCheckable(true);
    ui_control_button->setStyleSheet ("QPushButton {border:none}"); 
    ui_control_button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QHBoxLayout* hl = new QHBoxLayout();
    hl->setSpacing(0);
    hl->setContentsMargins(0, 0, 0, 0);    
    hl->addWidget(ui_control_button);
    hl->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    
    /* final layout */
    QVBoxLayout* verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    verticalLayout->addWidget(m_stackedWidget);
    verticalLayout->addLayout(hl);
    
    /* init */
    ui_control_button->setChecked(SETTINGS()->_is_menu_bar);
    
    /* action */
    connect(ui_control_button, SIGNAL(toggled(bool)), this, SLOT(slot_change_menu_style()));
}



/*******************************************************************************
    restoreState
*******************************************************************************/
void MenuWidget::restoreState()
{
    slot_change_menu_style();
}

/*******************************************************************************
    slot_change_menu_style
*******************************************************************************/
void MenuWidget::slot_change_menu_style ()
{
//     Debug::debug() << "MenuWidget::slot_change_menu_style is menu bar " << SETTINGS()->_is_menu_bar;
    
    SETTINGS()->_is_menu_bar = ui_control_button->isChecked();
    
    m_stackedWidget->setCurrentIndex(SETTINGS()->_is_menu_bar ? 1 : 0);
    ui_control_button->setIcon(SETTINGS()->_is_menu_bar ? QIcon(":/images/add_32x32.png") : QIcon(":/images/remove_32x32.png"));
    ui_control_button->setToolTip(SETTINGS()->_is_menu_bar ? tr("show extended menu") : tr("show compact menu"));
    ui_control_button->update();
    
    QSplitterHandle *hndl = m_splitter->handle(1);
    hndl->setEnabled(!SETTINGS()->_is_menu_bar);
    
    
    QList<int> list;
    if(SETTINGS()->_is_menu_bar)
      list << 50 << m_parent->width() -50;
    else
      list << 180 << m_parent->width() -180;
      
     m_splitter->setSizes (list);
}


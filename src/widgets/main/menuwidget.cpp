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

//! local
#include "menuwidget.h"
#include "menumodel.h"
#include "debug.h"

//! Qt
#include <QtGui>
#include <QApplication>
#include <QLayout>
#include <QVBoxLayout>


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
    palette.setColor(QPalette::Window, palette.color(QPalette::Base));
    this->setPalette(palette);

    this->setAutoFillBackground(true);

    this->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::MinimumExpanding );
    this->setMinimumWidth(50);
    this->setMaximumWidth(50);


    /* global instance */
    new MenuModel();

    /*  navigator bar  */
    m_menuBar = new MenuBar(this);

    /* layout */
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget( m_menuBar );
}


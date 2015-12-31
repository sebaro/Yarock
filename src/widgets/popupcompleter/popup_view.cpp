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

#include "popup_view.h"

//#include "debug.h"
#include <QApplication>
#include <QScrollBar>

/*
********************************************************************************
*                                                                              *
*    Class PopupView                                                           *
*                                                                              *
********************************************************************************
*/
PopupView::PopupView(QWidget *parent) : QListView(parent)
{
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    this->setPalette(palette);

    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    this->setFrameShape(QFrame::NoFrame);
    this->setFrameShadow(QFrame::Plain);
    this->setContentsMargins(0,0,0,0);
    this->setSpacing(1);

    this->setWordWrap(false);
    this->setTextElideMode(Qt::ElideRight);
    this->setUniformItemSizes(true);
    this->setSelectionMode(QAbstractItemView::SingleSelection);

    //! vertical scrolbar setup
    this->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(this,SIGNAL(clicked(const QModelIndex&)),this, SLOT(slot_item_clicked(const QModelIndex&)));
}


void PopupView::slot_item_clicked(const QModelIndex& idx)
{
    if(idx.isValid()) {
      emit itemActivated( idx.data ( Qt::DisplayRole ).toString() );
    }
}

bool PopupView::event ( QEvent * e )
{
    return QListView::event(e);
}

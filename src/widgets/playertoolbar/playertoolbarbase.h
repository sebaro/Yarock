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

#ifndef _PLAYER_TOOLBAR_BASE_H_
#define _PLAYER_TOOLBAR_BASE_H_


#include "views.h"

// Qt
#include <QWidget>


/*
********************************************************************************
*                                                                              *
*    Class PlayerToolBarBase                                                   *
*                                                                              *
********************************************************************************
*/
class PlayerToolBarBase : public QWidget
{
Q_OBJECT
    static PlayerToolBarBase         *TB_INSTANCE;

public:
    PlayerToolBarBase(QWidget *parent);

    static PlayerToolBarBase* instance() { return TB_INSTANCE; }
    
    virtual void setCollectionInfo(QString ,VIEW::Id ) =0;
    
    virtual void fullUpdate()  =0;
    
signals:
    void switchToolBarType();
};

#endif // _PLAYER_TOOLBAR_BASE_H_

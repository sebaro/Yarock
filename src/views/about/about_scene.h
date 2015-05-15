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
#ifndef _ABOUT_SCENE_H_
#define _ABOUT_SCENE_H_

#include "scene_base.h"

#include <QWidget>

class HeaderItem;
/*
********************************************************************************
*                                                                              *
*    Class AboutScene                                                          *
*                                                                              *
********************************************************************************
*/
class AboutScene : public SceneBase
{
Q_OBJECT
  public:
    AboutScene(QWidget* parent);

    void setData(const QVariant&) {};
    void setSearch(const QVariant&) {};
    void playSceneContents(const QVariant&) {};
    
  /* Basic Scene virtual */      
  public slots:
    void populateScene();
    void resizeScene();
    void initScene();
};


#endif // _ABOUT_SCENE_H_

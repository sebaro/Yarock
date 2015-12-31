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

#ifndef _SCENE_BASE_H_
#define _SCENE_BASE_H_

//! local
#include "views.h"

// Qt
#include <QGraphicsScene>
#include <QMap>
#include <QAction>
#include <QString>
#include <QVariant>
#include <QWidget>

namespace SCENE
{

enum Type {
    LOCAL,
    STREAM,
    CONTEXT,
    FILESYSTEM,
    SETTINGS
  };

} // end namespace

/*
********************************************************************************
*                                                                              *
*    Class SceneBase                                                           *
*                                                                              *
********************************************************************************
*/
class SceneBase : public QGraphicsScene
{
Q_OBJECT
  public:
    SceneBase(QWidget *parent);
    ~SceneBase();

    void setMode(VIEW::Id m) {m_view_mode = m;}
    VIEW::Id mode() {return m_view_mode;}
    
    QWidget* parentView() {return m_parentWidget;}
    
    virtual QList<QAction *> actions() {return QList<QAction *>();}

    bool isInit() { return is_initialised;}
    void setInit(bool b) {is_initialised = b;}

    virtual void setSearch(const QVariant&) = 0;
    virtual void setData(const QVariant&) = 0;
    
    virtual void playSceneContents(const QVariant&)  = 0;
    
  public slots :
    virtual void initScene() = 0;
    virtual void resizeScene() = 0;
    virtual void populateScene() = 0;
    
  private :
    bool                      is_initialised;
    VIEW::Id                  m_view_mode;
    QWidget                  *m_parentWidget;
};

#endif // _SCENE_BASE_H_

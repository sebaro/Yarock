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
#ifndef _SETTINGS_SCENE_H_
#define _SETTINGS_SCENE_H_

#include "scene_base.h"


#include <QGraphicsLinearLayout>
#include <QMap>

namespace SETTINGS {

enum SETTINGS_PAGES {
      GENERAL      = 0,
      PLAYER,
      LIBRARY,
      SHORTCUT,
      SCROBBLER,
      SONGINFO
      };

struct Results {
      bool    isSystrayChanged;
      bool    isDbusChanged;
      bool    isMprisChanged;

      bool    isEngineChanged;
      bool    isLibraryChanged;
      bool    isCoverSizeChanged;
      bool    isViewChanged;
      
      bool    isShorcutChanged;
      bool    isScrobblerChanged;
     };

};


/*
********************************************************************************
*                                                                              *
*    Class SettingsScene                                                       *
*                                                                              *
********************************************************************************
*/
class SettingsScene : public SceneBase
{
Q_OBJECT
  public:
    SettingsScene(QWidget* parent);

    void setSearch(const QVariant&) {};
    void setData(const QVariant&);
    void playSceneContents(const QVariant&) {};
    
    void restore_settings();

    const SETTINGS::Results results() {return m_result;}

  /* Basic Scene virtual */      
  public slots:
    void populateScene();
    void resizeScene();
    void initScene();
    
  private slots:
    void slot_apply_settings();
    void slot_cancel_settings();
    void slot_dbBuilder_stateChange();
    void slot_database_settingsChanged();
    
  private :
    QGraphicsLinearLayout        *m_layout;
    QGraphicsWidget              *m_container;

    SETTINGS::Results             m_result;

    QMap<SETTINGS::SETTINGS_PAGES, QGraphicsWidget*>  m_pages;

  signals:
    void settings_saved();
};


#endif // _SETTINGS_SCENE_H_

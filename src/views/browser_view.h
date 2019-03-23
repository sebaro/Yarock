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
#ifndef _BROWSER_VIEW_H_
#define _BROWSER_VIEW_H_

#include "scene_base.h"
#include "views.h"
#include "threadmanager.h"
#include "mediaitem.h"

#include "views/settings/settings_scene.h"

#include <QGraphicsView>
#include <QHash>
#include <QMap>
#include <QString>
#include <QModelIndex>
#include <QMenu>

class LocalScene;
class StreamScene;
class ContextScene;
class SettingsScene;
class FileScene;

/*
********************************************************************************
*                                                                              *
*    BrowserView                                                               *
*                                                                              *
********************************************************************************
*/
class BrowserView : public QGraphicsView
{
Q_OBJECT
public:
    BrowserView(QWidget *parent);
    
    void setLocalScene(LocalScene *scene);
    void setStreamScene(StreamScene *scene);
    void active_view(VIEW::Id, QString, QVariant);
    SETTINGS::Results settingsResults();
    bool playSelectedItems();

public slots:
    void restore_view();
    void save_view();

protected:
    bool eventFilter(QObject *obj, QEvent *ev);
    void resizeEvent ( QResizeEvent * event );
    void contextMenuEvent ( QContextMenuEvent * event );
    void keyPressEvent ( QKeyEvent * event );
    
private slots:
    void slot_on_search_changed(const QVariant& );
    void slot_on_model_populated(E_MODEL_TYPE);
    void slot_jump_to_media();
    void slot_check_slider(int);

    void slot_on_tag_clicked();
    void slot_on_menu_browser_triggered(VIEW::Id view, QVariant data);
    void slot_on_load_new_data(QVariant);    
    void slot_on_history_next_activated();
    void slot_on_history_prev_activated();

private:
    QString collectionInfo();
    QString name_for_view(VIEW::Id id);
    void update_statuswidget();
    void jump_to_media(MEDIA::MediaPtr media);

signals:
    void settings_saved();

  
private :
    struct BrowserParam
    {
      BrowserParam() {
        scroll = 0;
      }
      BrowserParam(VIEW::Id m,const QVariant& s, const QVariant& d)
      {
        mode   = m;
        search = s;
        data   = d;
        scroll = 0;
      }

      VIEW::Id      mode;
      QVariant      search; 
      QVariant      data;
      int           scroll;
    };
    void switch_view(BrowserParam& param);
    void add_history_entry(BrowserParam& param);
    
    /* prev/next navigation */
    QList<BrowserParam>   m_browser_params;
    int                   m_browser_params_idx;
    
    /* scene */
    QMap<VIEW::Id,SceneBase*>   m_scenes;

    /* scroll position */
    QScrollBar           *m_scrollbar;
    QMap<VIEW::Id, int>  scrolls;
   
    bool is_started;
    
    /* context menu */
    QMenu      *m_menu;
};

#endif // _BROWSER_VIEW_H_

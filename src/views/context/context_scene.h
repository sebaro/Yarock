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

#ifndef _CONTEXT_SCENE_H_
#define _CONTEXT_SCENE_H_

#include "scene_base.h"

#include "context_widget.h"
#include "info_system.h"
#include "mediaitem.h"
#include "views/item_common.h"


#include <QWidget>
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsScene>
#include <QHash>

class EngineBase;
/*
********************************************************************************
*                                                                              *
*    Class ContextScene                                                        *
*                                                                              *
********************************************************************************
*/
class ContextScene : public SceneBase
{
Q_OBJECT
  public:
    ContextScene(QWidget* parent);

    void setSearch(const QVariant&) {};
    void setData(const QVariant&) {};
    void playSceneContents(const QVariant&) {};
    
  /* Basic Scene virtual */      
  public slots:
    void populateScene();
    void resizeScene();
    void initScene();
    
  private:
    enum Mode
    {
        TrackPlay = 0,
        StreamPlayNoTrack,
        StreamPlay,
        Stopped
    };
    
    void init_widget(Mode mode);
    
  private slots:
    void slot_filter_media_change();
    void slot_filter_enginestate_change();

    void slot_update_draw();
    void infoSystemInfo(INFO::InfoRequestData request, QVariant output );
   
  private :
    Mode                       m_mode;

    /*   player  and states  */
    EngineBase                *m_engine_player;
    QHash< QString, QString >  m_metadata;
    
    QList<quint64>             m_requests_ids;
    
    /*    graphics items     */
    QGraphicsLinearLayout     *m_layout;
    QGraphicsWidget           *m_container;
    QGraphicsWidget           *m_container_now_playing;

    ArtistInfoWidget          *m_artist_info_widget;
    ArtistSimilarWidget       *m_similar_info_widget;
    DiscoInfoWidget           *m_disco_info_widget;
    LyricsInfoWidget          *m_lyrics_info_widget;
    
    InfoLayoutItem            *m_info_widget;
};

#endif // _CONTEXT_SCENE_H_

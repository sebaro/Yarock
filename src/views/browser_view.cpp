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

#include "browser_view.h"

/* scene */
#include "views/local/local_scene.h"
#include "views/stream/stream_scene.h"
#include "views/context/context_scene.h"
#include "views/filesystem/file_scene.h"
#include "views/settings/settings_scene.h"
#include "views/about/about_scene.h"

/* core */
#include "threadmanager.h"
#include "engine.h"
#include "media_search.h"

/* widgets */
#include "widgets/playertoolbar/playertoolbarbase.h"
#include "widgets/main/menumodel.h"
#include "widgets/main/main_left.h"

#include "settings.h"
#include "global_actions.h"
#include "debug.h"

#include <QScrollBar>
#include <QApplication>
/*
********************************************************************************
*                                                                              *
*    BrowserView                                                               *
*                                                                              *
********************************************************************************
*/
BrowserView::BrowserView(QWidget *parent) : QGraphicsView(parent)
{
    /* widget setup */
    this->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    this->setAutoFillBackground(false);

    /* GraphicView Setup */
    this->setFrameShape(QFrame::NoFrame);
    this->setFrameShadow(QFrame::Plain);

    this->setCacheMode(QGraphicsView::CacheNone);

    this->setRenderHint(QPainter::Antialiasing);
    this->setOptimizationFlags(  QGraphicsView::DontAdjustForAntialiasing
                                   | QGraphicsView::DontClipPainter
                                   | QGraphicsView::DontSavePainterState);
    this->setResizeAnchor(QGraphicsView::NoAnchor);
    this->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    this->setAlignment(Qt::AlignLeft | Qt::AlignTop );

    this->setDragMode(QGraphicsView::NoDrag); 
    
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    is_started = false;
    
    /* scroll bar */
    m_scrollbar = this->verticalScrollBar();
    m_scrollbar->setSingleStep(10);
    m_scrollbar->setPageStep(30);
    m_scrollbar->setContextMenuPolicy(Qt::NoContextMenu);
    m_scrollbar->installEventFilter(this);
    m_scrollbar->setMinimum( 0 );

    /* Build scene */  
      /* filesystem scene */    
      m_scenes.insert(VIEW::ViewFileSystem,  new FileScene(this));
      connect(static_cast<FileScene*>(m_scenes[VIEW::ViewFileSystem]), SIGNAL(load_directory(QVariant)), this, SLOT(slot_on_load_new_data(QVariant)));

      /* about scene */    
      m_scenes.insert(VIEW::ViewAbout,  new AboutScene(this));
    
      /* settings */
      m_scenes.insert(VIEW::ViewSettings,  new SettingsScene(this));
      connect(static_cast<SettingsScene*>(m_scenes[VIEW::ViewSettings]), SIGNAL(settings_saved()), this, SIGNAL(settings_saved()));

      /* context */
      m_scenes.insert(VIEW::ViewContext,  new ContextScene(this) );      
 
      /* stream scene */
      m_scenes.insert(VIEW::ViewDirble,         new StreamScene(this) );
      m_scenes.insert(VIEW::ViewRadionomy,      m_scenes[VIEW::ViewDirble] );
      m_scenes.insert(VIEW::ViewTuneIn,         m_scenes[VIEW::ViewDirble] );
      m_scenes.insert(VIEW::ViewFavoriteRadio,  m_scenes[VIEW::ViewDirble] );

      connect(static_cast<StreamScene*>(m_scenes[VIEW::ViewDirble]), SIGNAL(linked_changed(QVariant)), this, SLOT(slot_on_load_new_data(QVariant)));
      
      /* local scene */
      m_scenes.insert(VIEW::ViewArtist,         new LocalScene(this) );
      m_scenes.insert(VIEW::ViewAlbum,          m_scenes[VIEW::ViewArtist] );
      m_scenes.insert(VIEW::ViewTrack,          m_scenes[VIEW::ViewArtist] );
      m_scenes.insert(VIEW::ViewGenre,          m_scenes[VIEW::ViewArtist] );
      m_scenes.insert(VIEW::ViewYear,           m_scenes[VIEW::ViewArtist] );
      m_scenes.insert(VIEW::ViewDashBoard,      m_scenes[VIEW::ViewArtist] );
      m_scenes.insert(VIEW::ViewHistory,        m_scenes[VIEW::ViewArtist] );
      m_scenes.insert(VIEW::ViewPlaylist,       m_scenes[VIEW::ViewArtist] );
      m_scenes.insert(VIEW::ViewSmartPlaylist,  m_scenes[VIEW::ViewArtist] );
      m_scenes.insert(VIEW::ViewFavorite,       m_scenes[VIEW::ViewArtist] );
      connect(static_cast<LocalScene*>(m_scenes[VIEW::ViewArtist]), SIGNAL(linked_changed(QVariant)), this, SLOT(slot_on_load_new_data(QVariant)));
    
    /* connections */
    connect(MainLeftWidget::instance(), SIGNAL(browser_search_change(const QVariant&)),this, SLOT(slot_on_search_changed(const QVariant&)));
    connect(ThreadManager::instance(),  SIGNAL(modelPopulationFinished(E_MODEL_TYPE)), this, SLOT(slot_on_model_populated(E_MODEL_TYPE)));
    
    
    connect(ACTIONS()->value(BROWSER_PREV), SIGNAL(triggered()), this, SLOT(slot_on_history_prev_activated()));
    connect(ACTIONS()->value(BROWSER_NEXT), SIGNAL(triggered()), this, SLOT(slot_on_history_next_activated()));

    connect(MenuModel::instance(),SIGNAL(menu_browser_triggered(VIEW::Id, QVariant)),this, SLOT(slot_on_menu_browser_triggered(VIEW::Id, QVariant)));
    connect(ACTIONS()->value(TAG_CLICKED), SIGNAL(triggered()), this, SLOT(slot_on_tag_clicked()));
    
    connect (m_scrollbar, SIGNAL(actionTriggered(int)), this, SLOT(slot_check_slider(int)));
      
    connect(ACTIONS()->value(BROWSER_JUMP_TO_ARTIST), SIGNAL(triggered()), SLOT(slot_jump_to_media()));
    connect(ACTIONS()->value(BROWSER_JUMP_TO_ALBUM), SIGNAL(triggered()), SLOT(slot_jump_to_media()));
    connect(ACTIONS()->value(BROWSER_JUMP_TO_TRACK), SIGNAL(triggered()), SLOT(slot_jump_to_media()));
    connect(ACTIONS()->value(BROWSER_JUMP_TO_MEDIA), SIGNAL(triggered()), SLOT(slot_jump_to_media()));
    
    /* initialization        */  
    m_browser_params_idx = -1;    
    m_menu = 0;
}

/*******************************************************************************
   playSelectedItems
*******************************************************************************/
bool BrowserView::playSelectedItems()
{
    LocalScene* localscene = static_cast<LocalScene*>(m_scenes[VIEW::ViewArtist]);
    StreamScene* streamscene = static_cast<StreamScene*>(m_scenes[VIEW::ViewDirble]);
  
    if(!localscene->selectedItems().isEmpty()) {
      localscene->playSelected();
      return true;
    }
        
    if (!streamscene->selectedItems().isEmpty()) {
      streamscene->playSelected();
      return true;
    }  

    return false;
}

/*******************************************************************************
   settingsResults
*******************************************************************************/
SETTINGS::Results BrowserView::settingsResults()
{
    return static_cast<SettingsScene*>(m_scenes[VIEW::ViewSettings])->results();
}

/*******************************************************************************
   Save / Restore settings
*******************************************************************************/
void BrowserView::restore_view()
{
   Debug::debug() << "  [BrowserView] restore_view";
  
   BrowserParam param;

   param.mode    = VIEW::Id(SETTINGS()->_viewMode);
   param.scroll  = SETTINGS()->_browserScroll;
   param.search  = QVariant();
   
   if(param.mode == VIEW::ViewFileSystem)
     param.data   = QVariant( SETTINGS()->_filesystem_path );
   else
     param.data   = QVariant();

   add_history_entry(param);

   switch_view(param);   
  
   is_started = true;
}

void BrowserView::save_view()
{
    Debug::debug() << "  [BrowserView] save_view  ";
  
    SETTINGS()->_browserScroll = m_scrollbar->sliderPosition();
}

/*******************************************************************************
   Changing view slots 
*******************************************************************************/
/* slot triggered when use enter the search field */
void BrowserView::slot_on_search_changed(const QVariant& variant)
{           
    Debug::debug() << "  [BrowserView] slot_on_search_changed  ";

    BrowserParam current_param;
    if(m_browser_params_idx != -1)
      current_param = m_browser_params.at(m_browser_params_idx);

  
    BrowserParam param = BrowserParam(
           VIEW::Id(SETTINGS()->_viewMode),
           variant,
           current_param.data);
    
    add_history_entry(param);

    switch_view(param);
}

/* slot triggered when action from menu is activated */
void BrowserView::slot_on_menu_browser_triggered(VIEW::Id view, QVariant data)
{
    Debug::debug() << "  [BrowserView] slot_on_menu_browser_triggered data";
  
    QVariant search = MainLeftWidget::instance()->browserSearch();

    /* manage search case */
    if( (typeForView(view) != VIEW::LOCAL) || (view == VIEW::ViewSmartPlaylist) )
    {
      /* reset the advance search if the new mode can not support advance search */
      if(search.canConvert<MediaSearch>())
        search = QVariant();
    }
       
    BrowserParam param = BrowserParam(view,search,data);
    
    add_history_entry(param);

    switch_view(param);
}

/* slot slot_on_load_new_data                              */
/*    -> used by FileScene to load a directory inside view */
/*    -> used by StreamScene on new link                   */
void BrowserView::slot_on_load_new_data(const QVariant data)
{
    Debug::debug() << "  [BrowserView] slot_on_load_new_data";

    MEDIA::LinkPtr link = qvariant_cast<MEDIA::LinkPtr>(data);
    
    BrowserParam param = BrowserParam(
          VIEW::Id(SETTINGS()->_viewMode),
          MainLeftWidget::instance()->browserSearch(),
          data);
    
    /* hack because stream service notify every state change */
    /* no need to record all state, i.e DOWNLOADING state    */
    if( data.canConvert<QString>() || (qvariant_cast<MEDIA::LinkPtr>(data))->state == SERVICE::DATA_OK )
      add_history_entry( param );
            
    switch_view(param);
}


void BrowserView::slot_on_tag_clicked()
{
    Debug::debug() << "  [BrowserView] slot_on_tag_clicked";
    BrowserParam current_param;
    if(m_browser_params_idx != -1)
      current_param = m_browser_params.at(m_browser_params_idx);

  
    BrowserParam param = BrowserParam(
           VIEW::ViewGenre,
           ACTIONS()->value(TAG_CLICKED)->data(),
           QVariant());
    
    add_history_entry(param);

    switch_view(param);
}


void BrowserView::active_view(VIEW::Id m, QString f, QVariant d)
{
    BrowserParam param = BrowserParam(m, f, d);

    add_history_entry( param );
    
    switch_view(param);
}

    
/*******************************************************************************
    slot_on_model_populated
*******************************************************************************/
/* slot used by model to notify update  */
void BrowserView::slot_on_model_populated(E_MODEL_TYPE model)
{
    if(!is_started) return;
    Debug::debug() << "  [BrowserView] slot_on_model_populated ";
    //! no need to update graphics view if the viewmode is not impacted
    BrowserParam param = m_browser_params.at(m_browser_params_idx);
    
    switch(param.mode)
    {
      case VIEW::ViewAlbum     :
      case VIEW::ViewArtist    :
      case VIEW::ViewTrack     :
      case VIEW::ViewGenre     :
      case VIEW::ViewYear      :
      case VIEW::ViewFavorite  :
      case VIEW::ViewDashBoard : 
      case VIEW::ViewHistory   : 
        if(model == MODEL_COLLECTION)
          switch_view(param);
      break;

      case VIEW::ViewPlaylist      :
      case VIEW::ViewSmartPlaylist :
        if(model == MODEL_PLAYLIST)
         switch_view(param);
      break;
      default : break;
    }
}

/*******************************************************************************
    resizeEvent
*******************************************************************************/
void BrowserView::resizeEvent( QResizeEvent * event)
{
    Debug::debug() << "  [BrowserView] resizeEvent";
    if(!is_started) return;
    
    SceneBase * scene = m_scenes.value( VIEW::Id(SETTINGS()->_viewMode) );
    if( scene->isInit() )
      scene->resizeScene();
     
    event->accept();
}

/*******************************************************************************
    eventFilter
*******************************************************************************/
bool BrowserView::eventFilter(QObject *obj, QEvent *ev)
{
    if(!is_started) return false;

    //Debug::debug() << "BrowserView eventFilter  obj" << obj;
    int type = ev->type();
    QWidget *wid = qobject_cast<QWidget*>(obj);

    if (obj == this)
    {
        return false;
    }

    if (wid && (wid == m_scrollbar ))
    {
      if(type == QEvent::Hide || type == QEvent::Show) 
      {
        SceneBase * scene = m_scenes.value( VIEW::Id(SETTINGS()->_viewMode) );
        if( scene->isInit() )
          scene->resizeScene();
      }
    }

    return QWidget::eventFilter(obj, ev);
}

    
    
/*******************************************************************************
    switch_view
*******************************************************************************/
void BrowserView::switch_view(BrowserParam& param)
{
    //Debug::debug() << "  [BrowserView] switch_view param.mode " << param.mode;
    //Debug::debug() << "  [BrowserView] switch_view param.filter" << param.filter;
    //Debug::debug() << "  [BrowserView] switch_view param.data" << param.data;
    Debug::debug() << "  [BrowserView] switch_view";

    /* save current state */
    SETTINGS()->_viewMode = param.mode;
    
    /* handle scene switch view */
    SceneBase * scene = m_scenes[ param.mode ];

    if( !scene->isInit() )
      scene->initScene();
          
    scene->setMode( param.mode );
    scene->setSearch( param.search );
    scene->setData( param.data );
    scene->populateScene();
    scene->playSceneContents( param.search );
    this->setFocus();

    switch( VIEW::typeForView(VIEW::Id(param.mode)) )
    {
      case VIEW::LOCAL      : setScene(static_cast<LocalScene*>(m_scenes[param.mode]));    break;
      case VIEW::RADIO      : setScene(static_cast<StreamScene*>(m_scenes[param.mode]));   break;
      case VIEW::CONTEXT    : setScene(static_cast<ContextScene*>(m_scenes[param.mode]));  break;
      case VIEW::SETTING    : setScene(static_cast<SettingsScene*>(m_scenes[param.mode])); break;
      case VIEW::FILESYSTEM : setScene(static_cast<FileScene*>(m_scenes[param.mode]));     break;
      case VIEW::ABOUT      : setScene(static_cast<AboutScene*>(m_scenes[param.mode]));    break;
    }

    /* restore scroll position */
    m_scrollbar->setSliderPosition(param.scroll);

    /* update page title */    
    PlayerToolBarBase::instance()->setCollectionInfo( collectionInfo() , VIEW::Id(param.mode));
    MainLeftWidget::instance()->setMode(param.mode);
    MainLeftWidget::instance()->setBrowserSearch(param.search);
    MainLeftWidget::instance()->setTitle( name_for_view(VIEW::Id(param.mode)) );
}

    
/*******************************************************************************
    slot_jump_to_media
*******************************************************************************/
void BrowserView::slot_jump_to_media()
{
    //Debug::debug() << "  [BrowserView] slot_jump_to_media";
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action) return;

    MEDIA::MediaPtr media;
    if(action == ACTIONS()->value(BROWSER_JUMP_TO_MEDIA))
    {
      media = qvariant_cast<MEDIA::MediaPtr>( (ACTIONS()->value(BROWSER_JUMP_TO_MEDIA))->data() );
      jump_to_media(media);
    }
    else 
    {
      media = Engine::instance()->playingTrack();
      
      if(!media) return;
      
      if(!media->parent()) return;
      
      MEDIA::MediaPtr album = media->parent();      
      MEDIA::MediaPtr artist = album->parent();
      

      if(action == ACTIONS()->value(BROWSER_JUMP_TO_TRACK))
        jump_to_media(media);
      else if(action == ACTIONS()->value(BROWSER_JUMP_TO_ALBUM))
        jump_to_media(album);      
      else if(action == ACTIONS()->value(BROWSER_JUMP_TO_ARTIST))
        jump_to_media(artist);
    }
}


void BrowserView::jump_to_media(MEDIA::MediaPtr media)
{
    VIEW::Id mode;

    if(!media)
      return;
    
    if(media->type() == TYPE_ARTIST)
      mode = VIEW::ViewArtist;
    else if(media->type() == TYPE_ALBUM)
      mode = VIEW::ViewAlbum;
    else if(media->type() == TYPE_TRACK)
      mode = VIEW::ViewTrack;
    else return;  
      
    MainLeftWidget::instance()->setBrowserSearch(QVariant());
    
    m_scenes[VIEW::ViewArtist]->setSearch(QVariant());
    m_scenes[VIEW::ViewTuneIn]->setSearch(QVariant());
    m_scenes[VIEW::ViewFileSystem]->setSearch(QVariant());
      
    active_view(mode,"", QVariant());

    /* localise item */
    QPoint p = static_cast<LocalScene*>(m_scenes[mode])->get_item_position(media);

    if(!p.isNull()) 
      m_scrollbar->setSliderPosition( p.y() - 40 );
}


/*******************************************************************************
    Browser History 
*******************************************************************************/
void BrowserView::add_history_entry(BrowserParam& param)
{
    //Debug::debug() << "  [BrowserView] add_history_entry";

    /* save scroll position */
    if(m_browser_params_idx == 0) 
    {
      BrowserParam current_param = m_browser_params[0];
      current_param.scroll = m_scrollbar->sliderPosition();
      scrolls[ current_param.mode ] = current_param.scroll;

      if(scrolls.contains(param.mode))
        param.scroll = scrolls.value( param.mode );      
    }
    
    /* nouveau déclenchement --> on supprime les next */
    for(int i = 0; i < m_browser_params_idx; i++)
      m_browser_params.removeAt(i);

    m_browser_params.prepend(param);
    m_browser_params_idx = 0;

    /* limite de la taille de l'historique de navigation */
    if(m_browser_params.size() >= 30 && m_browser_params_idx != m_browser_params.size() -1)
      m_browser_params.takeLast();
    
    ACTIONS()->value(BROWSER_PREV)->setEnabled(m_browser_params_idx < m_browser_params.size() -1);
    ACTIONS()->value(BROWSER_NEXT)->setEnabled(m_browser_params_idx > 0);    
} 


void BrowserView::slot_on_history_prev_activated()
{
    if(m_browser_params_idx < m_browser_params.size() -1)
    {
      m_browser_params_idx++;

      ACTIONS()->value(BROWSER_PREV)->setEnabled(m_browser_params_idx < m_browser_params.size() -1);
      ACTIONS()->value(BROWSER_NEXT)->setEnabled(m_browser_params_idx > 0);

      BrowserParam param = m_browser_params.at(m_browser_params_idx);
      switch_view(param);
    }
}


void BrowserView::slot_on_history_next_activated()
{
    if(m_browser_params_idx > 0)
    {
      m_browser_params_idx--;

      ACTIONS()->value(BROWSER_PREV)->setEnabled(m_browser_params_idx < m_browser_params.size() -1);
      ACTIONS()->value(BROWSER_NEXT)->setEnabled(m_browser_params_idx > 0);

      BrowserParam param = m_browser_params.at(m_browser_params_idx);
      switch_view(param);      
    }
}



/*******************************************************************************
    contextMenuEvent
*******************************************************************************/
void BrowserView::contextMenuEvent ( QContextMenuEvent * event )
{
    //Debug::debug() << "  [BrowserView] contextMenuEvent ";
    SceneBase * scene = m_scenes[ VIEW::Id(SETTINGS()->_viewMode) ];
    
    if(scene->mouseGrabberItem()) 
    {
        QGraphicsView::contextMenuEvent(event) ;
    }
    else 
    {
       if(!m_menu) 
       {
         m_menu = new QMenu(this);
         
         m_menu->setStyleSheet(
             QString("QMenu {icon-size: 32px; background-color: none;border: none;}"
                     "QMenu::item {background-color: none;}") );         
       }
       
       m_menu->clear();
       m_menu->addActions(scene->actions());
       m_menu->addSeparator();
       m_menu->addAction(ACTIONS()->value(BROWSER_JUMP_TO_ARTIST));
       m_menu->addAction(ACTIONS()->value(BROWSER_JUMP_TO_ALBUM));
       m_menu->addAction(ACTIONS()->value(BROWSER_JUMP_TO_TRACK));       
       
       m_menu->popup(mapToGlobal(event->pos()));
       
       event->accept();
    }
}


/*******************************************************************************
    keyPressEvent
*******************************************************************************/
void BrowserView::keyPressEvent ( QKeyEvent * event )
{
    //Debug::debug() << "  [BrowserView] keyPressEvent " << event->key();
    switch(event->key())
    {
      case Qt::Key_Home  :
        if( Qt::ControlModifier == QApplication::keyboardModifiers() ) {
            this->verticalScrollBar()->setSliderPosition(0);
            event->accept();
        }
        break;

      case Qt::Key_End  :
        if( Qt::ControlModifier == QApplication::keyboardModifiers() ) {
            this->verticalScrollBar()->setSliderPosition(this->verticalScrollBar()->maximum());
            event->accept();
        }
        break;

      case Qt::Key_Left  :
      case Qt::Key_Right :
        event->ignore();
        break;      
      
      default : QGraphicsView::keyPressEvent(event);break;
    }
}

/*******************************************************************************
    collectionInfo
*******************************************************************************/
QString BrowserView::collectionInfo() 
{
    const int collectionSize = static_cast<LocalScene*>(m_scenes[VIEW::ViewArtist])->elementCount();
    const int radioSize      = static_cast<StreamScene*>(m_scenes[VIEW::ViewTuneIn])->elementCount();

    QString     text;

    switch( SETTINGS()->_viewMode )
    {
      case (VIEW::ViewHistory)    : text = QString(tr("History")); break;
      case (VIEW::ViewDashBoard)   : text = QString(tr("Dashboard")); break;
      case (VIEW::ViewSettings)   : text = QString(tr("Settings")); break;
      
      case (VIEW::ViewAlbum)    : text = QString(tr("Collection : <b>%1</b> albums")).arg(QString::number(collectionSize)); break;
      case (VIEW::ViewArtist)   : text = QString(tr("Collection : <b>%1</b> artist")).arg(QString::number(collectionSize));break;
      case (VIEW::ViewTrack)    : text = QString(tr("Collection : <b>%1</b> tracks")).arg(QString::number(collectionSize));break;
      case (VIEW::ViewGenre)    : text = QString(tr("Collection : <b>%1</b> styles")).arg(QString::number(collectionSize));break;
      case (VIEW::ViewYear)     : text = QString(tr("Collection : <b>%1</b> years")).arg(QString::number(collectionSize));break;
      case (VIEW::ViewFavorite) : text = QString(tr("Collection : <b>%1</b> favorite item")).arg(QString::number(collectionSize));break;

      case (VIEW::ViewPlaylist) : 
      case (VIEW::ViewSmartPlaylist) :
        text = QString(tr("Playlist : <b>%1</b> playlists")).arg(QString::number(collectionSize));
        break;
      
      case (VIEW::ViewDirble)         :
      case (VIEW::ViewRadionomy)      :
      case (VIEW::ViewTuneIn)         :
      case (VIEW::ViewFavoriteRadio)  :
         text = QString(tr("Radio : <b>%1</b> streams")).arg(QString::number(radioSize));
         break;
      default: text = "";break;
    }
    
    return text;
}


/*******************************************************************************
    name_for_view
*******************************************************************************/
QString BrowserView::name_for_view(VIEW::Id id)
{
    QString title;
    switch( id )
    {
      case VIEW::ViewAbout             : title = tr("About");  break;
      case VIEW::ViewSettings          : title = tr("Settings");  break;
      case VIEW::ViewDashBoard         : title = tr("Dashboard");  break;
      case VIEW::ViewContext           : title = tr("Context");  break;
      case VIEW::ViewHistory           : title = tr("History");  break;
      case VIEW::ViewArtist            : title = tr("Artists");  break;
      case VIEW::ViewAlbum             : title = tr("Albums");  break;
      case VIEW::ViewTrack             : title = tr("Tracks");  break;
      case VIEW::ViewGenre             : title = tr("Genres");  break;
      case VIEW::ViewYear              : title = tr("Years");  break;
      case VIEW::ViewFavorite          : title = tr("Favorites");  break;
      case VIEW::ViewPlaylist          : title = tr("Playlists");  break;
      case VIEW::ViewSmartPlaylist     : title = tr("Smart playlists");  break;
      case VIEW::ViewDirble            : title = ("Dirble");  break;
      case VIEW::ViewRadionomy         : title = ("Radionomy");  break;
      case VIEW::ViewTuneIn            : title = ("TuneIn");  break;
      case VIEW::ViewFavoriteRadio     : title = tr("Favorites radios");  break;
      case VIEW::ViewFileSystem        : title = tr("Filesystem");  break;
      default:break;
    }     
    return title;
}


/*******************************************************************************
    slot_check_slider
*******************************************************************************/
void BrowserView::slot_check_slider(int action)
{
    //Debug::debug() << "  [BrowserView]slot_showContextMenu slot_check_slider";
    if( action == QAbstractSlider::SliderPageStepAdd ||
        action == QAbstractSlider::SliderPageStepSub )
    {
        
      QList<QGraphicsItem*> categories;
      foreach(QGraphicsItem* gItem, this->scene()->items() )
        if(gItem->type() == GraphicsItem::CategoryType)
          categories << gItem;

      if( categories.isEmpty() ) return;
        
      int top = m_scrollbar->sliderPosition();
                  
      /* ------------ navigate go up ---------------- */
      if( action == QAbstractSlider::SliderPageStepSub) 
      {      
          for(int i=categories.size()-1; i>=0;i--)
          {
            QPoint itemPoint = categories.at(i)->scenePos().toPoint();
    
            if(top > itemPoint.y()) {
              m_scrollbar->setSliderPosition(itemPoint.y());
              break;
            }
          }
      }
      /* ------------ navigate go down ---------------*/      
      else
      {
          foreach (QGraphicsItem *item, categories)
          {
            QPoint itemPoint = (item->scenePos()).toPoint();

            if(top < itemPoint.y()) {
              m_scrollbar->setSliderPosition(itemPoint.y());
              break;
            }
          }      
      }
    }
}

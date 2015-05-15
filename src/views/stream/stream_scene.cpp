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

#include "stream_scene.h"
#include "stream_loader.h"

#include "views/stream/stream_item.h"
#include "views/item_common.h"
#include "views/item_button.h"

#include "core/player/engine.h"

#include "models/stream/stream_model.h"
#include "models/stream/service_dirble.h"
#include "models/stream/service_shoutcast.h"
#include "models/stream/service_tunein.h"
#include "models/stream/service_xspf.h"

#include "playqueue/virtual_playqueue.h"
#include "playqueue/playqueue_model.h"
#include "playqueue/task_manager.h"

#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/playlist_parser.h"
#include "networkaccess.h"
#include "utilities.h"

/* widget */
#include "widgets/exlineedit.h"
#include "widgets/dialogs/addstreamdialog.h"
#include "dialogs/filedialog.h"
#include "widgets/statuswidget.h"
#include "views/item_button.h"

#include "global_actions.h"
#include "settings.h"
#include "debug.h"

#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QGraphicsWidget>
#include <QDrag>

/*
********************************************************************************
*                                                                              *
*    Class StreamScene                                                         *
*                                                                              *
********************************************************************************
*/
StreamScene::StreamScene(QWidget *parent) : SceneBase(parent)
{
    this->setStickyFocus(false);
    
    /* init model */
    m_model = StreamModel::instance();
    
    connect(VirtualPlayqueue::instance(), SIGNAL(signal_playing_status_change()),  this, SLOT(update()));
}


/*******************************************************************************
    initScene
*******************************************************************************/
void StreamScene::initScene()
{
    /* init player */
    _player = Engine::instance();

    /* init stream services */
    m_services.insert(VIEW::ViewDirble,         new Dirble() );
    m_services.insert(VIEW::ViewShoutCast,      new ShoutCast() );
    m_services.insert(VIEW::ViewTuneIn,         new TuneIn() );
    m_services.insert(VIEW::ViewFavoriteRadio,  new XspfStreams() );

    foreach (Service* service, m_services.values() ) {
      connect(service, SIGNAL(stateChanged()), this, SLOT(populateScene()));      
      connect(service, SIGNAL(dataChanged()), this, SLOT(update()));
    }    
   
    /*  graphic item context menu */
    m_graphic_item_menu = new GraphicsItemMenu(0);
    connect(m_graphic_item_menu, SIGNAL(menu_action_triggered(ENUM_ACTION_ITEM_MENU)), this, SLOT(slot_contextmenu_triggered(ENUM_ACTION_ITEM_MENU)), Qt::DirectConnection);

    /*  ressources init */
    m_mouseGrabbedItem    = 0;
    m_infoSize            = 0;

    /*  scene actions */
    m_actions.insert("add_favorite", new QAction(QIcon(":/images/add_32x32.png"), tr("add"),this));
    m_actions.insert("import_favorite", new QAction(QIcon(":/images/add_32x32.png"), tr("import"),this));

    m_actions.insert("reload", new QAction(QIcon(":/images/rebuild.png"),QString("reload"),this));
    connect(m_actions.value("reload"), SIGNAL(triggered()), this, SLOT(slot_reload_stream_list()));
    connect(m_actions.value("add_favorite"), SIGNAL(triggered()), this, SLOT(slot_on_add_stream_clicked()));
    connect(m_actions.value("import_favorite"), SIGNAL(triggered()), this, SLOT(slot_on_import_stream_clicked()));

    /* drag items action */
    ACTIONS()->insert(BROWSER_STREAM_ITEM_MOUSE_MOVE, new QAction(this));
    
    connect(ACTIONS()->value(BROWSER_STREAM_ITEM_MOUSE_MOVE), SIGNAL(triggered()), this, SLOT(slot_item_mouseMove()), Qt::DirectConnection);
 
    /* search line edit */
    ui_search_widget = new QWidget(0);
    ui_search_widget->setPalette(QApplication::palette());
    ui_search_widget->setAttribute(Qt::WA_NoBackground, true);
    ui_search_widget->setAutoFillBackground(true);

    QLabel* label = new QLabel(ui_search_widget);
    label->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    label->setText("search");
    label->setAlignment( Qt::AlignRight | Qt::AlignVCenter);

    ui_line_edit = new QLineEdit(ui_search_widget);
    ui_line_edit->setMinimumWidth(120);
    ui_line_edit->setMaximumWidth(120);
    ui_line_edit->setMinimumHeight(26);
    ui_line_edit->setMaximumHeight(26);

    QHBoxLayout* layout = new QHBoxLayout(ui_search_widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setMargin(0);    
    layout->addWidget(label,Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(ui_line_edit, Qt::AlignRight | Qt::AlignVCenter);

    ui_proxy = this->addWidget(ui_search_widget, Qt::Widget);

    connect(ui_line_edit, SIGNAL(returnPressed()), this, SLOT(slot_on_search_activated()), Qt::QueuedConnection);
    
    ui_search_widget->hide();
    
    /* end init */    
    setInit(true);
}
    
    
/*******************************************************************************
    actions
*******************************************************************************/
QList<QAction *> StreamScene::actions() 
{
    QList<QAction*> list;
    list << m_actions.value("add_favorite") 
         << m_actions.value("import_favorite")
         << m_actions.value("reload");

    return list;
}

/*******************************************************************************
     setSearch
*******************************************************************************/
void StreamScene::setSearch(const QVariant& variant)
{
    m_model->setFilter( variant.toString() );
}

/*******************************************************************************
     resizeScene
*******************************************************************************/
void StreamScene::resizeScene()
{
    //Debug::debug() << "   [StreamScene] resizeScene";   
    update();
}

/*******************************************************************************
    populateScene
*******************************************************************************/
void StreamScene::populateScene()
{
    Debug::debug() << "   [StreamScene] PopulateScene";

    /* clear the scene FIRST */
    ui_search_widget->hide();    
    if(ui_proxy->scene())
      removeItem(ui_proxy);
    clear();
    
    /* update actions */
    m_actions.value("add_favorite")->setEnabled( m_services[mode()]->type() == SERVICE::LOCAL);
    m_actions.value("import_favorite")->setEnabled( m_services[mode()]->type() == SERVICE::LOCAL);
    
    /* handle service state */
    SERVICE::State state = m_services[mode()]->state();
        
    if(state == SERVICE::DATA_OK) 
    {
      m_model->setStreams( m_services[mode()]->streams() );
      populateExtendedStreamScene();
    }
    else if (state == SERVICE::NO_DATA || state == SERVICE::DOWNLOADING) 
    {
      populateWaitingScene(
        QString(QObject::tr("downloading streams from %1....please wait...")).arg(m_services[mode()]->name()) 
        );
      m_services[mode()]->load();
    }
    else if (state == SERVICE::ERROR) 
    {
      populateWaitingScene(
        QString(QObject::tr("error accessing stream from %1 !")).arg(m_services[mode()]->name())
        );
    }
     
    /* update playing item */     
    if(_player->state() == ENGINE::PLAYING)
        m_model->updateStatusOfPlayingItem(_player->playingTrack());

    /* we need to ajust SceneRect */
    setSceneRect ( itemsBoundingRect().adjusted(0, -10, 0, 40) );
}



/*******************************************************************************
    populateExtendedStreamScene
*******************************************************************************/
void StreamScene::populateExtendedStreamScene()
{
    Debug::debug() << "   [StreamScene] populateExtendedStreamScene";
  
    m_infoSize         = 0;
    int Xpos = 20,Ypos = 5;

    const int categoriesHeight = 40;
   
    /*  search field */    
    if(mode() != VIEW::ViewFavoriteRadio) {
      addItem(ui_proxy); 
     
      QWidget *w = qobject_cast<QGraphicsView*> (parentView())->viewport();
      ui_proxy->setPos(w->width() - 20 - ui_proxy->widget()->width(), 2);
      ui_search_widget->show();
      ui_line_edit->setText( m_services[mode()]->searchTerm() );
    }
    
    
    /*--------------------------------------------------*/
    /* link                                             */
    /* -------------------------------------------------*/
    QList<MEDIA::LinkPtr> links = m_services[mode()]->links();
    QString current_category;
    
    foreach(MEDIA::LinkPtr link, links) 
    {
      if(current_category != link->categorie)
      {        
        if(Xpos > 20 ) 
          Ypos = Ypos + 50;
        
        /*  new category */
        current_category = link->categorie;
        CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
        category->m_name = link->categorie;
        category->setPos(0 , Ypos );

        addItem(category);
        Ypos += categoriesHeight;
        Xpos = 20;
      }

      ButtonItem* button = new ButtonItem();
      button->setText(link->name);
      QVariant v;
      v.setValue(static_cast<MEDIA::LinkPtr>(link));
      button->setData(v);
  
      connect(button, SIGNAL(clicked()), m_services[mode()], SLOT(slot_activate_link()));
      
      if(Xpos + button->width() > parentView()->width()-20) {
        Xpos = 20;
        Ypos = Ypos + 30;
      }

      button->setPos(Xpos ,Ypos );
      Xpos = Xpos + button->width() + 20;

      addItem(button);
    } /* end foreach link */
     
    if(Xpos > 20 ) 
      Ypos = Ypos + 50;
     
    /*--------------------------------------------------*/
    /* stream list                                      */
    /* -------------------------------------------------*/
    /* stream model loop */
    for ( int i = 0; i < m_model->itemCount(); i++ )
    {
      if(!m_model->isStreamFiltered(i)) continue;

      if(current_category != m_model->streamAt(i)->categorie)
      {        
        /* new category */
        current_category = m_model->streamAt(i)->categorie;
        CategorieGraphicItem *category = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
        category->m_name = current_category;
        category->setPos(0 , Ypos );

        addItem(category);
        Ypos += categoriesHeight;
      }      

      if(mode() == VIEW::ViewTuneIn)
      {
          StreamGraphicItem_v2 *stream_item = new StreamGraphicItem_v2();
          stream_item->media = m_model->streamAt(i);
          stream_item->media->isFavorite = (mode() == VIEW::ViewFavoriteRadio);
          stream_item->setPos( 30, Ypos );
          stream_item->_width   = parentView()->width()-30-20;
          stream_item->setToolTip(stream_item->media->url);

          addItem(stream_item);
          Ypos += stream_item->height() + 10;
      }
      else 
      {
          StreamGraphicItem *stream_item = new StreamGraphicItem();
          stream_item->media = m_model->streamAt(i);
          stream_item->media->isFavorite = (mode() == VIEW::ViewFavoriteRadio);
          stream_item->setPos( 30, Ypos );
          stream_item->_width   = parentView()->width()-30-20;
          stream_item->setToolTip(stream_item->media->url);

          addItem(stream_item);
          Ypos += stream_item->height();
      }

      m_infoSize++;
    }
    
    if( (m_infoSize == 0 && m_model->itemCount()  != 0) || (this->items().count()  <= 1) ) {
        InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
        info->_text = "no information";
        info->setPos( 0, Ypos );
        addItem(info);
    }
}



void StreamScene::populateWaitingScene(QString text)
{
          
    if(m_services[mode()]->state() == SERVICE::DOWNLOADING) { 
      LoadingGraphicItem *info = new LoadingGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = text;
      info->setPos( 0 , 70);
      addItem(info);
    } 
    else {
      InfoGraphicItem *info = new InfoGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
      info->_text = text;
      info->setPos( 0 , 70);
      addItem(info);
    }
}

/*******************************************************************************
    playStream
*******************************************************************************/
void StreamScene::playStream()
{
    if(!m_mouseGrabbedItem) return;
    StreamGraphicItem *item = static_cast<StreamGraphicItem*>(m_mouseGrabbedItem);

    const QString streamUrl = item->media->url;

    /*--------------------------------------------------*/
    /* urls simple                                      */
    /* -------------------------------------------------*/
    if( MEDIA::isMediaPlayable(streamUrl) )
    {
      Debug::debug() << "   [StreamScene] playStream  we can play it's an url"<< streamUrl;

      VirtualPlayqueue::instance()->addTrackAndPlay(item->media);
      return;
    }
    
    /*--------------------------------------------------*/
    /* playlist remote or shoutcast                     */
    /* -------------------------------------------------*/
    Debug::debug() << "   [StreamScene] playStream  it's a playlist -> download to do" ;
    uint i = StatusWidget::instance()->startProgressMessage(tr("Loading stream"));
    m_messageIds.insert("StreamLoad", i);
    
    StreamLoader* streamloader = new StreamLoader(item->media);
    streamloader->start_asynchronous_download(item->media->url);
    connect(streamloader, SIGNAL(download_done(MEDIA::TrackPtr)), this, SLOT(slot_streams_fetched(MEDIA::TrackPtr)));
}

void StreamScene::slot_streams_fetched(MEDIA::TrackPtr track)
{
    QList<MEDIA::TrackPtr> tracks;
    foreach(MEDIA::MediaPtr media, track->children()) {
      tracks << MEDIA::TrackPtr::staticCast(media);
      Debug::debug() << "   [StreamScene] slot_streams_fetched URL=" << MEDIA::TrackPtr::staticCast(media)->url;
    }
    
    if(!tracks.isEmpty())
      VirtualPlayqueue::instance()->addTracksAndPlayAt(tracks,0);
    
    StreamLoader* loader = qobject_cast<StreamLoader*>(sender());
    if(loader) delete loader;
    
    if (m_messageIds.contains("StreamLoad"))
      StatusWidget::instance()->stopProgressMessage( m_messageIds.take("StreamLoad") );    
}

/*******************************************************************************
    playSelected
*******************************************************************************/
void StreamScene::playSelected()
{
    //Debug::debug() << "   [StreamScene] playSelected";
    QList<MEDIA::TrackPtr> tracks;

    QList<QGraphicsItem*> list = sortedSelectedItem();
    
    if(list.isEmpty())
      return;
          
    QGraphicsItem* gi = list.first();

    if (gi->type() != GraphicsItem::StreamType) 
      return;
    
    StreamGraphicItem *item = static_cast<StreamGraphicItem*>(gi);
    QString streamUrl = item->media->url;

    /*--------------------------------------------------*/
    /* urls simple                                      */
    /* -------------------------------------------------*/
    if( MEDIA::isMediaPlayable(streamUrl) )
    {
        VirtualPlayqueue::instance()->addTrackAndPlay(item->media);
        return;
    }
      
    /*--------------------------------------------------*/
    /* playlist remote or shoutcast                     */
    /* -------------------------------------------------*/
    Debug::debug() << "   [StreamScene] playStream  it's a playlist -> download to do" ;

    uint i = StatusWidget::instance()->startProgressMessage(tr("Loading stream"));
    m_messageIds.insert("StreamLoad", i);
    
    StreamLoader* streamloader = new StreamLoader(item->media);
    streamloader->start_asynchronous_download(item->media->url);
    connect(streamloader, SIGNAL(download_done(MEDIA::TrackPtr)), this, SLOT(slot_streams_fetched(MEDIA::TrackPtr)));
}



/*******************************************************************************
    enqueueSelected
*******************************************************************************/
void StreamScene::enqueueSelected(bool is_new_playqueue)
{
    //Debug::debug() << "   [StreamScene] enqueueSelected";
    QList<MEDIA::TrackPtr> tracks;

    foreach(QGraphicsItem* gi, sortedSelectedItem())
    {
      if (gi->type() == GraphicsItem::StreamType)
      {
        StreamGraphicItem *item = static_cast<StreamGraphicItem*>(gi);
        tracks << item->media;
      }
    } // end foreach

    if(!tracks.isEmpty()) {
      if(is_new_playqueue)
        Playqueue::instance()->clear();
    
      Playqueue::instance()->manager()->playlistAddMediaItems(tracks);
    }
}


/*******************************************************************************
    enqueueStream
*******************************************************************************/
void StreamScene::enqueueStream(bool is_new_playqueue)
{
    if(!m_mouseGrabbedItem) return;

    if(is_new_playqueue)
      Playqueue::instance()->clear();

    StreamGraphicItem *item = static_cast<StreamGraphicItem*>(m_mouseGrabbedItem);

    const QString streamUrl = item->media->url;

    /*--------------------------------------------------*/
    /* urls simple                                      */
    /* -------------------------------------------------*/
    if( MEDIA::isMediaPlayable(streamUrl) )
    {
      Playqueue::instance()->addMediaItem(item->media);
      return;
    }

    /*--------------------------------------------------*/
    /* playlist remote or shoutcast                     */
    /* -------------------------------------------------*/
    // use playlist task manager to load streams into playqueue    
    Playqueue::instance()->manager()->playlistAddFile(streamUrl);
}

/*******************************************************************************
    startStreamsDrag
*******************************************************************************/
void StreamScene::startStreamsDrag(QGraphicsItem* i)
{
    if(!i) // multiple tracks drags
    {
      MediaMimeData* mimedata = new MediaMimeData(SOURCE_COLLECTION);

      foreach(QGraphicsItem* gi, sortedSelectedItem())
      {
        StreamGraphicItem *item = static_cast<StreamGraphicItem *>(gi);
        mimedata->addTrack(item->media);
      }

      QDrag *drag = new QDrag(parentView());
      drag->setMimeData(mimedata);
      drag->exec();
    }
    else // single drag
    {
        StreamGraphicItem *item = static_cast<StreamGraphicItem *>(i);
        item->startDrag(parentView());
    }
}

/*******************************************************************************
    sortedSelectedItem
*******************************************************************************/
QList<QGraphicsItem*> StreamScene::sortedSelectedItem()
{
    QList<QGraphicsItem*> selected = selectedItems();
    int count = selected.count();

    bool sort_ok = false;

    do
    {
      sort_ok = false;
      for(int i =0; i<=count-2; i++)
      {
        QGraphicsItem* gi1 = selected.at(i);
        QGraphicsItem* gi2 = selected.at(i+1);

        QPoint p1 = gi1->scenePos().toPoint();
        QPoint p2 = gi2->scenePos().toPoint();

        bool lessthan;
        if(p1.y() < p2.y())
          lessthan = true;
        else if (p1.y() > p2.y())
          lessthan = false;
        else if (p1.x() < p2.x())
          lessthan = true;
        else
          lessthan = false;

        if(!lessthan) {
          selected.swap(i,i+1);
          sort_ok = true;
        }
      } // end for
    } while(sort_ok);

    return selected;
}

/*******************************************************************************
    contextMenuEvent
*******************************************************************************/
void StreamScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    //Debug::debug() << "   [StreamScene] contextMenuEvent";
    m_mouseGrabbedItem = this->itemAt(event->scenePos(), QTransform());

    if(!m_mouseGrabbedItem) {
      m_graphic_item_menu->hide();
      event->accept();
      return;
    }

    m_graphic_item_menu->setMode(mode());
    
    if(this->selectedItems().isEmpty() || !selectedItems().contains(m_mouseGrabbedItem))
    {
      m_graphic_item_menu->appendItem(m_mouseGrabbedItem);
      m_graphic_item_menu->updateMenu(false);      
    }
    else if (selectedItems().count() == 1)
    {
      m_mouseGrabbedItem = selectedItems().first();
      clearSelection();
      m_graphic_item_menu->appendItem(m_mouseGrabbedItem);
      m_graphic_item_menu->updateMenu(false);
    }    
    else
    {
      m_graphic_item_menu->appendItems(selectedItems());
      m_graphic_item_menu->updateMenu(true);      
    }

    m_graphic_item_menu->exec(event->screenPos());

    // no need to propagate to item
    event->accept();
}

/*******************************************************************************
    slot_contextmenu_triggered
*******************************************************************************/
void StreamScene::slot_contextmenu_triggered(ENUM_ACTION_ITEM_MENU id)
{
    switch(id)
    {
      case STREAM_PLAY         : playStream();            break;
      case STREAM_EDIT         : editStream(); break;
      case STREAM_QUEUE_END    : enqueueStream(false);    break;
      case STREAM_FAVORITE     : updateStreamFavorite();  break;

      case SELECTION_PLAY      : playSelected();break;
      case SELECTION_QUEUE_END : enqueueSelected(false);break;
      default:break;
    }
}

/*******************************************************************************
    slot_item_mouseMove
*******************************************************************************/
void StreamScene::slot_item_mouseMove()
{
    //Debug::debug() << "   [StreamScene] slot_item_mouseMove";

    // single drag and drop
    QGraphicsItem *gItem = qvariant_cast<QGraphicsItem *>( (ACTIONS()->value(BROWSER_STREAM_ITEM_MOUSE_MOVE))->data() );

    if(this->selectedItems().isEmpty() || !selectedItems().contains(gItem))
    {
        startStreamsDrag(gItem);
    }
    // multiple drag and drop
    else
    {
        startStreamsDrag();
    }
}


/*******************************************************************************
    mouseDoubleClickEvent
*******************************************************************************/
void StreamScene::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
{
    //Debug::debug() << "   [StreamScene] mouseDoubleClickEvent";
    m_mouseGrabbedItem = this->itemAt(event->scenePos(), QTransform());

    if(!m_mouseGrabbedItem || m_mouseGrabbedItem->type() != GraphicsItem::StreamType) {
      event->ignore();
      return;
    }

    if(Qt::ControlModifier == QApplication::keyboardModifiers())
    {
      enqueueStream(false);
    }
    else
    {
      playStream();
    }
    // no need to propagate to item
    event->accept();
}

/*******************************************************************************
    keyPressEvent
*******************************************************************************/
void StreamScene::keyPressEvent ( QKeyEvent * keyEvent )
{
//     Debug::debug() << "  [StreamScene] keyPressEvent " << keyEvent->key();

    if((mode() == VIEW::ViewFavoriteRadio) && (keyEvent->key() == Qt::Key_Delete))
    {
       XspfStreams* xspfstream = static_cast<XspfStreams*>(m_services[VIEW::ViewFavoriteRadio]);
      
       foreach(QGraphicsItem* gitem, selectedItems())
       {
          StreamGraphicItem *item = static_cast<StreamGraphicItem*>(gitem);
          xspfstream->updateItem(item->media);
       }

       xspfstream->saveToFile();
       xspfstream->reload();
       
       keyEvent->accept();
       return;
    }
    else if( keyEvent->key() == Qt::Key_Escape ) 
    {
       clearSelection();
       keyEvent->accept();
       return;      
    }
    
    QGraphicsScene::keyPressEvent (keyEvent);
}


/*******************************************************************************
    slot_reload_stream_list
*******************************************************************************/
void StreamScene::slot_reload_stream_list()
{
    m_services[mode()]->reload();
    populateScene();
}

/*******************************************************************************
    slot_on_search_activated
*******************************************************************************/
void StreamScene::slot_on_search_activated()
{
    Debug::debug() << "   [StreamScene] slot_on_search_activated";
    
    QLineEdit* line_edit = qobject_cast<QLineEdit*>(sender());
    
    if (!line_edit) return;  

    QString term = line_edit->text();
    
    m_services[mode()]->setSearchTerm( line_edit->text() );

    if(!term.isEmpty()) 
      m_services[mode()]->slot_activate_link( m_services[mode()]->searchLink() );
    else
      m_services[mode()]->slot_activate_link( m_services[mode()]->rootLink() );
}


/*******************************************************************************
    Manage favorite
*******************************************************************************/
void StreamScene::updateStreamFavorite()
{
    Debug::debug() << "   [StreamScene] updateStreamFavorite";

    if(!m_mouseGrabbedItem) return;
    StreamGraphicItem *item = static_cast<StreamGraphicItem*>(m_mouseGrabbedItem);

    XspfStreams* xspfstream = static_cast<XspfStreams*>(m_services[VIEW::ViewFavoriteRadio]);
    xspfstream->updateItem(item->media);
    xspfstream->saveToFile();
    xspfstream->reload();
}

void StreamScene::slot_on_add_stream_clicked()
{
    Debug::debug() << "   [StreamScene] slot_on_add_stream_clicked";
  
    AddStreamDialog dialog(this->parentView(),true);

    if(dialog.exec() == QDialog::Accepted)
    {
      const QString category = dialog.category();
      const QString name     = dialog.name();
      const QString url      = dialog.url();

      if( !category.isEmpty() && !name.isEmpty() && ! url.isEmpty() )
      {
          MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());
          media->setType(TYPE_STREAM);
          media->id          = -1;
          media->url         = url;
          media->name        = !name.isEmpty() ? name : url ;
          media->categorie   = category;
          media->isFavorite  = false;
          media->isPlaying   = false;
          media->isBroken    = false;
          media->isPlayed    = false;
          media->isStopAfter = false;

          XspfStreams* xspfstream = static_cast<XspfStreams*>(m_services[VIEW::ViewFavoriteRadio]);
          xspfstream->updateItem(media);
          xspfstream->saveToFile();
          xspfstream->reload();
      }
      else {
          StatusWidget::instance()->startShortMessage("please fill all requested fields", STATUS::TYPE_WARNING, 5000);
      }      
    }
}

void StreamScene::slot_on_import_stream_clicked()
{
    Debug::debug() << "   [StreamScene] slot_on_import_stream_clicked";
    
    /* open stream playlist file */
    FileDialog fd(this->parentView(), FileDialog::AddFile, tr("import stream file"));
    QStringList filters = QStringList() << tr("xspf playlist (*.xspf)");
    fd.setFilters(filters);
    
    if(fd.exec() == QDialog::Accepted) 
    {
        QString filename  = fd.addFile();
        Debug::debug() << "   [StreamScene] slot_on_import_stream_clicked : " << filename;

        /* parse playlist */
        QList<MEDIA::TrackPtr> streams = MEDIA::PlaylistFromFile(filename);
    
        /* add to favorite model */
        XspfStreams* xspfstream = static_cast<XspfStreams*>(m_services[VIEW::ViewFavoriteRadio]);

        foreach(MEDIA::TrackPtr stream, streams)
          xspfstream->updateItem(stream);
    
        xspfstream->saveToFile();
        xspfstream->reload();    
    }
}



void StreamScene::editStream()
{
    Debug::debug() << "   [StreamScene] editStream";
    if(!m_mouseGrabbedItem) return;
    StreamGraphicItem *item = static_cast<StreamGraphicItem*>(m_mouseGrabbedItem);

    MEDIA::TrackPtr stream = MEDIA::TrackPtr::staticCast(item->media);

    AddStreamDialog dialog(this->parentView(),true);
    dialog.setCategory(stream->categorie);
    dialog.setName(stream->name);
    dialog.setUrl(stream->url);
    
    if(dialog.exec() == QDialog::Accepted)
    {
      const QString category = dialog.category();
      const QString name     = dialog.name();
      const QString url      = dialog.url();

      stream->url         = url;
      stream->name        = !name.isEmpty() ? name : url ;
      stream->categorie   = category;

      XspfStreams* xspfstream = static_cast<XspfStreams*>(m_services[VIEW::ViewFavoriteRadio]);

      xspfstream->saveToFile();
      xspfstream->reload();
    }
}


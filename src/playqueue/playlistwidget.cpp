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
#include "playlistwidget.h"

#include "playqueue_proxymodel.h"
#include "task_manager.h"

#include "widgets/statusmanager.h"
#include "widgets/exlineedit.h"
#include "widgets/header_widget.h"
#include "widgets/sort_widget.h"

#include "widgets/dialogs/addstreamdialog.h"
#include "widgets/dialogs/filedialog.h"

#include "global_actions.h"
#include "settings.h"
#include "debug.h"

#include <QLayout>
#include <QApplication>
#include <QWidgetAction>
#include <QToolButton>


/*
********************************************************************************
*                                                                              *
*    Class PlaylistWidget                                                      *
*                                                                              *
********************************************************************************
*/
PlaylistWidget::PlaylistWidget(QWidget *parent) : QWidget(parent), PlaylistWidgetBase()
{
    m_actions  = ACTIONS();

    init(new Playqueue());
}
    
PlaylistWidget::PlaylistWidget(QWidget *parent, PlayqueueModel* model) : QWidget(parent), PlaylistWidgetBase()
{    
    m_actions  = new QMap<ENUM_ACTION, QAction*>;

    init(model);
}

void PlaylistWidget::init(PlayqueueModel* model)
{
    m_model    = model;
    m_view     = new PlaylistView(this, m_model);
    
    /* ------- menu & actions ------------------ */
    m_menu     = 0;
    
    m_actions->insert(PLAYQUEUE_JUMP_TO_TRACK, new QAction(QIcon(":/images/jump_to_32x32.png"),tr("Jump to track"), this));
    
    m_actions->insert(PLAYQUEUE_ADD_FILE, new QAction(QIcon(":/images/track-48x48.png"),tr("&Add media to playlist"), this));
    m_actions->insert(PLAYQUEUE_ADD_DIR, new QAction(QIcon(":/images/folder-48x48.png"),tr("&Add directory to playlist"), this));
    m_actions->insert(PLAYQUEUE_ADD_URL, new QAction(QIcon(":/images/media-url-48x48.png"),tr("&Add Url..."), this));
    m_actions->insert(PLAYQUEUE_CLEAR, new QAction(QIcon::fromTheme("edit-clear-list"), tr("&Clear playlist"), this));    
    m_actions->insert(PLAYQUEUE_EXPORT, new QAction(QIcon(":/images/save-32x32.png"), tr("&Export playlist to file"), this));
    m_actions->insert(PLAYQUEUE_AUTOSAVE, new QAction(QIcon(":/images/save-32x32.png"), tr("&Save playlist"), this));
    m_actions->insert(PLAYQUEUE_REMOVE_ITEM,new QAction(QIcon::fromTheme("edit-delete"),tr("&Remove media from playlist"), this));
    m_actions->insert(PLAYQUEUE_REMOVE_DUPLICATE,new QAction(QIcon(),tr("Remove duplicate"), this));
    
    m_actions->value(PLAYQUEUE_REMOVE_DUPLICATE)->setCheckable(true);
    m_actions->value(PLAYQUEUE_REMOVE_DUPLICATE)->setChecked( !SETTINGS()->_playqueueDuplicate );

    m_actions->insert(PLAYQUEUE_TRACK_LOVE, new QAction(QIcon(":/images/lastfm.png"), tr("Send LastFm love"), this));
    
    m_actions->insert(PLAYQUEUE_OPTION_SHOW_COVER, new QAction(tr("Show cover"), this));
    m_actions->value(PLAYQUEUE_OPTION_SHOW_COVER)->setCheckable(true);
    m_actions->value(PLAYQUEUE_OPTION_SHOW_COVER)->setChecked( SETTINGS()->_playqueueShowCover );

    m_actions->insert(PLAYQUEUE_OPTION_SHOW_RATING, new QAction(tr("Show rating"), this));
    m_actions->value(PLAYQUEUE_OPTION_SHOW_RATING)->setCheckable(true);
    m_actions->value(PLAYQUEUE_OPTION_SHOW_RATING)->setChecked( SETTINGS()->_playqueueShowRating );

    m_actions->insert(PLAYQUEUE_OPTION_SHOW_NUMBER, new QAction(tr("Show track number"), this));
    m_actions->value(PLAYQUEUE_OPTION_SHOW_NUMBER)->setCheckable(true);
    m_actions->value(PLAYQUEUE_OPTION_SHOW_NUMBER)->setChecked( SETTINGS()->_playqueueShowNumber );
    

    /* ------- playqueue sorter widget --------- */
    m_sort_menu = new QMenu();
    ui_sort_widget = 0;

    QToolButton *button = new QToolButton();
    button->setText(tr("sort"));
    button->setAutoRaise(true);
    button->setToolTip(tr("Sort playqueue"));
    button->setMenu(m_sort_menu);
    button->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( button, SIGNAL( clicked ( bool ) ), this, SLOT( slot_show_sortmenu() ) );

    /* ------- playqueue filter widget --------- */
    ui_playqueue_filter = new ExLineEdit();
    ui_playqueue_filter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui_playqueue_filter->setInactiveText(tr("Playqueue filter"));
    
    ui_filter_container = new QWidget(this);
    ui_filter_container->setMinimumHeight(30);
  
    QHBoxLayout* h1 = new QHBoxLayout(ui_filter_container);
    h1->setSpacing(0);
    h1->setContentsMargins(2, 2, 2, 2);   
    h1->addWidget(ui_playqueue_filter); 
    h1->addWidget(button); 


    /* ------- main layout --------------------- */
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addWidget(ui_filter_container);
    main_layout->addWidget(m_view);
    
    /* ------- actions ------------------------- */
    m_action_show_filter = new QAction(QIcon(),tr("Show filter"), this);
    m_action_show_filter->setCheckable(true);    
    m_action_show_filter->setChecked( SETTINGS()->_playqueueShowFilter );
    
    m_action_stop_after = new QAction(QIcon(),tr("Stop after this track"), this);
    m_action_stop_after->setCheckable(true);
    
    /* ------- signals connection -------------- */
    connect(m_model, SIGNAL(updated()), this, SLOT(slot_update_playqueue_status_info()));

    connect(m_actions->value(PLAYQUEUE_TRACK_LOVE), SIGNAL(triggered()), m_view, SLOT(slot_lastfm_love()));    
    connect(m_actions->value(PLAYQUEUE_JUMP_TO_TRACK), SIGNAL(triggered()), m_view, SLOT(jumpToCurrentlyPlayingTrack()));
    
    connect(m_actions->value(PLAYQUEUE_REMOVE_DUPLICATE), SIGNAL(triggered()), SLOT(slot_removeduplicate_changed()));
    connect(m_actions->value(PLAYQUEUE_CLEAR), SIGNAL(triggered()), SLOT(slot_playqueue_clear()));
    
    connect(m_actions->value(PLAYQUEUE_ADD_FILE), SIGNAL(triggered()), SLOT(slot_add_to_playqueue()));
    connect(m_actions->value(PLAYQUEUE_ADD_DIR), SIGNAL(triggered()), SLOT(slot_add_to_playqueue()));
    connect(m_actions->value(PLAYQUEUE_ADD_URL), SIGNAL(triggered()), SLOT(slot_add_to_playqueue()));
    connect(m_actions->value(PLAYQUEUE_EXPORT), SIGNAL(triggered()), SLOT(slot_playqueue_export()));
    connect(m_actions->value(PLAYQUEUE_AUTOSAVE), SIGNAL(triggered()), SLOT(slot_playqueue_save_auto()));
    connect(m_actions->value(PLAYQUEUE_REMOVE_ITEM), SIGNAL(triggered()), this, SLOT(slot_remove_selected_tracks()));

    connect(ui_playqueue_filter, SIGNAL(textfield_entered()),this, SLOT(slot_update_filter()));

    connect(m_action_show_filter, SIGNAL(triggered()), this, SLOT(slot_show_filter_triggered()));
    connect(m_action_stop_after, SIGNAL(triggered()), this, SLOT(slot_stop_after_triggered()));

    /* ------- init state ---------------------- */
    ui_filter_container->setVisible( SETTINGS()->_playqueueShowFilter );
}


void PlaylistWidget::slot_show_filter_triggered()
{
    //Debug::debug() << "    [PlaylistWidget] slot_show_filter_triggered"
    SETTINGS()->_playqueueShowFilter = m_action_show_filter->isChecked(); 

    ui_filter_container->setVisible(SETTINGS()->_playqueueShowFilter);
}

/*******************************************************************************
    PlaylistWidget::slot_show_sortmenu
*******************************************************************************/
void PlaylistWidget::slot_show_sortmenu()
{
    //Debug::debug() << "    [PlaylistWidget] slot_show_sortmenu"
    if(!ui_sort_widget)
    {
        ui_sort_widget = new SortWidget(m_sort_menu);

        QWidgetAction * sort_action_widget = new QWidgetAction( this );
        sort_action_widget->setDefaultWidget( ui_sort_widget );

        m_sort_menu->addAction( sort_action_widget );   
        
        connect(ui_sort_widget, SIGNAL(triggered(QVariant)), m_model, SLOT(slot_sort(QVariant)));
    }
    
    QPoint pos( this->width()-m_sort_menu->sizeHint().width()-4 , 
                ui_playqueue_filter->height() -4);
    
    m_sort_menu->exec( mapToGlobal( pos ) );
}

    
/*******************************************************************************
    PlaylistWidget::slot_update_filter
*******************************************************************************/
void PlaylistWidget::slot_update_filter()
{
    //Debug::debug() << "PlaylistWidget --> slot_update_filter  ";
    m_model->proxy()->setFilterRegExp( ui_playqueue_filter->text() );
}

/*******************************************************************************
    PlaylistWidget::slot_update_playqueue_status_info
*******************************************************************************/
void PlaylistWidget::slot_update_playqueue_status_info()
{
    //Debug::debug() << "    [PlaylistWidget] slot_update_plaqueue_status_info";

    const int rowCount = m_model->rowCount(QModelIndex());
    QString duree      = m_model->queueDuration();

    QString playlistInfo;

    if (rowCount < 1)
      playlistInfo = tr("Empty Playlist");
    else
      playlistInfo = QString(tr("Playlist : <b>%1</b> tracks - %2")).arg(QString::number(rowCount), duree);

    StatusManager::instance()->startMessage(playlistInfo, STATUS::PLAYQUEUE, 2500);
}


/*******************************************************************************
    PlaylistWidget::slot_stop_after_triggered
*******************************************************************************/
void PlaylistWidget::slot_stop_after_triggered()
{
    MEDIA::TrackPtr requested_media = qvariant_cast<MEDIA::TrackPtr>( m_action_stop_after->data() );

    if(!requested_media)
      return;

    m_model->setStopAfterTrack(requested_media);

    m_action_stop_after->setChecked( requested_media->isStopAfter );
}

/*******************************************************************************
    PlaylistWidget::slot_removeduplicate_changed
*******************************************************************************/
void PlaylistWidget::slot_removeduplicate_changed()
{
    if( ACTIONS()->value(PLAYQUEUE_REMOVE_DUPLICATE)->isChecked() )
      m_model->removeDuplicate();  
}

/*******************************************************************************
    PlaylistWidget::slot_remove_selected_tracks
*******************************************************************************/
void PlaylistWidget::slot_remove_selected_tracks()
{
    m_view->removeSelected();
}

/*******************************************************************************
    PlaylistWidget::contextMenuEvent
*******************************************************************************/
void PlaylistWidget::contextMenuEvent(QContextMenuEvent* e)
{
    //Debug::debug() << "    [PlaylistWidget] contextMenuEvent";
    /* ---- update playqueue STOP AFTER action ---- */
    const QModelIndex idx           = m_view->indexAt( m_view->mapFromParent(e->pos()) );
    const QModelIndex source_idx    = m_model->proxy()->mapToSource(idx);
    const int right_click_row = source_idx.row();

    if( right_click_row != -1) 
    {
      m_action_stop_after->setEnabled(true);

      MEDIA::TrackPtr right_click_track = m_model->trackAt(right_click_row);

      QVariant v;
      v.setValue(static_cast<MEDIA::TrackPtr>(right_click_track));
      m_action_stop_after->setData(v);

      m_action_stop_after->setChecked( right_click_track->isStopAfter );
    }
    else 
    {
      m_action_stop_after->setEnabled(false);
      m_action_stop_after->setChecked(false);
    }

    /* ---- update actions ---- */
    const bool isPlaylistEmpty = m_model->rowCount(QModelIndex()) < 1;

    m_actions->value(PLAYQUEUE_CLEAR)->setEnabled(!isPlaylistEmpty);
    m_actions->value(PLAYQUEUE_EXPORT)->setEnabled(!isPlaylistEmpty);
    m_actions->value(PLAYQUEUE_AUTOSAVE)->setEnabled(!isPlaylistEmpty);
    m_actions->value(PLAYQUEUE_REMOVE_ITEM)->setEnabled((!isPlaylistEmpty) && m_view->isTrackSelected());


    /* ---- build menu ---- */
    if (!m_menu) {
      m_menu = new QMenu(this);
      m_menu->setPalette(QApplication::palette());

      QMenu *m1 = m_menu->addMenu(QIcon(":/images/add_32x32.png"), tr("Add"));
      m1->addAction(m_actions->value(PLAYQUEUE_ADD_FILE));
      m1->addAction(m_actions->value(PLAYQUEUE_ADD_DIR));
      m1->addAction(m_actions->value(PLAYQUEUE_ADD_URL));

      m_menu->addSeparator();
      m_menu->addAction(m_actions->value(PLAYQUEUE_REMOVE_ITEM));
      m_menu->addAction(m_actions->value(PLAYQUEUE_CLEAR));
      m_menu->addAction(m_actions->value(PLAYQUEUE_EXPORT));
      m_menu->addAction(m_actions->value(PLAYQUEUE_AUTOSAVE));
      m_menu->addAction( m_action_stop_after );

      m_menu->addSeparator();
      m_menu->addAction(m_actions->value(PLAYQUEUE_JUMP_TO_TRACK));
      m_menu->addAction(m_actions->value(PLAYQUEUE_REMOVE_DUPLICATE));

      QMenu *m2 = m_menu->addMenu(tr("Display options"));
      m2->addAction(m_actions->value(PLAYQUEUE_OPTION_SHOW_COVER));
      m2->addAction(m_actions->value(PLAYQUEUE_OPTION_SHOW_RATING));
      m2->addAction(m_actions->value(PLAYQUEUE_OPTION_SHOW_NUMBER));
      
      QMenu *m3 = m_menu->addMenu(tr("Widgets options"));
      m3->addAction(m_action_show_filter);
      
      m_menu->addSeparator();
      m_menu->addAction(m_actions->value(PLAYQUEUE_TRACK_LOVE));
    }

    /* build menu */
    m_menu->popup(e->globalPos());
    e->accept();
}


/*******************************************************************************
    PlaylistWidget::slot_playqueue_clear
*******************************************************************************/
void PlaylistWidget::slot_playqueue_clear()
{
    m_model->clear();
}


/*******************************************************************************
    PlaylistWidget::slot_add_to_playqueue
*******************************************************************************/
void PlaylistWidget::slot_add_to_playqueue()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action) return;

    /*--------------------------------------------------*/
    /* add file to playqueue                            */
    /* -------------------------------------------------*/
    if( action == m_actions->value(PLAYQUEUE_ADD_FILE) )
    {
      FileDialog fd(this, FileDialog::AddFiles, tr("Add music files or playlist"));

      if(fd.exec() == QDialog::Accepted) {
        QStringList files  = fd.addFiles();
        m_model->manager()->playlistAddFiles(files);
      }      
    }
    /*--------------------------------------------------*/
    /* add dir to playqueue                             */
    /* -------------------------------------------------*/
    else if ( action == m_actions->value(PLAYQUEUE_ADD_DIR) )
    {
      FileDialog fd(this, FileDialog::AddDirs, tr("Add music directories"));

      if(fd.exec() == QDialog::Accepted) {
        QStringList dirs  = fd.addDirectories();
        m_model->manager()->playlistAddFiles(dirs);
      }    
    }
    /*--------------------------------------------------*/
    /* add url to playqueue                             */
    /* -------------------------------------------------*/
    else if ( action == m_actions->value(PLAYQUEUE_ADD_URL) )
    {
      AddStreamDialog stream_dialog(this,false);

      if(stream_dialog.exec() == QDialog::Accepted)
      {
        const QString url   = stream_dialog.url();

        if(!QUrl(url).isEmpty() && QUrl(url).isValid()) 
        {
          const QString name  = stream_dialog.name();

          MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());
          media->setType(TYPE_STREAM);
          media->id          = -1;
          media->url         = url;
          media->extra["station"] = !name.isEmpty() ? name : url ;
          media->isFavorite  = false;
          media->isPlaying   = false;
          media->isBroken    = false;
          media->isPlayed    = false;
          media->isStopAfter = false;

          m_model->addMediaItem(media);
          media.reset();
        }
        else 
        {
          StatusManager::instance()->startMessage("invalid url can not be added !!", STATUS::WARNING, 5000);
        }
      }
    }
}


void PlaylistWidget::slot_playqueue_export()
{
    FileDialog fd(this, FileDialog::SaveFile, tr("Export playlist to file"));

    if(fd.exec() == QDialog::Accepted) 
    {
      QString file  = fd.saveFile();
      if (!file.isEmpty())
        m_model->manager()->playlistSaveToFile(file);
    }  
}

void PlaylistWidget::slot_playqueue_save_auto()
{
    DialogInput input(this, tr("name"), tr("Save playlist"));
    input.setFixedSize(480,140);
        
    if(input.exec() == QDialog::Accepted) 
    {
      //Debug::debug() << "[PlaylistWidget] slot_playqueue_save_auto : " << input.editValue();
      m_model->manager()->playlistSaveToDb(input.editValue());
    }
}

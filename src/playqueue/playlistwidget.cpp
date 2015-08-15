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

#include "playlistwidget.h"
#include "playqueue_proxymodel.h"
#include "widgets/statusmanager.h"
#include "widgets/exlineedit.h"
#include "widgets/header_widget.h"
#include "widgets/sort_widget.h"
#include "widgets/nowplaying/nowplayingview.h"

#include "settings.h"

#include "global_actions.h"
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
PlaylistWidget::PlaylistWidget(QWidget *parent,PlaylistView *view, PlayqueueModel* model) : QWidget(parent)
{
    /* init */
    m_menu     = 0;
    m_view     = view;
    m_model    = model;

    /* playqueue sorter widget */
      m_sort_menu = new QMenu();
      ui_sort_widget = 0;

      QToolButton *button = new QToolButton();
      button->setText(tr("sort"));
      button->setAutoRaise(true);
      button->setToolTip(tr("Sort playqueue"));
      button->setMenu(m_sort_menu);
      button->setContextMenuPolicy( Qt::CustomContextMenu );
      connect( button, SIGNAL( clicked ( bool ) ), this, SLOT( slot_show_sortmenu() ) );

    /* playqueue filter widget */
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


    /* now playing */
    m_nowplaying = new NowPlayingView(this);
    
    /* main layout */
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addWidget(ui_filter_container);
    main_layout->addWidget(m_nowplaying);
    main_layout->addWidget(m_view);
    
    /* ---------- actions ---------- */
    m_action_show_filter = new QAction(QIcon(),tr("Show filter"), this);
    m_action_show_filter->setCheckable(true);    
    m_action_show_filter->setChecked( SETTINGS()->_playqueueShowFilter );
    
    m_action_stop_after = new QAction(QIcon(),tr("Stop after this track"), this);
    m_action_stop_after->setCheckable(true);
    
    
    
    /* signals connection */
    connect(m_model, SIGNAL(updated()), this, SLOT(slot_update_playqueue_status_info()));
    connect(ACTIONS()->value(PLAYQUEUE_REMOVE_DUPLICATE), SIGNAL(triggered()), SLOT(slot_removeduplicate_changed()));

    connect(ui_playqueue_filter, SIGNAL(textfield_entered()),this, SLOT(slot_update_filter()));

    connect(m_action_show_filter, SIGNAL(triggered()), this, SLOT(slot_show_filter_triggered()));
    connect(m_action_stop_after, SIGNAL(triggered()), this, SLOT(slot_stop_after_triggered()));

    connect(ACTIONS()->value(APP_SHOW_NOW_PLAYING), SIGNAL(triggered()), SLOT(slot_show_now_playing_triggered()));
    
    /* ---------- init state ---------- */
    ui_filter_container->setVisible( SETTINGS()->_playqueueShowFilter );

    m_nowplaying->setVisible( SETTINGS()->_showNowPlaying );
}


void PlaylistWidget::slot_show_now_playing_triggered()
{
    SETTINGS()->_showNowPlaying = ACTIONS()->value(APP_SHOW_NOW_PLAYING)->isChecked();
    m_nowplaying->setVisible(SETTINGS()->_showNowPlaying);
}

void PlaylistWidget::slot_show_filter_triggered()
{
    //Debug::debug() << "    [PlaylistWidget] slot_show_filter_triggered"
    ui_filter_container->setVisible(m_action_show_filter->isChecked());
    SETTINGS()->_playqueueShowFilter = m_action_show_filter->isChecked(); 
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
    PlaylistWidget::contextMenuEvent
*******************************************************************************/
void PlaylistWidget::contextMenuEvent(QContextMenuEvent* e)
{
    //Debug::debug() << "    [PlaylistWidget] contextMenuEvent";
    QMap<ENUM_ACTION, QAction*> *actions = ACTIONS();

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

    ACTIONS()->value(PLAYQUEUE_CLEAR)->setEnabled(!isPlaylistEmpty);
    ACTIONS()->value(PLAYQUEUE_SAVE)->setEnabled(!isPlaylistEmpty);
    ACTIONS()->value(PLAYQUEUE_AUTOSAVE)->setEnabled(!isPlaylistEmpty);
    ACTIONS()->value(PLAYQUEUE_REMOVE_ITEM)->setEnabled((!isPlaylistEmpty) && m_view->isTrackSelected());


    /* ---- build menu ---- */
    if (!m_menu) {
      m_menu = new QMenu(this);
      QMenu *m1 = m_menu->addMenu(QIcon(":/images/add_32x32.png"), tr("Add"));
      m1->addAction(actions->value(PLAYQUEUE_ADD_FILE));
      m1->addAction(actions->value(PLAYQUEUE_ADD_DIR));
      m1->addAction(actions->value(PLAYQUEUE_ADD_URL));
      m_menu->addAction(actions->value(PLAYQUEUE_CLEAR));
      m_menu->addAction(actions->value(PLAYQUEUE_SAVE));
      m_menu->addAction(actions->value(PLAYQUEUE_AUTOSAVE));
      m_menu->addAction(actions->value(PLAYQUEUE_REMOVE_ITEM));
      m_menu->addAction( m_action_stop_after );
      m_menu->addAction(actions->value(PLAYQUEUE_REMOVE_DUPLICATE));
      m_menu->addAction(actions->value(PLAYQUEUE_JUMP_TO_TRACK));
      m_menu->addSeparator();
      QMenu *m2 = m_menu->addMenu(tr("Display options"));
      m2->addAction(actions->value(PLAYQUEUE_OPTION_SHOW_COVER));
      m2->addAction(actions->value(PLAYQUEUE_OPTION_SHOW_RATING));
      m_menu->addAction(actions->value(APP_SHOW_NOW_PLAYING));
      m_menu->addAction(m_action_show_filter);
      m_menu->addSeparator();
      m_menu->addAction(actions->value(PLAYQUEUE_TRACK_LOVE));
    }

    /* build menu */
    m_menu->popup(e->globalPos());
    e->accept();
}

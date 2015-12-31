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

#include "editor_playlist.h"
#include "playlistwidget.h"
#include "task_manager.h"
#include "threadmanager.h"

#include "global_actions.h"
#include "settings.h"
#include "debug.h"

#include <QtCore>
#include <QLabel>
#include <QLayout>


/*
********************************************************************************
*                                                                              *
*    Class EditorPlaylist                                                      *
*                                                                              *
********************************************************************************
*/
EditorPlaylist::EditorPlaylist(QWidget *parent) : QWidget(parent), PlaylistWidgetBase()
{
    /* ------- init ---------------------------- */    
    PlaylistWidget* widget = new PlaylistWidget(this, new PlayqueueModel(this));
    
    m_model         = widget->model();
    m_view          = widget->view();
    m_task_manager  = m_model->manager();
    m_actions       = widget->menuActions();
    
    /* ----- init playlist data ----- */    
    m_playlist = MEDIA::PlaylistPtr( new MEDIA::Playlist() );
    m_playlist->id       =  -1;
    m_playlist->url      =  QString();
    m_playlist->name     =  QString("untitled playlist");
    m_playlist->p_type   =  T_DATABASE;
    
    /* ------- create ui ----------------------- */    
    QLabel* ui_header = new QLabel(tr("Edit playlist"));
    ui_header->setFont(QFont("Arial",12,QFont::Bold));
    ui_header->setStyleSheet(QString("QLabel { color : %1; }").arg(SETTINGS()->_baseColor.name()));
    
    ui_edit_name = new ExLineEdit();
    ui_edit_name->clearFocus();
    ui_edit_name->setInactiveText(tr("playlist name"));

    ui_buttonBox = new QDialogButtonBox();
    ui_buttonBox->setOrientation(Qt::Horizontal);
    ui_buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Close);

    /* ------- layouts ------------------------- */    
    QVBoxLayout *l2 = new QVBoxLayout();
    l2->setSpacing(2);
    l2->setContentsMargins(0, 6, 0, 6);
    l2->addWidget( ui_header );
    
    QHBoxLayout *hl1 = new QHBoxLayout();
    hl1->setSpacing(2);
    hl1->setContentsMargins(0, 0, 0, 0);
    hl1->addWidget( ui_edit_name );
    hl1->addWidget( ui_buttonBox );
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(4, 2, 4, 4);
    layout->addLayout( l2 );
    layout->addWidget( widget );
    layout->addLayout( hl1 );
    
    /* ------- focus --------------------------- */    
    this->setFocus(Qt::OtherFocusReason);    
    
    /* ------- connections --------------------- */ 
    disconnect(m_actions->value(PLAYQUEUE_AUTOSAVE), 0, 0, 0);
    
    connect(m_actions->value(PLAYQUEUE_AUTOSAVE), SIGNAL(triggered()), SLOT(slot_save_playlist()));
               
    connect(ui_buttonBox,  SIGNAL(clicked ( QAbstractButton *)), this, SLOT(slot_on_buttonbox_clicked(QAbstractButton *)));
 
    connect(m_task_manager, SIGNAL(playlistSaved()), ThreadManager::instance(), SLOT(populateLocalPlaylistModel()));
    connect(m_task_manager, SIGNAL(loadPlaylist(MediaMimeData*,int)), this, SLOT(slot_on_playlist_load(MediaMimeData*,int)));    
}

EditorPlaylist::~EditorPlaylist()
{
    delete m_model;
}


/*******************************************************************************
    slot_on_buttonbox_clicked
*******************************************************************************/
void EditorPlaylist::slot_on_buttonbox_clicked(QAbstractButton * button)
{
    Debug::debug() << "   [EditorPlaylist] slot_on_buttonbox_clicked";
    
    QDialogButtonBox::ButtonRole role = ui_buttonBox->buttonRole(button);
    
    if(role == QDialogButtonBox::RejectRole) 
    {
      emit close();
    }
    else if (role == QDialogButtonBox::ApplyRole)
    {
      slot_save_playlist();
    }
}


/*******************************************************************************
 public interface
*******************************************************************************/
void EditorPlaylist::setPlaylist(MEDIA::PlaylistPtr playlist)
{
    Debug::debug() << "   [EditorPlaylist] setPlaylist";
    
    m_playlist = playlist;
    
    /* --------- update ui  -------- */
    ui_edit_name->setText( playlist->name );

    /* --------- fill playlist content   -------- */
    m_model->clear();
    foreach(MEDIA::MediaPtr media, playlist->children())   {
      m_model->addMediaItem( MEDIA::TrackPtr::staticCast(media) );
    }
}


void EditorPlaylist::slot_on_playlist_load(MediaMimeData* mime, int row)
{
    Debug::debug() << "   [EditorPlaylist] slot_on_playlist_load";

    if( m_model->rowCount() == 0)
    {
        if( mime->parent() )
          setPlaylist (    
              MEDIA::PlaylistPtr::staticCast( mime->parent() )
          );
    }
    else
    {
        m_task_manager->playlistAddMediaItems( mime->getTracks(), row);
    }
}


/*******************************************************************************
    EditorPlaylist::slot_save_playlist
*******************************************************************************/
void EditorPlaylist::slot_save_playlist()
{
    Debug::debug() << "   [EditorPlaylist] slot_save_playlist";
    
    m_playlist->name = ui_edit_name->text();

    if(m_playlist->p_type == T_DATABASE)
    {   
        m_task_manager->playlistSaveToDb( m_playlist );
    }
    else if (m_playlist->p_type == T_FILE)
    {
        m_task_manager->playlistSaveToFile( m_playlist->url );
    }    
}

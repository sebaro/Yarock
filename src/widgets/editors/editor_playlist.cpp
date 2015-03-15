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

#include "editor_playlist.h"
#include "playqueue_model.h"
#include "task_manager.h"
#include "playlistview.h"

#include "threadmanager.h"
#include "widgets/statuswidget.h"
#include "widgets/dialogs/addstreamdialog.h"
#include "widgets/dialogs/filedialog.h"

#include "settings.h"
#include "debug.h"

#include <QtCore>

/*
********************************************************************************
*                                                                              *
*    Class EditorPlaylist                                                      *
*                                                                              *
********************************************************************************
*/
EditorPlaylist::EditorPlaylist(QWidget *parent) : QWidget(parent)
{
    /* ----- init ----- */    
    m_model         = new PlayqueueModel();
    m_task_manager  = m_model->manager();

    m_view  = new PlaylistView(this, m_model);

    m_menu = 0;

    /* ----- init playlist data ----- */    
    m_playlist = MEDIA::PlaylistPtr( new MEDIA::Playlist() );
    m_playlist->id       =  -1;
    m_playlist->url      =  QString();
    m_playlist->name     =  QString("untitled playlist");
    m_playlist->p_type   =  T_DATABASE;

    /* -----    ui   ----- */    
    create_ui();
    slot_on_type_change(0);

    /* ----- connections ----- */    
    connect(ui_combo_type, SIGNAL(currentIndexChanged ( int )), this, SLOT(slot_on_type_change(int)));
    connect(ui_buttonBox, SIGNAL(clicked ( QAbstractButton *)), this, SLOT(slot_on_buttonbox_clicked(QAbstractButton *)));
    connect(ui_file_button, SIGNAL(clicked()), this, SLOT(slot_dialog_file_to_save()));
    connect(m_task_manager, SIGNAL(playlistSaved()), ThreadManager::instance(), SLOT(populateLocalPlaylistModel()));
}

EditorPlaylist::~EditorPlaylist()
{
    delete m_model;
}

/*******************************************************************************
 ui part
*******************************************************************************/
void EditorPlaylist::create_ui()
{
    QLabel* ui_header = new QLabel(tr("Edit playlist"));
    ui_header->setFont(QFont("Arial",12,QFont::Bold));
    ui_header->setStyleSheet(QString("QLabel { color : %1; }").arg(SETTINGS()->_baseColor.name()));
    
    /* ui items */    
    ui_edit_name = new ExLineEdit();
    ui_edit_name->clearFocus();
    ui_edit_name->setInactiveText(tr("playlist name"));

    ui_buttonBox = new QDialogButtonBox();
    ui_buttonBox->setOrientation(Qt::Horizontal);
    ui_buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Close);

    ui_combo_type = new QComboBox();
    ui_combo_type->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    ui_combo_type->addItem(tr("To database"));
    ui_combo_type->addItem(tr("To file"));  
    ui_combo_type->setStyleSheet("QComboBox {background:transparent}");

    ui_file_path = new QLineEdit();
    ui_file_path->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui_file_path->setText(tr("Choose file to save playlist"));
    ui_file_path->setStyleSheet("QLineEdit {background:transparent}");
    ui_file_path->setReadOnly( true );
    ui_file_path->setFocusPolicy( Qt::NoFocus );
    ui_file_path->setFrame( false );
    
    ui_file_button = new QToolButton();
    ui_file_button->setIcon(QIcon(":/images/folder-48x48.png"));
    ui_file_button->setToolTip(tr("Open"));
    

    /* ---- Layouts ---- */
    QVBoxLayout *l2 = new QVBoxLayout();
    l2->setSpacing(2);
    l2->setContentsMargins(0, 6, 0, 6);
    l2->addWidget( ui_header );
    
    ui_save_to_file_widget = new QWidget();
    QHBoxLayout *hb = new QHBoxLayout(ui_save_to_file_widget);
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing( 4 );

    hb->addWidget( ui_file_path );
    hb->addWidget( ui_file_button );

    
    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing( 4 );
    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 2);
    grid->setRowStretch(2, 1);
    
    grid->addWidget(new QLabel(tr("Name")), 0, 0, 1, 1, Qt::AlignVCenter);
    grid->addWidget(ui_edit_name, 0, 1, 1, 1);
    grid->addWidget(ui_combo_type, 0, 2, 1, 1);
    grid->addWidget(ui_save_to_file_widget, 1, 0, 1, 4);
    grid->addWidget(m_view, 2,0,1,4);
    grid->addWidget(ui_buttonBox, 3,0,1,4);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(4, 2, 4, 4);
    layout->addLayout( l2 );
    layout->addLayout( grid );
    
    
    /* focus : avoid focus to be on line edit */
    this->setFocus(Qt::OtherFocusReason);
}


/*******************************************************************************
    slot_on_buttonbox_clicked
*******************************************************************************/
void EditorPlaylist::slot_on_buttonbox_clicked(QAbstractButton * button)
{
    QDialogButtonBox::ButtonRole role = ui_buttonBox->buttonRole(button);
    
    if(role == QDialogButtonBox::RejectRole) 
    {
      emit close();
    }
    else if (role == QDialogButtonBox::ApplyRole)
    {
      /* save current database */
      if(m_playlist->p_type == T_DATABASE)
      {   
         m_task_manager->playlistSaveToDb( ui_edit_name->text() , m_playlist->id);
      }
      else if (m_playlist->p_type == T_FILE)
      {
          //Debug::debug() << "EditorPlaylist::slot_on_buttonbox_clicked => SAVE TO FILE";
          m_task_manager->playlistSaveToFile( m_playlist->url );
      }
    }
}

/*******************************************************************************
    slot_on_type_change
*******************************************************************************/
void EditorPlaylist::slot_on_type_change(int type)
{
   ui_save_to_file_widget->setVisible(type != 0);
   ui_file_path->setEnabled(type != 0);
   ui_file_button->setEnabled(type != 0);

   m_playlist->p_type   = type == 0 ? T_DATABASE : T_FILE;
}


/*******************************************************************************
 public interface
*******************************************************************************/
void EditorPlaylist::setPlaylist(MEDIA::PlaylistPtr playlist)
{
    m_playlist = playlist;
    
    /* update ui  */
    ui_combo_type->setCurrentIndex(m_playlist->p_type == T_DATABASE ? 0 : 1);
    ui_combo_type->setEnabled(false);
    ui_edit_name->setText( playlist->name );
    
    if(m_playlist->p_type == T_FILE) 
    {
      ui_file_path->setText( m_playlist->url );
      ui_file_path->home(false);
    }

    /* fill playlist content */
    m_model->clear();
    foreach(MEDIA::MediaPtr media, playlist->children())   {
      m_model->addMediaItem( MEDIA::TrackPtr::staticCast(media) );
    }
}


/*******************************************************************************
 Playlist Population
*******************************************************************************/
void EditorPlaylist::slot_add_file()
{
    FileDialog fd(this, FileDialog::AddFiles, tr("Add music files or playlist"));

    if(fd.exec() == QDialog::Accepted) 
    {
       QStringList files  = fd.addFiles();
       
       m_task_manager->playlistAddFiles(files);
    }      
}


void EditorPlaylist::slot_add_dir()
{
    /*--------------------------------------------------*/
    /* add dir to playqueue                             */
    /* -------------------------------------------------*/
    FileDialog fd(this, FileDialog::AddDirs, tr("Add music directories"));

    if(fd.exec() == QDialog::Accepted)
    {
      QStringList dirs  = fd.addDirectories();
      m_task_manager->playlistAddFiles(dirs);
    }    
}


void EditorPlaylist::slot_add_url()
{
    AddStreamDialog stream_dialog(this,false);

    if(stream_dialog.exec() == QDialog::Accepted)
    {
      const QString url   = stream_dialog.url();

      if(!QUrl(url).isEmpty() && QUrl(url).isValid()) {
        const QString name  = stream_dialog.name();

        MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());
        media->setType(TYPE_STREAM);
        media->id          = -1;
        media->url         = url;
        media->name        = !name.isEmpty() ? name : url ;
        media->isFavorite  = false;
        media->isPlaying   = false;
        media->isBroken    = false;
        media->isPlayed    = false;
        media->isStopAfter = false;

        m_model->addMediaItem(media);
        media.reset();
      }
      else {
        StatusWidget::instance()->startShortMessage("invalid url can not be added !!", STATUS::TYPE_WARNING, 5000);
      }
    }
}


/*******************************************************************************
    slot_dialog_file_to_save
*******************************************************************************/
void EditorPlaylist::slot_dialog_file_to_save()
{
    FileDialog fd(this, FileDialog::SaveFile, tr("Save playlist to file"));
  
    if(fd.exec() == QDialog::Accepted)
    {
      ui_file_path->setText( fd.saveFile() );
      ui_file_path->home(false);
      m_playlist->url = ui_file_path->text();
    }
}


 
/*******************************************************************************
    contextMenuEvent
*******************************************************************************/
void EditorPlaylist::contextMenuEvent(QContextMenuEvent* e)
{
    /* build menu */
    if (!m_menu) {      
      m_menu = new QMenu(this);

      m_menu->addAction( QIcon(":/images/track-48x48.png"), tr("&Add media to playlist"), this, SLOT(slot_add_file()));
      m_menu->addAction( QIcon(":/images/folder-48x48.png"), tr("&Add directory to playlist"), this, SLOT(slot_add_dir()));
      m_menu->addAction( QIcon(":/images/media-url-48x48.png"),tr("&Add Url..."), this, SLOT(slot_add_url()));
      m_menu->addSeparator();
      m_menu->addAction( QIcon::fromTheme("edit-clear-list"), tr("Clear playlist"), m_model, SLOT(clear()));
    }

    /* build menu */
    m_menu->popup(e->globalPos());
    e->accept();
}
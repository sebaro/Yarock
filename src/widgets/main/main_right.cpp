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
#include "main_right.h"
#include "global_actions.h"
#include "debug.h"

#include "playqueue/playlistwidget.h"
#include "widgets/editors/editor_playlist.h"
#include "widgets/editors/editor_smart.h"
#include "widgets/editors/editor_artist.h"
#include "widgets/editors/editor_album.h"
#include "widgets/editors/editor_track.h"


#include <QtGui>


MainRightWidget* MainRightWidget::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class MainRightWidget                                                     *
*                                                                              *
********************************************************************************
*/
MainRightWidget::MainRightWidget(QWidget *parent)
{
    INSTANCE = this;
    
    m_parent = parent;
    
    /* ----- header ----- */
    m_header = new HeaderWidget(m_parent);
    m_header->setMinimumHeight(36);

    QToolButton* ui_tool_button = new QToolButton(m_parent);
    ui_tool_button->setArrowType(Qt::DownArrow); 
    ui_tool_button->setAutoRaise( true );
    ui_tool_button->setPopupMode(QToolButton::InstantPopup);
    
    /* ----- header layout ----- */
    QHBoxLayout* h1 = new QHBoxLayout(m_header);
    h1->setSpacing(0);
    h1->setContentsMargins(4, 4, 4, 0 /* bottom*/ );
    h1->addSpacerItem ( new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum) );
    h1->addWidget( ui_tool_button );

    
    /* ----- content ----- */
    m_stackedWidget = new QStackedWidget(m_parent);
    m_stackedWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding );
      
    /* ----- init ----- */
    m_buttons = new QButtonGroup();
    m_buttons->setExclusive(true);    
    m_buttonId = 0;
    
    m_right_menu = 0;

    /* ----- connections ----- */
    connect( m_buttons, SIGNAL(buttonClicked ( QAbstractButton* )), this, SLOT(slot_button_clicked(QAbstractButton*)));
    connect( ui_tool_button, SIGNAL( clicked ( bool ) ), this, SLOT( slot_onmenu_clicked() ) );
}


void MainRightWidget::slot_onmenu_clicked()
{
    if( !m_right_menu )
    {
      m_right_menu = new QMenu();
  
      QAction* Action_Playlist_Edit = new QAction(QIcon(":/images/edit-48x48.png"),tr("New playlist"), this);
      QAction* Action_Smart_Edit    = new QAction(QIcon(":/images/smart-playlist-48x48.png"),tr("New smart playlist"), this);
      
      connect(Action_Playlist_Edit, SIGNAL(triggered()), SLOT(slot_create_new_playlist_editor()));
      connect(Action_Smart_Edit, SIGNAL(triggered()), SLOT(slot_create_new_smart_editor()));
      
      m_right_menu->addAction( Action_Playlist_Edit );
      m_right_menu->addAction( Action_Smart_Edit );
    }
    
    QPoint pos( m_parent->width() - m_right_menu->sizeHint().width() , m_header->height() + 2 );
    m_right_menu->exec( m_parent->mapToGlobal( pos ) );
}


void MainRightWidget::slot_create_new_playlist_editor()
{
    addWidget(MainRightWidget::PLAYLIST_EDIT, new EditorPlaylist(m_parent), true);
  
}

void MainRightWidget::slot_create_new_smart_editor()
{
    addWidget(MainRightWidget::SMART_EDIT, new EditorSmart(m_parent), true);
}
    
    
void MainRightWidget::addWidget(QWidget* widget, bool activate)
{
    if( ! widget )
      return;
    
    if ( PlaylistWidget *w = dynamic_cast<PlaylistWidget*>(widget) )
      addWidget(MainRightWidget::PLAYQUEUE, w, activate);
    else if ( EditorPlaylist *w = dynamic_cast<EditorPlaylist*>(widget) )
      addWidget(MainRightWidget::PLAYLIST_EDIT, w, activate);
    else if ( EditorSmart *w = dynamic_cast<EditorSmart*>(widget) )
      addWidget(MainRightWidget::SMART_EDIT, w, activate);
    else if ( EditorArtist *w = dynamic_cast<EditorArtist*>(widget) )
      addWidget(MainRightWidget::TAG_EDIT, w, activate);
    else if ( EditorAlbum *w = dynamic_cast<EditorAlbum*>(widget) )
      addWidget(MainRightWidget::TAG_EDIT, w, activate);
    else if ( EditorTrack *w = dynamic_cast<EditorTrack*>(widget) )
      addWidget(MainRightWidget::TAG_EDIT, w, activate);
}



void MainRightWidget::addWidget(WidgetType type, QWidget* widget, bool activate)
{
    /* create button */
    QPushButton* button = new QPushButton(m_parent);
    button->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum);

    switch (type)
    {
      case MainRightWidget::PLAYQUEUE: 
          button->setIcon(QIcon(":/images/media-playlist-48x48.png"));
          button->setToolTip(tr("Playqueue"));
      break;
      case MainRightWidget::PLAYLIST_EDIT: 
          button->setIcon(QIcon(":/images/edit-48x48.png"));
          button->setToolTip(tr("Playlist editor"));
      break;  
      case MainRightWidget::TAG_EDIT: 
          button->setIcon(QIcon(":/images/edit-48x48.png"));
          button->setToolTip(tr("Tag editor"));
      break;  
      case MainRightWidget::SMART_EDIT: 
          button->setIcon(QIcon(":/images/smart-playlist-48x48.png"));
          button->setToolTip(tr("Smart editor"));
      break;        

      default:break; 
    }

    /* show panel if panel is currently hiden */
    if(!ACTIONS()->value(APP_SHOW_PLAYQUEUE)->isChecked() && type != MainRightWidget::PLAYQUEUE)
      ACTIONS()->value(APP_SHOW_PLAYQUEUE)->trigger();

    
    button->setCheckable(true);
    button->setStyleSheet(
      QString ("QPushButton { border: none;min-width: 40px;min-height: 32px;}" \
              "QPushButton:checked { background-color: %1 ;border: none;min-width: 40px;min-height: 32px;}" ) 
          .arg( QApplication::palette().color( QPalette::Base ).name() )
    );
    
    button->setChecked(activate);

    static_cast<QHBoxLayout*>(m_header->layout())->insertWidget (static_cast<QHBoxLayout*>(m_header->layout())->count()-2,  button);

    m_buttons->addButton(button);
    m_buttons->setId (button, ++m_buttonId );
    
    
    /* create widget */
    m_stackedWidget->addWidget(widget);
    
    /* associate button/widget */
    m_ids.insert( m_buttons->id(button), widget );
    m_stackedWidget->setCurrentIndex( m_stackedWidget->indexOf(widget) );

    /* connect widget close event */
    if( type != MainRightWidget::PLAYQUEUE )
      connect(widget, SIGNAL( close() ), this, SLOT(slot_widget_closed()));
}


void MainRightWidget::slot_button_clicked(QAbstractButton* button)
{
    //Debug::debug() << Q_FUNC_INFO;

    int id = m_buttons->id( button );
    
    m_stackedWidget->setCurrentWidget(m_ids.value(id));
}

void MainRightWidget::slot_widget_closed()
{
    //Debug::debug() << Q_FUNC_INFO;
  
    QWidget *w = qobject_cast<QWidget *>(sender());
    
    /* active previous tab */
    int currentIdx = m_stackedWidget->currentIndex();
    m_stackedWidget->setCurrentIndex(currentIdx -1);
    
    int button_idx = m_ids.key( m_stackedWidget->currentWidget() );
    m_buttons->button(button_idx)->setChecked(true);
    
    /* remove button */
    button_idx = m_ids.key(w);
    static_cast<QHBoxLayout*>(m_header->layout())->removeWidget( m_buttons->button(button_idx) );
    m_ids.remove( button_idx );
    delete m_buttons->button(button_idx);
    
    /* remove widget */
    m_stackedWidget->removeWidget(w);
    disconnect(w, 0,this, 0);
    w->deleteLater();
}

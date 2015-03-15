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
#include "main_left.h"

// for content
#include "views/browser_view.h"
#include "widgets/main/menuwidget.h"


// for header
#include "maintoolbutton.h"
#include "sort_widget.h"

#include "widgets/searchline_edit.h"
#include "widgets/spacer.h"
#include "widgets/popupcompleter/search_popup.h"

#include "widgets/editors/editor_search.h"
#include "core/mediasearch/media_search.h"

// others
#include "settings.h"
#include "global_actions.h"
#include "debug.h"

#include <QtGui>

MainLeftWidget* MainLeftWidget::INSTANCE = 0;
    
/*
********************************************************************************
*                                                                              *
*    Class MainLeftWidget                                                      *
*                                                                              *
********************************************************************************
*/
MainLeftWidget::MainLeftWidget(QWidget *parent) : QWidget( parent )
{
    INSTANCE   = this;
      
    /* header */
    m_header = new HeaderWidget(this);
    m_header->setMinimumHeight(36);

    create_header_ui();
    
    /* content */
    m_viewsSplitter = new CustomSplitter(this);
    m_viewsSplitter->setObjectName(QString::fromUtf8("viewsSplitter_2"));
    m_viewsSplitter->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );       

    MenuWidget* m_menu_widget = new MenuWidget(this);
    m_menu_widget->setSplitter(m_viewsSplitter);    
    
    m_viewsSplitter->addWidget(m_menu_widget);
    
    /* layout */
    QVBoxLayout* vl0 = new QVBoxLayout(this);
    vl0->setSpacing(0);
    vl0->setContentsMargins(0, 0, 0, 0);
    vl0->addWidget(m_header);
    vl0->addWidget(m_viewsSplitter);   
    
    /* init */
    ui_editor_search = 0;
}

void MainLeftWidget::setBrowser(BrowserView* browser)
{
    m_viewsSplitter->addWidget(browser);
}

void MainLeftWidget::create_header_ui()
{
    /* main tool button */
      MainToolButton* main_tb = new MainToolButton(m_header);
      main_tb->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

      /* title  */
      m_title = new QLabel();
      m_title->setText("Artists");
      m_title->setFont(QFont("Arial",12,QFont::Bold));

      /* settings button */
      ui_save_button   = new QPushButton(QIcon(":/images/save-32x32.png"), tr("Apply"), 0);
      ui_cancel_button = new QPushButton(QIcon::fromTheme("dialog-cancel"), tr("Cancel"), 0);      
      
      /*  prev/next browsing actions  */
      ACTIONS()->insert(BROWSER_PREV, new QAction(QIcon::fromTheme("go-previous", QIcon(":/images/go-previous.png")),tr("Go back"),this));
      ACTIONS()->insert(BROWSER_NEXT, new QAction(QIcon::fromTheme("go-next", QIcon(":/images/go-next.png")),tr("Go forward"),this));
      ACTIONS()->insert(BROWSER_UP,   new QAction(QIcon::fromTheme("go-up", QIcon(":/images/go-up.png")),tr("Go up"),this));
      ACTIONS()->value(BROWSER_PREV)->setEnabled(false);
      ACTIONS()->value(BROWSER_NEXT)->setEnabled(false);
    
      /* browser search line edit */
      ui_search_lineedit = new SearchLineEdit(m_header);
      ui_search_lineedit->setInactiveText(tr("Quick filter"));
      ui_search_lineedit->setMinimumWidth(300);
      ui_search_lineedit->setMaximumWidth(300);
      ui_search_lineedit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
      new QShortcut(QKeySequence("Ctrl+F"), ui_search_lineedit, SLOT(slotFocus()));

      QMenu* menu = new QMenu();
      menu->addAction(ACTIONS()->value(APP_ENABLE_SEARCH_POPUP));
      ui_search_lineedit->setCustomContextMenu(menu);
      
      /* explorer advanced search */
      ui_advance_search_button = new QPushButton();
      ui_advance_search_button->setIcon(QIcon(":/images/add_32x32.png"));
      
      ui_advance_search_button->setFocusPolicy(Qt::NoFocus);
      ui_advance_search_button->setStyleSheet("QPushButton { background-color:transparent; border: none; padding: 0px}");
      ui_advance_search_button->setCursor(Qt::ArrowCursor);
      ui_advance_search_button->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
      ui_advance_search_button->setContextMenuPolicy(Qt::PreventContextMenu);
      
      
      ui_search_lineedit->addLeftIcon(ui_advance_search_button);

      /* explorer popup completer */
      ui_popup_completer = 0;
    
      /* navigation button */
      ui_button_prev = new QToolButton(m_header);
      ui_button_prev->setAutoRaise(true);
      ui_button_prev->setArrowType(Qt::LeftArrow);
      ui_button_prev->setDefaultAction( ACTIONS()->value(BROWSER_PREV) );
    
      ui_button_next = new QToolButton(m_header);
      ui_button_next->setAutoRaise(true);
      ui_button_next->setArrowType(Qt::RightArrow);
      ui_button_next->setDefaultAction( ACTIONS()->value(BROWSER_NEXT) );

      ui_button_up = new QToolButton(m_header);
      ui_button_up->setAutoRaise(true);
      ui_button_up->setArrowType(Qt::UpArrow);
      ui_button_up->setDefaultAction( ACTIONS()->value(BROWSER_UP) );

    
    QHBoxLayout* h2 = new QHBoxLayout(m_header);
    h2->setSpacing(2);
    h2->setContentsMargins(4, 4, 4, 4);
    h2->addWidget(main_tb);    
    h2->addWidget( ui_button_prev );
    h2->addWidget( ui_button_next );
    h2->addWidget( ui_button_up );
    h2->addWidget( new FixedSpacer( m_header, QSize(4, 0)) );
    h2->addWidget( m_title );
    h2->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
    h2->addWidget( ui_search_lineedit );
    h2->addWidget( ui_save_button );
    h2->addWidget( ui_cancel_button );    
    h2->addWidget( new FixedSpacer( m_header, QSize(4, 0)) );
    
    ui_button_up->setVisible(false);
    
    /*--------------------------------*/
    /* Connection                     */
    /*--------------------------------*/      
    connect(ui_advance_search_button, SIGNAL(clicked ()), this, SLOT(slot_advance_search_clicked()));
    
    connect(main_tb, SIGNAL(dbNameChanged()), this, SIGNAL(dbNameChanged()));
    connect(ui_search_lineedit, SIGNAL(textfield_entered()), this, SLOT(slot_send_quick_filter_change()));
    connect(ACTIONS()->value(APP_ENABLE_SEARCH_POPUP), SIGNAL(triggered()), this, SLOT(slot_explorer_popup_setting_change()));
    connect(ui_save_button, SIGNAL(clicked()), this, SIGNAL(settings_save_clicked()));
    connect(ui_cancel_button, SIGNAL(clicked()), this, SIGNAL(settings_cancel_clicked()));
  
    /*--------------------------------*/
    /* Init                           */
    /*--------------------------------*/  
    slot_explorer_popup_setting_change();
}


void MainLeftWidget::setTitle(const QString& title)
{
    m_title->setText(title);
}


void MainLeftWidget::slot_advance_search_clicked()
{  
    if(!ui_editor_search) {
      ui_editor_search = new EditorSearch(this);
      QObject::connect(ui_editor_search, SIGNAL(search_triggered()), this, SLOT(slot_advanced_search_triggered()));
    }
    
    ui_editor_search->show();
  
    ui_editor_search->move (
      mapToGlobal(QPoint(this->width()-ui_editor_search->width(), m_header->height())) 
    );
}


void MainLeftWidget::slot_advanced_search_triggered()
{
    /* check is custom search is cleared or invalid */
    if(!ui_editor_search->isActive()) {
      ui_search_lineedit->lineEdit()->clear();
      ui_search_lineedit->clearFocus();
    }
    
    QVariant var = ui_editor_search->get_search();
    emit browser_search_change( var );
    
    Debug::debug() << "MainLeftWidget::slot_advanced_search_triggered: " << var;
}


void MainLeftWidget::setBrowserSearch(QVariant variant)
{
    if(variant.canConvert<MediaSearch>())
    {
       if(ui_editor_search)
         ui_editor_search->set_search( variant );
       
       ui_search_lineedit->setCustomSearch(true);
    }
    else
    {
      QString pattern = variant.toString();
      ui_search_lineedit->setCustomSearch(false);
      ui_search_lineedit->setText(pattern);
      ui_search_lineedit->clearFocus();
    }      
}


QVariant MainLeftWidget::browserSearch()
{
  Debug::debug() << "MainLeftWidget::browserSearch";
  
    QVariant variant;
    if(ui_editor_search && ui_editor_search->isActive() )
        variant = ui_editor_search->get_search();
    else
        variant = QVariant( ui_search_lineedit->text() );

    return variant;
}


void MainLeftWidget::slot_send_quick_filter_change()
{
    const QString pattern = ui_search_lineedit->text();
    
    if(pattern.isEmpty())
    {
      /* quick filter is cleared */
      if(ui_editor_search)
        ui_editor_search->set_search( QVariant() );
    }

    emit browser_search_change( QVariant(ui_search_lineedit->text()) );
}


void MainLeftWidget::slot_explorer_popup_setting_change()
{
    SETTINGS()->_enableSearchPopup = ACTIONS()->value(APP_ENABLE_SEARCH_POPUP)->isChecked();

    if( SETTINGS()->_enableSearchPopup)
    {
      if(!ui_popup_completer)
        ui_popup_completer = new SearchPopup(ui_search_lineedit);
    }
    else
    {
      if(ui_popup_completer) {
        delete ui_popup_completer;
        ui_popup_completer = 0;
      }
    }
}


void MainLeftWidget::setMode(VIEW::Id mode)
{
    m_mode = mode;
    
    ui_advance_search_button->setEnabled(
       (typeForView(m_mode) == VIEW::LOCAL) && (m_mode != VIEW::ViewSmartPlaylist)
    );
    
    ui_button_up->setVisible(m_mode == VIEW::ViewFileSystem);
    
    ui_save_button->setVisible(m_mode == VIEW::ViewSettings);
    ui_cancel_button->setVisible(m_mode == VIEW::ViewSettings);
    
    ui_search_lineedit->setVisible(  typeForView(m_mode) == VIEW::FILESYSTEM ||
                                     typeForView(m_mode) == VIEW::LOCAL ||
                                     typeForView(m_mode) == VIEW::RADIO );
}

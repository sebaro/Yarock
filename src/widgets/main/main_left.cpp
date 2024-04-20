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
#include "main_left.h"

// for content
#include "views/browser_view.h"
#include "widgets/main/menuwidget.h"
#include "widgets/main/menumodel.h"


// for header
#include "sort_widget.h"

#include "widgets/searchline_edit.h"
#include "widgets/spacer.h"
#include "widgets/popupcompleter/search_popup.h"
#include "widgets/iconloader.h"

#include "widgets/editors/editor_search.h"
#include "core/mediasearch/media_search.h"

// others
#include "iconmanager.h"
#include "settings.h"
#include "global_actions.h"
#include "debug.h"

#include <QtGui>

#if QT_VERSION >= 0x050000
  #include <QShortcut>
#endif

MainLeftWidget* MainLeftWidget::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class MainLeftWidget                                                      *
*                                                                              *
********************************************************************************
*/

MainLeftWidget::MainLeftWidget(QWidget *parent)
{
    INSTANCE   = this;

    m_parent = parent;

    /* content */
    m_contentWidget = new QWidget(m_parent);
    m_contentWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    QHBoxLayout * layout = new QHBoxLayout(m_contentWidget);

    QPalette p1;
    p1.setColor(QPalette::Window, QApplication::palette().color(QPalette::Normal, QPalette::Base));
    m_contentWidget->setPalette( p1 );


    MenuWidget* m_menu_widget = new MenuWidget(m_parent);
    layout->addWidget(m_menu_widget);

    /* header */
    m_header = new HeaderWidget(m_parent);
    m_header->setMinimumHeight(36);

    create_header_ui();

    /* init */
    ui_editor_search = 0;
}

void MainLeftWidget::setBrowser(BrowserView* browser)
{
    m_contentWidget->layout()->addWidget(browser);
}

void MainLeftWidget::create_header_ui()
{
      /* title  */
      m_title = new QLabel();
      m_title->setText("Artists");
      m_title->setFont(QFont("Arial",12,QFont::Bold));

      /* settings button */
      ui_save_button   = new QPushButton(QIcon(":/images/save-32x32.png"), tr("Apply"), 0);
      ui_save_button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

      ui_cancel_button = new QPushButton(QIcon::fromTheme("dialog-cancel"), tr("Cancel"), 0);
      ui_cancel_button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

      /*  prev/next browsing actions  */
      ACTIONS()->insert(BROWSER_PREV, new QAction( IconManager::instance()->icon( "chevron-left") ,tr("Go backward"),this));
      ACTIONS()->insert(BROWSER_NEXT, new QAction( IconManager::instance()->icon( "chevron-right"), tr("Go forward"),this));
      ACTIONS()->insert(BROWSER_UP,   new QAction( IconManager::instance()->icon( "chevron-up") ,tr("Go up"),this));

      ACTIONS()->value(BROWSER_PREV)->setEnabled(false);
      ACTIONS()->value(BROWSER_NEXT)->setEnabled(false);

      /* browser search line edit */
      ui_search_lineedit = new SearchLineEdit(m_header);
      ui_search_lineedit->setInactiveText(tr("Quick filter"));
      ui_search_lineedit->setMinimumWidth(300);
      ui_search_lineedit->setMaximumWidth(300);
      ui_search_lineedit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
      new QShortcut(QKeySequence("Ctrl+F"), ui_search_lineedit, SLOT(slotFocus()));

      ui_save_button->setMaximumHeight(ui_search_lineedit->height());
      ui_cancel_button->setMaximumHeight(ui_search_lineedit->height());


      QMenu* menu = new QMenu();
      menu->addAction(ACTIONS()->value(APP_ENABLE_SEARCH_POPUP));
      menu->addAction(ACTIONS()->value(APP_PLAY_ON_SEARCH));
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
      //ui_button_prev->setArrowType(Qt::LeftArrow);
      ui_button_prev->setDefaultAction( ACTIONS()->value(BROWSER_PREV) );

      ui_button_next = new QToolButton(m_header);
      ui_button_next->setAutoRaise(true);
      //ui_button_next->setArrowType(Qt::RightArrow);
      ui_button_next->setDefaultAction( ACTIONS()->value(BROWSER_NEXT) );

      ui_button_up = new QToolButton(m_header);
      ui_button_up->setAutoRaise(true);
      //ui_button_up->setArrowType(Qt::UpArrow);
      ui_button_up->setDefaultAction( ACTIONS()->value(BROWSER_UP) );


    QHBoxLayout* h2 = new QHBoxLayout(m_header);
    h2->setSpacing(2);
    h2->setContentsMargins(4, 4, 4, 4);
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
      ui_editor_search = new EditorSearch(m_parent);
      QObject::connect(ui_editor_search, SIGNAL(search_triggered()), this, SLOT(slot_advanced_search_triggered()));
    }

    ui_editor_search->show();

    ui_editor_search->move (
      m_parent->mapToGlobal(QPoint(contentWidget()->width()-ui_editor_search->width(), m_header->height()))
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

    //Debug::debug() << "MainLeftWidget::slot_advanced_search_triggered: " << var;
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
    //Debug::debug() << "MainLeftWidget::browserSearch";

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

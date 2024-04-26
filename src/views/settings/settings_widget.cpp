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

#include "settings_widget.h"

#include "views/item_common.h"
#include "views/item_button.h"

#include "core/player/engine.h"
#include "core/database/database.h"

#include "online/lastfm.h"
#include "infosystem/services/service_lyrics.h"

#include "shortcuts_manager.h"
#include "utilities.h"
#include "settings.h"
#include "iconmanager.h"
#include "debug.h"

#include <QColorDialog>
#include <QFileInfo>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QCryptographicHash>

#include <QButtonGroup>
#include <QMenu>

/*
********************************************************************************
*                                                                              *
*    Class PageGeneral                                                         *
*                                                                              *
********************************************************************************
*/
PageGeneral::PageGeneral(QWidget* parentView) : QGraphicsWidget(0)
{
    m_parent       = parentView;
    isOpen         = true;

    //! create Gui
    createGui();
}


//! ----------- createGui ------------------------------------------------------
void PageGeneral::createGui()
{
    m_title   = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->m_name   = tr("General");


    m_button = new ButtonItem();
    m_button->setPos(0,0);
    m_button->setColor(QApplication::palette().color(QPalette::Base));
    m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
    m_button->setParentItem(this);

    connect(m_button, SIGNAL(clicked()), this, SLOT(slot_on_titlebutton_clicked()));


    // main widget
    QWidget* main_widget = new QWidget();

    main_widget->setAttribute(Qt::WA_OpaquePaintEvent, true);
    main_widget->setAutoFillBackground(true);

    QVBoxLayout* vl0 = new QVBoxLayout(main_widget);

    /*-------------------------------------------------*/
    /* General settings                                */
    /* ------------------------------------------------*/
    //! dbus & mpris parameter
    ui_check_enableMpris = new QCheckBox(main_widget);
    ui_check_enableMpris->setText(tr("Enable Mpris"));

    ui_check_enableDbus = new QCheckBox(main_widget);
    ui_check_enableDbus->setText( tr("Enable notification") );

    ui_check_enableHistory = new QCheckBox(main_widget);
    ui_check_enableHistory->setText( tr("Enable playing history") );

    //! systray parameter
    ui_check_systray = new QCheckBox(main_widget);
    ui_check_systray->setText(tr("Minimize application to systray"));

    ui_check_hideAtStartup = new QCheckBox(main_widget);
    ui_check_hideAtStartup->setText(tr("Hide window at startup"));

    ui_color_button = new QPushButton(main_widget);
    ui_color_button->setMinimumWidth(150);
    ui_color_button->setMaximumWidth(150);
    ui_color_button->setFlat(false);

    QPalette  pal;
    pal.setColor( QPalette::Active, QPalette::Button, SETTINGS()->_baseColor );
    pal.setColor( QPalette::Inactive, QPalette::Button, SETTINGS()->_baseColor );
    ui_color_button->setPalette(pal);

    ui_color_button->setText(tr("Choose color"));
    connect(this->ui_color_button, SIGNAL(clicked()), this, SLOT(slot_color_button_clicked()));

    /*-------------------------------------------------*/
    /* Graphical Interface settings                    */
    /* ------------------------------------------------*/
    //! layout
    vl0->addWidget(ui_check_enableMpris);
    vl0->addWidget(ui_check_enableDbus);
    vl0->addWidget(ui_check_enableHistory);
    vl0->addWidget(ui_check_systray);
    vl0->addWidget(ui_check_hideAtStartup);
    vl0->addWidget(ui_color_button);

    // proxy widget
    proxy_widget = new QGraphicsProxyWidget( this );
    proxy_widget->setWidget( main_widget );
    proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    m_title->setParentItem(this);
    m_title->setPos(0,0);
}

void PageGeneral::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
    //Debug::debug() << "## PageGeneral::resizeEvent";
this->update();

}

void PageGeneral::doLayout()
{
    m_button->setPos(qobject_cast<QGraphicsView*> (m_parent)->viewport()->width()-40,0);
    proxy_widget->setPos(50,30);

    updateGeometry();
}

void PageGeneral::update()
{
    prepareGeometryChange();

    doLayout();

    //updateGeometry();

    QGraphicsWidget::update();

    emit layout_changed();
}

QSizeF PageGeneral::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which);
Q_UNUSED(constraint);

  if(proxy_widget->isVisible())
    return QSize(
      proxy_widget->geometry().size().width(),
      proxy_widget->geometry().size().height() + 30
      );
  else
    return QSize( proxy_widget->geometry().size().width(), 30);
}

//! ----------- saveSettings ---------------------------------------------------
void PageGeneral::saveSettings()
{
    Debug::debug() << "PageGeneral::saveSettings";
    SETTINGS()->_useTrayIcon         = this->ui_check_systray->isChecked();
    SETTINGS()->_hideAtStartup       = this->ui_check_hideAtStartup->isChecked();
    SETTINGS()->_useDbusNotification = this->ui_check_enableDbus->isChecked();
    SETTINGS()->_useMpris            = this->ui_check_enableMpris->isChecked();
    SETTINGS()->_useHistory          = this->ui_check_enableHistory->isChecked();
}

//! ----------- restoreSettings ------------------------------------------------
void PageGeneral::restoreSettings()
{
    this->ui_check_systray->setChecked( SETTINGS()->_useTrayIcon );
    this->ui_check_hideAtStartup->setChecked( SETTINGS()->_hideAtStartup );
    this->ui_check_enableDbus->setChecked( SETTINGS()->_useDbusNotification );
    this->ui_check_enableMpris->setChecked( SETTINGS()->_useMpris );
    this->ui_check_enableHistory->setChecked( SETTINGS()->_useHistory );
}


bool PageGeneral::isSystrayChanged()
{
    return SETTINGS()->_useTrayIcon != ui_check_systray->isChecked();
}

bool PageGeneral::isDbusChanged()
{
    return SETTINGS()->_useDbusNotification != ui_check_enableDbus->isChecked();
}

bool PageGeneral::isMprisChanged()
{
    return SETTINGS()->_useMpris != ui_check_enableMpris->isChecked();
}

bool PageGeneral::isHistoryChanged()
{
    return SETTINGS()->_useHistory != ui_check_enableHistory->isChecked();
}


void PageGeneral::slot_color_button_clicked()
{
    QColor color = QColorDialog::getColor(SETTINGS()->_baseColor, m_parent);
    if (color.isValid())
    {
       SETTINGS()->_baseColor = color;
       SETTINGS()->updateCheckedColor();

       QPalette  pal;
       pal.setColor( QPalette::Active, QPalette::Button, color );
       pal.setColor( QPalette::Inactive, QPalette::Button, color );
       ui_color_button->setPalette(pal);
    }
}

void PageGeneral::setContentVisible(bool b)
{
    if(isOpen != b)
      slot_on_titlebutton_clicked();
}


void PageGeneral::slot_on_titlebutton_clicked()
{
    if(isOpen) {
      proxy_widget->hide();
      m_button->setPixmap(QPixmap(":/images/add_32x32.png"));
      m_button->update();
      isOpen = false;
    }
    else
    {
      proxy_widget->show();
      m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
      m_button->update();
      isOpen = true;
    }

    this->update();
}

/*
********************************************************************************
*                                                                              *
*    Class PagePlayer                                                          *
*                                                                              *
********************************************************************************
*/
PagePlayer::PagePlayer(QWidget* parentView) : QGraphicsWidget(0)
{
    m_parent         = parentView;
    isOpen           = true;
    _isEngineChanged = false;

    //! create Gui
    createGui();
}

//! ----------- createGui ------------------------------------------------------
void PagePlayer::createGui()
{
    m_title           = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->m_name   = tr("Player settings");

    m_button = new ButtonItem();
    m_button->setPos(0,0);
    m_button->setColor(QApplication::palette().color(QPalette::Base));
    m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
    m_button->setParentItem(this);

    connect(m_button, SIGNAL(clicked()), this, SLOT(slot_on_titlebutton_clicked()));

    // main widget
    QWidget* main_widget = new QWidget();

    main_widget->setAttribute(Qt::WA_OpaquePaintEvent, true);
    main_widget->setAutoFillBackground(true);

    /*-------------------------------------------------*/
    /* Player settings                                 */
    /* ------------------------------------------------*/
    ui_engineButton = new QPushButton(main_widget);
    ui_engineButton->setMaximumWidth(150);
    ui_engineButton->setMinimumWidth(150);

    QMenu *menu = new QMenu(ui_engineButton);
    menu->setMaximumWidth(150);
    menu->setMinimumWidth(150);
    menu->setContentsMargins(8,8,8,8);
    menu->setStyleSheet(
        QString ("QMenu {icon-size: 32px; background-color: none;border: none;} ")
    );

    ui_engineButton->setMenu(menu);

    ui_engineGroup = new QActionGroup(this);
    ui_engineGroup->setExclusive(true);

        QAction *a1 = new QAction(QIcon(), "vlc", this);
        a1->setData(QVariant::fromValue((int)ENGINE::VLC));
        a1->setIconVisibleInMenu(true);
        a1->setCheckable(true);
#ifndef ENABLE_VLC
        a1->setEnabled(false);
#endif
        QAction *a2 = new QAction(QIcon(), "mpv", this);
        a2->setData(QVariant::fromValue((int)ENGINE::MPV));
        a2->setIconVisibleInMenu(true);
        a2->setCheckable(true);
#ifndef ENABLE_MPV
        a2->setEnabled(false);
#endif
        QAction *a3 = new QAction(QIcon(), "phonon", this);
        a3->setData(QVariant::fromValue((int)ENGINE::PHONON));
        a3->setIconVisibleInMenu(true);
        a3->setCheckable(true);
#ifndef ENABLE_PHONON
        a3->setEnabled(false);
#endif
        QAction *a4 = new QAction(QIcon(), "qtmultimedia", this);
        a4->setData(QVariant::fromValue((int)ENGINE::QTMULTIMEDIA));
        a4->setIconVisibleInMenu(true);
        a4->setCheckable(true);
#ifndef ENABLE_QTMULTIMEDIA
        a4->setEnabled(false);
#endif
        QAction *a5 = new QAction(QIcon(), "no engine", this);
        a5->setData(QVariant::fromValue((int)ENGINE::NO_ENGINE));
        a5->setIconVisibleInMenu(true);
        a5->setCheckable(true);

        ui_engineGroup->addAction(a1);
        ui_engineGroup->addAction(a2);
        ui_engineGroup->addAction(a3);
        ui_engineGroup->addAction(a4);
        ui_engineGroup->addAction(a5);
        menu->addActions(ui_engineGroup->actions());

        foreach(QAction* a, ui_engineGroup->actions())
        {
           connect(a, SIGNAL(triggered()), this, SLOT(slot_engineClicked()));
        }


    ui_stopOnPlayqueueClear = new QCheckBox(main_widget);
    ui_stopOnPlayqueueClear->setText(tr("Stop playing on playqueue clear"));

    ui_restartPlayingAtStartup = new QCheckBox(main_widget);
    ui_restartPlayingAtStartup->setText(tr("Restart playing at startup"));

    ui_restorePlayqueue = new QCheckBox(main_widget);
    ui_restorePlayqueue->setText(tr("Restore last playqueue content at startup"));

    ui_enable_replaygain = new QCheckBox(main_widget);
    ui_enable_replaygain->setText(tr("Use ReplayGain"));
    connect(this->ui_enable_replaygain, SIGNAL(clicked()), this, SLOT(slot_enable_replaygain()));


    ui_comboRGMode = new QComboBox(main_widget);

    ui_comboRGMode->setMaximumWidth(150);
    ui_comboRGMode->setMinimumWidth(150);
    ui_comboRGMode->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    ui_comboRGMode->addItem("radio");
    ui_comboRGMode->addItem("album");

    QLabel *lbl1 = new QLabel(tr("Engine"), main_widget);
    lbl1->setFont(QFont("Arial",10,QFont::Bold));

    QLabel *lbl2 = new QLabel(tr("Replaygain"), main_widget);
    lbl2->setFont(QFont("Arial",10,QFont::Bold));

    QLabel *lbl3 = new QLabel(tr("Others"), main_widget);
    lbl3->setFont(QFont("Arial",10,QFont::Bold));

    QVBoxLayout* vl0 = new QVBoxLayout(main_widget);
    vl0->addWidget( lbl1 );
    vl0->addWidget( ui_engineButton );
    vl0->addItem( new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding) );
    vl0->addWidget( lbl2 );
    vl0->addWidget( ui_enable_replaygain );
    vl0->addWidget( ui_comboRGMode );
    vl0->addItem( new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding) );
    vl0->addWidget( lbl3 );
    vl0->addWidget( ui_stopOnPlayqueueClear );
    vl0->addWidget( ui_restartPlayingAtStartup );
    vl0->addWidget( ui_restorePlayqueue );


    // proxy widget
    proxy_widget = new QGraphicsProxyWidget( this );
    proxy_widget->setWidget( main_widget );
    proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );


    m_title->setParentItem(this);
    m_title->setPos(0,0);
}

void PagePlayer::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
    //Debug::debug() << "## PagePlayer::resizeEvent";
this->update();
}

void PagePlayer::doLayout()
{
    //Debug::debug() << "## PagePlayer::doLayout";
    m_button->setPos(qobject_cast<QGraphicsView*> (m_parent)->viewport()->width()-40,0);
    proxy_widget->setPos(50,30);
}

void PagePlayer::update()
{
    prepareGeometryChange();

    doLayout();

    updateGeometry();

    QGraphicsWidget::update();

    emit layout_changed();
}


QSizeF PagePlayer::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which);
Q_UNUSED(constraint);

  if(proxy_widget->isVisible())
    return QSize(
      proxy_widget->geometry().size().width(),
      proxy_widget->geometry().size().height() + 30
      );
  else
    return QSize( proxy_widget->geometry().size().width(), 30);
}


void PagePlayer::saveSettings()
{
    Debug::debug() << "PagePlayer::saveSettings";

    SETTINGS()->_stopOnPlayqueueClear    = this->ui_stopOnPlayqueueClear->isChecked();
    SETTINGS()->_restartPlayingAtStartup = this->ui_restartPlayingAtStartup->isChecked();
    SETTINGS()->_restorePlayqueue        = this->ui_restorePlayqueue->isChecked();

    if(!ui_enable_replaygain->isChecked())
        SETTINGS()->_replaygain = 0;
    else if(ui_comboRGMode->currentIndex() == 0)
        SETTINGS()->_replaygain = 1;
    else
        SETTINGS()->_replaygain = 2;

    /* save active engine */
    foreach(QAction* a, ui_engineGroup->actions())
    {
       if( a->isChecked() && (SETTINGS()->_engine != a->data().toInt()) )
       {
           _isEngineChanged = true;
           SETTINGS()->_engine = a->data().toInt();
           break;
       }
    }
}

void PagePlayer::restoreSettings()
{
    Debug::debug() << "PagePlayer::restoreSettings";

    this->ui_stopOnPlayqueueClear->setChecked( SETTINGS()->_stopOnPlayqueueClear );
    this->ui_restartPlayingAtStartup->setChecked( SETTINGS()->_restartPlayingAtStartup );
    this->ui_restorePlayqueue->setChecked( SETTINGS()->_restorePlayqueue );

    this->ui_enable_replaygain->setChecked( SETTINGS()->_replaygain != 0 );
    this->ui_comboRGMode->setCurrentIndex(SETTINGS()->_replaygain == 1 ? 0 : 1);
    this->ui_comboRGMode->setEnabled(ui_enable_replaygain->isChecked());

    /* restore active engine */
    foreach(QAction* a, ui_engineGroup->actions())
    {
        if( a->data().toInt() == Engine::activeEngine())
        {
            a->setChecked(true);
            ui_engineButton->setText(a->text());
            break;
        }
    }
}

void PagePlayer::slot_engineClicked()
{
    QAction *action = qobject_cast<QAction *>(sender());

    ui_engineButton->setText( action->text() );
}


void PagePlayer::slot_enable_replaygain()
{
    ui_comboRGMode->setEnabled(ui_enable_replaygain->isChecked());
}


void PagePlayer::setContentVisible(bool b)
{
    if(isOpen != b)
      slot_on_titlebutton_clicked();
}

void PagePlayer::slot_on_titlebutton_clicked()
{
    if(isOpen) {
      proxy_widget->hide();
      m_button->setPixmap(QPixmap(":/images/add_32x32.png"));
      m_button->update();
      isOpen = false;
    }
    else
    {
      proxy_widget->show();
      m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
      m_button->update();
      isOpen = true;
    }

    this->update();
}


/*
********************************************************************************
*                                                                              *
*    Class PageLibrary                                                         *
*                                                                              *
********************************************************************************
*/
PageLibrary::PageLibrary(QWidget* parentView) : QGraphicsWidget(0)
{
    m_parent       = parentView;
    isOpen         = true;

    //! create Gui
    createGui();

    //! connection
    connect(this->ui_add_path_button, SIGNAL(clicked()), this, SLOT(slot_on_add_folder_clicked()));

    connect(this->ui_auto_update, SIGNAL(stateChanged (int)), this, SLOT(slot_oncheckbox_clicked()));
    connect(this->ui_download_cover, SIGNAL(stateChanged (int)), this, SLOT(slot_oncheckbox_clicked()));
    connect(this->ui_search_cover, SIGNAL(stateChanged (int)), this, SLOT(slot_oncheckbox_clicked()));
    connect(this->ui_group_albums, SIGNAL(stateChanged (int)), this, SLOT(slot_oncheckbox_clicked()));
    connect(this->ui_use_artist_image, SIGNAL(stateChanged (int)), this, SLOT(slot_oncheckbox_clicked()));
    connect(this->ui_rating_to_file, SIGNAL(stateChanged (int)), this, SLOT(slot_oncheckbox_clicked()));


    connect(this->ui_choose_db, SIGNAL(currentIndexChanged(QString)), SLOT(loadDatabaseParam(QString)));
    connect(this->ui_db_new_button, SIGNAL(clicked()), this, SLOT(newDatabaseParam()));
    connect(this->ui_db_del_button, SIGNAL(clicked()), this, SLOT(delDatabaseParam()));
    connect(this->ui_db_rename_button, SIGNAL(clicked()), this, SLOT(renameDatabaseParam()));
}


//! ----------- createGui ------------------------------------------------------
void PageLibrary::createGui()
{
    m_title           = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->m_name   = tr("Library settings");


    m_button = new ButtonItem();
    m_button->setPos(0,0);
    m_button->setColor(QApplication::palette().color(QPalette::Base));
    m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
    m_button->setParentItem(this);

    connect(m_button, SIGNAL(clicked()), this, SLOT(slot_on_titlebutton_clicked()));


    // main widget
    m_main_widget = new QWidget();
    m_main_widget->setAttribute(Qt::WA_OpaquePaintEvent, true);
    m_main_widget->setAutoFillBackground(true);

    /*-------------------------------------------------*/
    /* Libray settings                                 */
    /* ------------------------------------------------*/

    //! label
    QLabel *lbl1 = new QLabel(tr("Collection"), m_main_widget);
    lbl1->setFont(QFont("Arial",10,QFont::Bold));

    //! horizontal Layout (database list + save + delete)
    ui_choose_db = new QComboBox(m_main_widget);
    ui_choose_db->setMaximumWidth(150);
    ui_choose_db->setMinimumWidth(150);
    ui_choose_db->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    ui_db_new_button = new QToolButton(m_main_widget);
    ui_db_new_button->setIcon(QIcon(":/images/add_32x32.png"));
    ui_db_new_button->setToolTip(tr("New database"));

    ui_db_del_button = new QToolButton(m_main_widget);
    ui_db_del_button->setIcon(QIcon::fromTheme("edit-delete"));
    ui_db_del_button->setToolTip(tr("Delete database"));

    ui_db_rename_button = new QToolButton(m_main_widget);
    ui_db_rename_button->setIcon(QIcon(":/images/rename-48x48.png"));
    ui_db_rename_button->setToolTip(tr("Rename database"));

    QHBoxLayout *hl1 = new QHBoxLayout();
    hl1->addWidget(ui_choose_db);
    hl1->addWidget(ui_db_new_button);
    hl1->addWidget(ui_db_del_button);
    hl1->addWidget(ui_db_rename_button);
    hl1->addItem( new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed) );

    /* ----- Database Properties ----- */
      QLabel *lbl2 = new QLabel(tr("Properties"), m_main_widget);
      lbl2->setFont(QFont("Arial",10,QFont::Bold));


      ui_folders_layout = new QVBoxLayout();
      ui_folders_layout->setSpacing(1);
      ui_folders_layout->setContentsMargins(0,0,0,0);
      ui_folders_layout->setSizeConstraint( QLayout::SetMinimumSize );

      AddFolderWidget* widget = new AddFolderWidget();
      widget->setMinimumWidth(350);
      widget->setText( tr(" Choose music folder") );

      ui_folders_layout->addWidget(widget);
      ui_folderWidgets.append(widget);
      QObject::connect(widget, SIGNAL(pathChanged()), this, SLOT(slot_on_path_changed()));

      ui_add_path_button = new QPushButton(m_main_widget);
      ui_add_path_button->setText(tr("Add ..."));

      /* Check box auto check database */
      ui_auto_update = new QCheckBox(m_main_widget);
      ui_auto_update->setText(tr("Update collections automatically after start"));

      /* Check box : group multiset albums */
      ui_group_albums = new QCheckBox(m_main_widget);
      ui_group_albums->setText(tr("Group multi disc albums as one album"));

      /* Check box : artist image */
      ui_use_artist_image = new QCheckBox(m_main_widget);
      ui_use_artist_image->setText(tr("Use artist image (album cover stack otherwise)"));

      /* Check box : artist image */
      ui_rating_to_file = new QCheckBox(m_main_widget);
      ui_rating_to_file->setText(tr("Write rating to file"));

      /* Images options */
      QLabel *lbl3 = new QLabel(tr("Image settings"), m_main_widget);
      lbl3->setFont(QFont("Arial",10,QFont::Bold));

      ui_cover_size_spinbox = new QSpinBox();
      ui_cover_size_spinbox->setMinimum(128);
      ui_cover_size_spinbox->setMaximum(256);
      ui_cover_size_spinbox->setValue(200);
      ui_cover_size_spinbox->setPrefix(tr("image size: "));
      ui_cover_size_spinbox->setFocusPolicy( Qt::NoFocus );

      /* Check box --> search cover into directory */
      ui_search_cover = new QCheckBox(m_main_widget);
      ui_search_cover->setText(tr("Search cover art from file directory"));

      ui_download_cover = new QCheckBox(m_main_widget);
      ui_download_cover->setText(tr("Download image (artist and album) from internet"));


    /* ----- Final Layout ----- */
      QVBoxLayout* layout = new QVBoxLayout( m_main_widget );
      layout->addWidget( lbl1 );
      layout->addLayout( hl1 );
      layout->addItem( new QSpacerItem(20, 15, QSizePolicy::Fixed, QSizePolicy::Fixed) );

      layout->addWidget( lbl2 );
      layout->addLayout( ui_folders_layout ,0);
      layout->addWidget( ui_add_path_button );
      layout->addWidget( ui_auto_update );
      layout->addWidget( ui_group_albums );
      layout->addWidget( ui_use_artist_image );
      layout->addWidget( ui_rating_to_file );
      layout->addItem( new QSpacerItem(20, 15, QSizePolicy::Fixed, QSizePolicy::Fixed) );

      layout->addWidget( lbl3 );
      layout->addWidget( ui_search_cover );
      layout->addWidget( ui_download_cover );
      layout->addWidget( ui_cover_size_spinbox );


      layout->setSizeConstraint(QLayout::SetMinimumSize );
      layout->addItem( new QSpacerItem(20, 15, QSizePolicy::Fixed, QSizePolicy::Expanding) );

    // proxy widget
    proxy_widget = new QGraphicsProxyWidget( this );
    proxy_widget->setWidget( m_main_widget );
    proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    m_title->setParentItem(this);
    m_title->setPos(0,0);
}

bool PageLibrary::isCoverSizeChanged()
{
    return ( SETTINGS()->_coverSize != ui_cover_size_spinbox->value() );
}


bool PageLibrary::isViewChanged()
{
    return ( Database::instance()->param()._option_group_albums != ui_group_albums->isChecked() );
}


void PageLibrary::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
//     Debug::debug() << "## PageLibrary::resizeEvent";
    this->update();
}

void PageLibrary::doLayout()
{
//     Debug::debug() << "## PageLibrary::doLayout";
    m_button->setPos(qobject_cast<QGraphicsView*> (m_parent)->viewport()->width()-40,0);
    proxy_widget->setPos(50,30);
}

void PageLibrary::update()
{
    prepareGeometryChange();

    m_main_widget->layout()->invalidate();
    doLayout();

    updateGeometry();

    QGraphicsWidget::update();

    emit layout_changed();
}

QSizeF PageLibrary::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which);
Q_UNUSED(constraint);

  if(proxy_widget->isVisible())
    return QSize(
      proxy_widget->geometry().size().width(),
      proxy_widget->geometry().size().height() + 30
      );
  else
    return QSize( proxy_widget->geometry().size().width(), 30);
}

//! ---- restoreSettings -------------------------------------------------------
void PageLibrary::restoreSettings()
{
    Debug::debug() << "PageLibrary::restoreSettings";

    //! read settings from Database
    Database *database = Database::instance();
    Debug::debug() << "PageLibrary::restoreSettings multi database" << database->isMultiDb();

    m_db_params.clear();
    ui_choose_db->clear();

    Debug::debug() << "PageLibrary::restoreSettings database current name" << database->param()._name;

    foreach (const QString& name, database->param_names()) {
      addDatabaseParam(name, database->param(name));
    }

    int selectedIdx = ui_choose_db->findText( database->param()._name );
    if (selectedIdx != -1)
      ui_choose_db->setCurrentIndex(selectedIdx);


    if(SETTINGS()->_coverSize < 120 || SETTINGS()->_coverSize > 256 )
    {
      SETTINGS()->_coverSize = 200;
    }

    ui_cover_size_spinbox->setValue( SETTINGS()->_coverSize );

    _isLibraryChanged = false;
}

//! ---- saveSettings -----------------------------------------------------------
void PageLibrary::saveSettings()
{
    Debug::debug() << "PageLibrary::saveSettings";

    //! save cover size
    SETTINGS()->_coverSize = ui_cover_size_spinbox->value();


    //! save settings file
    Database *database = Database::instance();

    database->param_clear();

    foreach (const QString& name, m_db_params.keys()) {
      database->param_add(m_db_params[name]);
    }

    database->change_database( ui_choose_db->currentText() );
    database->settings_save();
}


void PageLibrary::slot_oncheckbox_clicked()
{
    QCheckBox *cb = qobject_cast<QCheckBox *>(sender());
    if(!cb)
      return;

    const QString db_name = ui_choose_db->currentText();
    if( !m_db_params.contains(db_name))
      return;

    if( cb == ui_download_cover )
    {
      m_db_params[db_name]._option_download_cover = ui_download_cover->isChecked();
    }
    else if( cb == ui_search_cover )
    {
      m_db_params[db_name]._option_check_cover   = ui_search_cover->isChecked();
    }
    else if ( cb == ui_auto_update )
    {
      m_db_params[db_name]._option_auto_rebuild  = ui_auto_update->isChecked();
    }
    else if (cb == ui_group_albums)
    {
      m_db_params[db_name]._option_group_albums   = ui_group_albums->isChecked();
    }
    else if (cb == ui_use_artist_image)
    {
      m_db_params[db_name]._option_artist_image   = ui_use_artist_image->isChecked();
    }
    else if (cb == ui_rating_to_file)
    {
      m_db_params[db_name]._option_wr_rating_to_file   = ui_rating_to_file->isChecked();
    }
}


void PageLibrary::loadDatabaseParam(QString db_name)
{
    Debug::debug() << "PageLibrary::loadDatabaseParam";

    if(db_name.isEmpty()) return;

    if(m_db_params.contains(db_name))
    {
     //Debug::debug() << "SettingCollectionPage::loadDatabaseParam   database name = " << db_name;
     //Debug::debug() << "SettingCollectionPage::loadDatabaseParam   option_auto_rebuild = " << m_db_params[db_name]._option_auto_rebuild;
     //Debug::debug() << "SettingCollectionPage::loadDatabaseParam   option_check_cover = " << m_db_params[db_name]._option_check_cover;
     //Debug::debug() << "SettingCollectionPage::loadDatabaseParam   option_download_cover = " << m_db_params[db_name].option_download_cover;

      ui_auto_update->setChecked( m_db_params[db_name]._option_auto_rebuild );

      ui_search_cover->setChecked( m_db_params[db_name]._option_check_cover );
      ui_download_cover->setChecked( m_db_params[db_name]._option_download_cover );

      ui_group_albums->setChecked( m_db_params[db_name]._option_group_albums );
      ui_use_artist_image->setChecked( m_db_params[db_name]._option_artist_image );
      ui_rating_to_file->setChecked( m_db_params[db_name]._option_wr_rating_to_file );

      foreach(AddFolderWidget* widget, ui_folderWidgets) {
        ui_folders_layout->removeWidget(widget);
        const int index = ui_folderWidgets.indexOf(widget);
        ui_folderWidgets.takeAt(index)->deleteLater();
      }

      foreach (QString path, m_db_params[db_name]._paths) {
        slot_on_add_folder_clicked( path );
      }
     _isLibraryChanged = true;
   }
}


void PageLibrary::newDatabaseParam()
{
    QString currentDbName = ui_choose_db->currentText();

    DialogInput input(0, tr("New database"), tr("Name"));
    input.setEditValue(currentDbName);
    input.setFixedSize(480,140);


    if(input.exec() == QDialog::Accepted)
    {
      QString name = input.editValue();

       if (name.isEmpty()) return;

       if (m_db_params.contains(name))
       {
          DialogMessage dlg(0,tr("New database"));
          dlg.setMessage(tr("The database  \"%1\" already exists, please try another name").arg(name));
          dlg.resize(445, 120);
          dlg.exec();
          return;
       }

      Database::Param  param;
      param._option_auto_rebuild    = false;
      param._option_check_cover     = true;
      param._option_download_cover  = true;
      param._paths                  = QStringList();
      param._option_group_albums    = false;
      param._option_artist_image    = true;
      param._option_wr_rating_to_file = false;

      addDatabaseParam(name, param);

      ui_choose_db->setCurrentIndex(ui_choose_db->findText(name));
    }
}


void PageLibrary::addDatabaseParam(const QString& name, const Database::Param& param)
{
    Debug::debug() << "PageLibrary::addDatabaseParam";

    m_db_params[name]._name                     = name;
    m_db_params[name]._option_auto_rebuild      = param._option_auto_rebuild;
    m_db_params[name]._option_check_cover       = param._option_check_cover;
    m_db_params[name]._option_download_cover       = param._option_download_cover;
    m_db_params[name]._option_group_albums      = param._option_group_albums;
    m_db_params[name]._option_artist_image      = param._option_artist_image;
    m_db_params[name]._option_wr_rating_to_file = param._option_wr_rating_to_file;
    m_db_params[name]._paths                    = param._paths;

    Debug::debug() << "PageLibrary::addDatabaseParam  name" <<  name;
    Debug::debug() << "PageLibrary::addDatabaseParam  paths" << param._paths;
    Debug::debug() << "PageLibrary::addDatabaseParam  option_auto_rebuild" << param._option_auto_rebuild;
    Debug::debug() << "PageLibrary::addDatabaseParam  option_check_cover" << param._option_check_cover;
    Debug::debug() << "PageLibrary::addDatabaseParam  option_download_cover" << param._option_download_cover;
    Debug::debug() << "PageLibrary::addDatabaseParam  option_artist_image" << param._option_artist_image;
    Debug::debug() << "PageLibrary::addDatabaseParam  option_rating_tofile" << param._option_wr_rating_to_file;

    if (ui_choose_db->findText(name) == -1)
      ui_choose_db->addItem(name);
}


void PageLibrary::delDatabaseParam()
{
    Debug::debug() << "PageLibrary::delDatabaseParam";

    QString name = ui_choose_db->currentText();

    if (!m_db_params.contains(name) || name.isEmpty()) return;

    DialogQuestion dlg(0,tr("Delete database properties"));
    dlg.setQuestion(tr("Are you sure you want to delete the \"%1\" database?").arg(name));
    dlg.resize(445, 120);

    if(dlg.exec() == QDialog::Accepted)
    {
      m_db_params.remove(name);
      ui_choose_db->removeItem(ui_choose_db->currentIndex());
    }
}

void PageLibrary::renameDatabaseParam()
{
    //! rename an existing database
    Debug::debug() << "PageLibrary::renameDatabaseParam";

    QString oldName = ui_choose_db->currentText();


    DialogInput input(0, tr("Rename database"), tr("Name"));
    input.setFixedSize(480,140);
    input.setEditValue(oldName);

    if(input.exec() != QDialog::Accepted)
      return;

    QString newName = input.editValue();


    if (newName.isEmpty() || (newName == oldName) )
      return;

    if (m_db_params.contains(oldName))
    {
      Debug::debug() << "SettingCollectionPage::renameDatabaseParam";

      Database::Param dbParam = m_db_params.take(oldName);
      addDatabaseParam(newName, dbParam);
      ui_choose_db->removeItem(ui_choose_db->findText(oldName));
      //! TODO rename dabatase file, cover folder, playlist folder

      QCryptographicHash hash(QCryptographicHash::Sha1);
      hash.addData(oldName.toUtf8().constData());
      QFile oldDb(QString(UTIL::CONFIGDIR + "/" + hash.result().toHex() + ".db"));

      QCryptographicHash hash2(QCryptographicHash::Sha1);
      hash2.addData(newName.toUtf8().constData());

      if(oldDb.exists()) {
        oldDb.rename(QString(UTIL::CONFIGDIR + "/" + hash2.result().toHex() + ".db"));
      }
    }
}



void PageLibrary::slot_on_add_folder_clicked(const QString text/*=QString()*/)
{
    Debug::debug() << "PageLibrary::slot_on_add_folder_clicked";

    AddFolderWidget* widget = new AddFolderWidget();
    if(text.isEmpty())
      widget->setText( tr(" Choose music folder") );
    else
      widget->setText( text );

    ui_folders_layout->addWidget(widget);
    ui_folderWidgets.append(widget);
    QObject::connect(widget, SIGNAL(removedClicked()), this, SLOT(slot_on_remove_folder_clicked()));
    QObject::connect(widget, SIGNAL(pathChanged()), this, SLOT(slot_on_path_changed()));

    this->update();
}

void PageLibrary::slot_on_remove_folder_clicked()
{
    Debug::debug() << "PageLibrary::slot_on_remove_folder_clicked";

    AddFolderWidget * widget = qobject_cast<AddFolderWidget*>(sender());
    ui_folders_layout->removeWidget(widget);

    const int index = ui_folderWidgets.indexOf(widget);

    widget->disconnect();

    ui_folderWidgets.takeAt(index)->deleteLater();

    this->update();

    slot_on_path_changed();
}

void PageLibrary::slot_on_path_changed()
{
    Debug::debug() << "PageLibrary::slot_on_path_changed";

    QString dbName = ui_choose_db->currentText();
    m_db_params[dbName]._paths.clear();

    for (int i = 0; i < ui_folderWidgets.count(); i++)
    {
      const QString path = ui_folderWidgets.at(i)->path();
      if( QFileInfo(path).isDir() )
        m_db_params[dbName]._paths << path;
    }

    _isLibraryChanged = true;
}


void PageLibrary::setContentVisible(bool b)
{
    if(isOpen != b)
      slot_on_titlebutton_clicked();
}

void PageLibrary::slot_on_titlebutton_clicked()
{
    if(isOpen) {
      proxy_widget->hide();
      m_button->setPixmap(QPixmap(":/images/add_32x32.png"));
      m_button->update();
      isOpen = false;
    }
    else
    {
      proxy_widget->show();
      m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
      m_button->update();
      isOpen = true;
    }

    this->update();
}

/*
********************************************************************************
*                                                                              *
*    Class PageShortcut                                                        *
*                                                                              *
********************************************************************************
*/
PageShortcut::PageShortcut(QWidget* parentView) : QGraphicsWidget(0)
{
    m_parent       = parentView;
    isOpen         = true;

    createGui();

    connect(ShortcutsManager::instance(), SIGNAL(setting_changed()), this, SLOT(restoreSettings()));
}


PageShortcut::~PageShortcut()
{
}

void PageShortcut::createGui()
{
    //Debug::debug() << "###### PageShortcut createGui";

    m_title           = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->m_name   = tr("Shortcut settings");

    m_button = new ButtonItem();
    m_button->setPos(0,0);
    m_button->setColor(QApplication::palette().color(QPalette::Base));
    m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
    m_button->setParentItem(this);

    connect(m_button, SIGNAL(clicked()), this, SLOT(slot_on_titlebutton_clicked()));



    //! Check box to enable/disable shorcuts
    QWidget* main_widget = new QWidget();
    main_widget->setAttribute(Qt::WA_OpaquePaintEvent, true);
    main_widget->setAutoFillBackground(true);

    QVBoxLayout* vl0 = new QVBoxLayout(main_widget);
    ui_enable_shortcut = new QCheckBox();
    ui_enable_shortcut->setText(tr("Enable shortcuts"));
    vl0->addWidget(ui_enable_shortcut);

    proxy_widget = new QGraphicsProxyWidget( this );
    proxy_widget->setWidget( main_widget );
    proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );


    m_items["play"]        = new ShortcutGraphicItem(tr("Play/Pause"), SETTINGS()->_shortcutsKey["play"], IconManager::instance()->icon("media-play").pixmap(QSize(32,32)));
    m_items["stop"]        = new ShortcutGraphicItem(tr("Stop"), SETTINGS()->_shortcutsKey["stop"], IconManager::instance()->icon("media-stop").pixmap(QSize(32,32)));
    m_items["prev_track"]  = new ShortcutGraphicItem(tr("Previous track"), SETTINGS()->_shortcutsKey["prev_track"], IconManager::instance()->icon("media-prev").pixmap(QSize(32,32)));
    m_items["next_track"]  = new ShortcutGraphicItem(tr("Next track"), SETTINGS()->_shortcutsKey["next_track"], IconManager::instance()->icon("media-next").pixmap(QSize(32,32)));
    m_items["inc_volume"]  = new ShortcutGraphicItem(tr("Increase volume"), SETTINGS()->_shortcutsKey["inc_volume"], QPixmap(":/images/volume-icon.png"));
    m_items["dec_volume"]  = new ShortcutGraphicItem(tr("Decrease volume"), SETTINGS()->_shortcutsKey["dec_volume"], QPixmap(":/images/volume-icon.png"));
    m_items["mute_volume"] = new ShortcutGraphicItem(tr("Mute/Unmute volume"), SETTINGS()->_shortcutsKey["mute_volume"], QPixmap(":/images/volume-muted.png"));
    m_items["jump_to_track"] = new ShortcutGraphicItem(tr("Jump to track"), SETTINGS()->_shortcutsKey["jump_to_track"], IconManager::instance()->icon("goto").pixmap(QSize(32,32)));
    m_items["clear_playqueue"] = new ShortcutGraphicItem(tr("Clear playqueue"), SETTINGS()->_shortcutsKey["clear_playqueue"], QPixmap());

    connect(m_items.value("play"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("stop"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("prev_track"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("next_track"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("inc_volume"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("dec_volume"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("mute_volume"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("jump_to_track"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));
    connect(m_items.value("clear_playqueue"), SIGNAL(clicked()),this, SLOT(slot_on_shorcutItem_clicked()));

    m_items.value("play")->setParentItem(this);
    m_items.value("stop")->setParentItem(this);
    m_items.value("prev_track")->setParentItem(this);
    m_items.value("next_track")->setParentItem(this);
    m_items.value("inc_volume")->setParentItem(this);
    m_items.value("dec_volume")->setParentItem(this);
    m_items.value("mute_volume")->setParentItem(this);
    m_items.value("jump_to_track")->setParentItem(this);
    m_items.value("clear_playqueue")->setParentItem(this);

    m_title->setParentItem(this);
    m_title->setPos(0,0);

    // slots
    connect(this->ui_enable_shortcut, SIGNAL(clicked()), this, SLOT(enableChange()));
}

void PageShortcut::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
    //Debug::debug() << "## PageShortcut::resizeEvent";
    this->update();
}

void PageShortcut::doLayout()
{
    //Debug::debug() << "## PageShortcut::doLayout";
    m_button->setPos(qobject_cast<QGraphicsView*> (m_parent)->viewport()->width()-40,0);

    int Xpos = 50;
    int Ypos = 30;

    proxy_widget->setPos(Xpos,Ypos);
    Ypos += 50;

    m_items.value("play")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("stop")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("prev_track")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("next_track")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("inc_volume")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("dec_volume")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("mute_volume")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("jump_to_track")->setPos(Xpos,Ypos);
    Ypos += 35;
    m_items.value("clear_playqueue")->setPos(Xpos,Ypos);
}

void PageShortcut::update()
{
    prepareGeometryChange();

    doLayout();

    updateGeometry();

    QGraphicsWidget::update();

    emit layout_changed();
}

QSizeF PageShortcut::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which);
Q_UNUSED(constraint);

  if(m_items.value("play")->isVisible())
    return QSize( m_parent->width()-350,
                  proxy_widget->geometry().size().height() + 50 + m_items.size()*35 );
  else
    return QSize( m_parent->width()-350, 30);
}



//! ----------- restoreSettings ------------------------------------------------
void PageShortcut::restoreSettings()
{
    Debug::debug() << "PageShortcut restoreSettings";
    m_items["play"]->m_key            = SETTINGS()->_shortcutsKey["play"];
    m_items["stop"]->m_key            = SETTINGS()->_shortcutsKey["stop"];
    m_items["prev_track"]->m_key      = SETTINGS()->_shortcutsKey["prev_track"];
    m_items["next_track"]->m_key      = SETTINGS()->_shortcutsKey["next_track"];
    m_items["inc_volume"]->m_key      = SETTINGS()->_shortcutsKey["inc_volume"];
    m_items["dec_volume"]->m_key      = SETTINGS()->_shortcutsKey["dec_volume"];
    m_items["mute_volume"]->m_key     = SETTINGS()->_shortcutsKey["mute_volume"];
    m_items["jump_to_track"]->m_key   = SETTINGS()->_shortcutsKey["jump_to_track"];
    m_items["clear_playqueue"]->m_key = SETTINGS()->_shortcutsKey["clear_playqueue"];


    m_items["play"]->m_status            = ShortcutsManager::instance()->shortcuts().value("play").status;
    m_items["stop"]->m_status            = ShortcutsManager::instance()->shortcuts().value("stop").status;
    m_items["prev_track"]->m_status      = ShortcutsManager::instance()->shortcuts().value("prev_track").status;
    m_items["next_track"]->m_status      = ShortcutsManager::instance()->shortcuts().value("next_track").status;
    m_items["inc_volume"]->m_status      = ShortcutsManager::instance()->shortcuts().value("inc_volume").status;
    m_items["dec_volume"]->m_status      = ShortcutsManager::instance()->shortcuts().value("dec_volume").status;
    m_items["mute_volume"]->m_status     = ShortcutsManager::instance()->shortcuts().value("mute_volume").status;
    m_items["jump_to_track"]->m_status   = ShortcutsManager::instance()->shortcuts().value("jump_to_track").status;
    m_items["clear_playqueue"]->m_status = ShortcutsManager::instance()->shortcuts().value("clear_playqueue").status;


    _isEnableOld = SETTINGS()->_useShortcut;
    this->ui_enable_shortcut->setChecked(_isEnableOld);

    _isChanged = false;
    update();
}

//! ----------- saveSettings ------------------------------------------------
void PageShortcut::saveSettings()
{
    Debug::debug() << "PageShortcut::saveSettings";

    SETTINGS()->_shortcutsKey["play"]            = m_items["play"]->m_key;
    SETTINGS()->_shortcutsKey["stop"]            = m_items["stop"]->m_key;
    SETTINGS()->_shortcutsKey["prev_track"]      = m_items["prev_track"]->m_key;
    SETTINGS()->_shortcutsKey["next_track"]      = m_items["next_track"]->m_key;
    SETTINGS()->_shortcutsKey["inc_volume"]      = m_items["inc_volume"]->m_key;
    SETTINGS()->_shortcutsKey["dec_volume"]      = m_items["dec_volume"]->m_key;
    SETTINGS()->_shortcutsKey["mute_volume"]     = m_items["mute_volume"]->m_key;
    SETTINGS()->_shortcutsKey["jump_to_track"]   = m_items["jump_to_track"]->m_key;
    SETTINGS()->_shortcutsKey["clear_playqueue"] = m_items["clear_playqueue"]->m_key;


    SETTINGS()->_useShortcut = this->ui_enable_shortcut->isChecked();
}

void PageShortcut::slot_on_shorcutItem_clicked()
{
    //Debug::debug() << "PageShortcut slot_on_shorcutItem_clicked";

    ShortcutGraphicItem *item = qobject_cast<ShortcutGraphicItem *>(sender());
    if(!item) return;

    QString item_name = m_items.key(item);
    QString shortcut  = item->m_key;

    ShortcutDialog* dialog = new ShortcutDialog(item_name, shortcut, m_parent);


    if(dialog->exec() == QDialog::Accepted)
    {
      item->m_key = dialog->newShortcut();
      item->update();
     _isChanged = true;
    }

    delete dialog;
}

void PageShortcut::setContentVisible(bool b)
{
    if(isOpen != b)
      slot_on_titlebutton_clicked();
}


void PageShortcut::slot_on_titlebutton_clicked()
{
    if(isOpen)
    {
        proxy_widget->hide();
        foreach(ShortcutGraphicItem* item, m_items)
            item->hide();

        m_button->setPixmap(QPixmap(":/images/add_32x32.png"));
        m_button->update();
        isOpen = false;
    }
    else
    {
        proxy_widget->show();

        foreach(ShortcutGraphicItem* item, m_items)
            item->show();

        m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
        m_button->update();
        isOpen = true;
    }

    this->update();
}


//! ----------- enableChange ---------------------------------------------------
void PageShortcut::enableChange()
{
    //Debug::debug() << "PageShortcut::enableChange";
    bool isEnableNew = this->ui_enable_shortcut->isChecked();

    if(isEnableNew != _isEnableOld)
       _isChanged = true;
    else
       _isChanged = false;
}


/*
********************************************************************************
*                                                                              *
*    Class ShortcutGraphicItem                                                 *
*                                                                              *
********************************************************************************
*/
ShortcutGraphicItem::ShortcutGraphicItem(const QString& name, const QString& key, QPixmap pix)
{
    m_name   = name;
    m_key    = key;
    m_pixmap = pix.scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_status = true;

    setAcceptHoverEvents(true);
    setAcceptDrops(false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setCursor(Qt::PointingHandCursor);

    setGraphicsItem(this);
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

    //! option configuration
    QStyle *style =  QApplication::style();
    opt.widget = 0;
    opt.palette = QApplication::palette();
    opt.font = QApplication::font();
    opt.fontMetrics = QFontMetrics(opt.font);

    opt.showDecorationSelected = style->styleHint(QStyle::SH_ItemView_ShowDecorationSelected);
    int pm = style->pixelMetric(QStyle::PM_IconViewIconSize);

    opt.decorationSize = QSize(pm, pm);
    opt.decorationPosition = QStyleOptionViewItem::Top;
    opt.displayAlignment = Qt::AlignCenter;

    opt.locale.setNumberOptions(QLocale::OmitGroupSeparator);
    opt.state |= QStyle::State_Active;
    opt.state |= QStyle::State_Enabled;
    opt.state &= ~QStyle::State_Selected;
}

QRectF ShortcutGraphicItem::boundingRect() const
{
    return QRectF(0, 0, 450, 30);
}

// Inherited from QGraphicsLayoutItem
void ShortcutGraphicItem::setGeometry(const QRectF &geom)
{
    setPos(geom.topLeft());
}

// Inherited from QGraphicsLayoutItem
QSizeF ShortcutGraphicItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which);
Q_UNUSED(constraint);
    return boundingRect().size();
}

void ShortcutGraphicItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
Q_UNUSED(option)
   //! Draw frame for State_HasFocus item
   QStyle *style = widget ? widget->style() : QApplication::style();
   opt.rect = boundingRect().toRect();
   style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

   //! paint shortcut name
   painter->setPen(QApplication::palette().color(QPalette::Normal,QPalette::WindowText)) ;
   painter->setFont(opt.font);

   painter->drawPixmap(4, 3, m_pixmap);
   painter->drawText(QRect (32,0,300, 30), Qt::AlignLeft | Qt::AlignVCenter,m_name );
   painter->drawText(QRect (310,0,190, 30), Qt::AlignLeft | Qt::AlignVCenter,m_key );

   if(m_status == false)
     painter->drawPixmap(400, 5, QPixmap(":/images/media-broken-18x18.png") );
   else
     painter->drawPixmap(400, 5, QPixmap(":/images/checkmark-48x48.png").scaled(QSize(18,18), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void ShortcutGraphicItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    opt.state |= QStyle::State_MouseOver;
    this->update();
}

void ShortcutGraphicItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
Q_UNUSED(event)
    opt.state &= ~QStyle::State_MouseOver;
    this->update();
}

void ShortcutGraphicItem::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
Q_UNUSED(event)
    emit clicked();
}



/*
********************************************************************************
*                                                                              *
*    Class PageScrobbler                                                       *
*                                                                              *
********************************************************************************
*/
PageScrobbler::PageScrobbler(QWidget* parentView) : QGraphicsWidget(0)
{
    m_parent       = parentView;
    isOpen         = true;

    m_title           = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->m_name   = tr("Scrobbler settings");

    m_button = new ButtonItem();
    m_button->setPos(0,0);
    m_button->setColor(QApplication::palette().color(QPalette::Base));
    m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
    m_button->setParentItem(this);

    connect(m_button, SIGNAL(clicked()), this, SLOT(slot_on_titlebutton_clicked()));


    // main widget
    QWidget* main_widget = new QWidget();

    main_widget->setAttribute(Qt::WA_OpaquePaintEvent, true);
    main_widget->setAutoFillBackground(true);

    /*-------------------------------------------------*/
    /* Scrobbler settings widget                       */
    /* ------------------------------------------------*/
    QVBoxLayout* vl0 = new QVBoxLayout(main_widget);

    //! Check box for lastFm scrobbler enable/disable
    useLastFmScrobbler = new QCheckBox(main_widget);
    useLastFmScrobbler->setText(tr("Use LastFm scrobbler"));

    //! lastfm connexion
    QHBoxLayout* hl0 = new QHBoxLayout();
    statusLabel  = new QLabel(main_widget);
    statusPixmap = new QLabel(main_widget);
    hl0->addWidget(statusPixmap);
    hl0->addWidget(statusLabel);
    hl0->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    accountGroupB = new QGroupBox(tr("Account details"), main_widget);
    accountGroupB->setFlat(true);

    QGridLayout* gl = new QGridLayout(accountGroupB);
    gl->setContentsMargins(0, 0, 0, 0);

    QLabel* label_1 = new QLabel(tr("username"), accountGroupB);
    gl->addWidget(label_1, 0, 0, 1, 1);

    QLabel* label_2 = new QLabel(tr("password"), accountGroupB);
    gl->addWidget(label_2, 1, 0, 1, 1);

    lineEdit_1 = new QLineEdit(accountGroupB);
    lineEdit_1->setMinimumWidth(QFontMetrics(QFont()).horizontalAdvance("WWWWWWWWWWWW"));
    gl->addWidget(lineEdit_1, 0, 1, 1, 1);

    lineEdit_2 = new QLineEdit(accountGroupB);
    lineEdit_2->setEchoMode(QLineEdit::Password);
    gl->addWidget(lineEdit_2, 1, 1, 1, 1);

    //! sign in button
    signInButton = new QPushButton(main_widget);
    signInButton->setText(tr("Sign In"));

    //! connextion status
    vl0->addWidget(useLastFmScrobbler);
    vl0->addLayout(hl0);
    vl0->addWidget(accountGroupB);
    vl0->addWidget(signInButton);
    vl0->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // proxy widget
    proxy_widget = new QGraphicsProxyWidget( this );
    proxy_widget->setWidget( main_widget );
    proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    m_title->setParentItem(this);
    m_title->setPos(0,0);


    // scrobbler & slots
    connect(this->useLastFmScrobbler, SIGNAL(clicked()), this, SLOT(enableChange()));
    connect(this->signInButton, SIGNAL(clicked()), this, SLOT(signInClicked()));
    connect(LastFmService::instance(), SIGNAL(signInFinished()), this, SLOT(slotSignInDone()));
}

void PageScrobbler::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
this->update();
}

void PageScrobbler::doLayout()
{
    //Debug::debug() << "## PageScrobbler::doLayout";Minimum
    m_button->setPos(qobject_cast<QGraphicsView*> (m_parent)->viewport()->width()-40,0);
    proxy_widget->setPos(50,30);
}


void PageScrobbler::update()
{
    prepareGeometryChange();

    doLayout();

    updateGeometry();

    QGraphicsWidget::update();

    emit layout_changed();
}

QSizeF PageScrobbler::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which);
Q_UNUSED(constraint);

  if(proxy_widget->isVisible())
    return QSize(
      proxy_widget->geometry().size().width(),
      proxy_widget->geometry().size().height() + 30
      );
  else
    return QSize( proxy_widget->geometry().size().width(), 30);
}

//! ----------- saveSettings ---------------------------------------------------
void PageScrobbler::saveSettings()
{
    Debug::debug() << "PageScrobbler::saveSettings";

    SETTINGS()->_useLastFmScrobbler = this->useLastFmScrobbler->isChecked();

    LastFmService::instance()->saveSettings();
}

//! ----------- restoreSettings ------------------------------------------------
void PageScrobbler::restoreSettings()
{
    _isEnableOld = SETTINGS()->_useLastFmScrobbler;
    this->useLastFmScrobbler->setChecked(_isEnableOld);

    enableChange();

    updateSignInStatus();

    _isChanged = false;
}


//! ----------- enableChange ---------------------------------------------------
void PageScrobbler::enableChange()
{
    //Debug::debug() << "SettingScrobblerPage::enableChange";
    bool isEnableNew = this->useLastFmScrobbler->isChecked();

    if(isEnableNew != _isEnableOld)
       _isChanged = true;
    else
       _isChanged = false;

    // update state according to enable status
    statusLabel->setEnabled(isEnableNew);
    signInButton->setEnabled(isEnableNew);
    accountGroupB->setEnabled(isEnableNew);
}


//! ----------- getAuthentificationStatus --------------------------------------
void PageScrobbler::updateSignInStatus()
{
    bool auth = LastFmService::instance()->isAuthenticated();
    if(auth) {
      lineEdit_1->setText(LastFmService::instance()->username());
      lineEdit_2->clear();
      statusPixmap->setPixmap(QPixmap(":/images/checkmark-48x48.png").scaled(QSize(24,24)));
      //statusPixmap->setPixmap(QPixmap(":/images/signal_accepted-48x48.png").scaled(QSize(24,24)));
      statusLabel->setText(QString(tr("You are log in Last.fm service as <b>%1</b>").arg(LastFmService::instance()->username())));

      signInButton->setText(tr("Sign Out"));
    }
    else {
      statusPixmap->setPixmap(QPixmap(":/images/warning-48x48.png").scaled(QSize(24,24)));
      statusLabel->setText(tr("You are not logged in"));
      signInButton->setText(tr("Sign In"));
    }
}


//! ----------- signInClicked --------------------------------------------------
void PageScrobbler::signInClicked()
{
     Debug::debug() << "PageScrobbler::signInClicked";

    _isChanged = true;
    bool auth = LastFmService::instance()->isAuthenticated();
    if(auth) {
       lineEdit_1->clear();
       lineEdit_2->clear();
       LastFmService::instance()->signOut();
       updateSignInStatus();
    }
    else {
      LastFmService::instance()->signIn(lineEdit_1->text(), lineEdit_2->text());
    }
}



//! ----------- slotSignInDone -------------------------------------------------
void PageScrobbler::slotSignInDone()
{
    Debug::debug() << "PageScrobbler::slotSignInDone";

    bool success = LastFmService::instance()->isAuthenticated();
    updateSignInStatus();

    if (!success) {
        DialogMessage dlg(0,tr("Authentication failed"));
        dlg.setMessage( tr("Your Last.fm credentials were incorrect"));
        dlg.resize(445, 120);
        dlg.exec();
    }
}

void PageScrobbler::setContentVisible(bool b)
{
    if(isOpen != b)
      slot_on_titlebutton_clicked();
}

void PageScrobbler::slot_on_titlebutton_clicked()
{
    if(isOpen) {
      proxy_widget->hide();
      m_button->setPixmap(QPixmap(":/images/add_32x32.png"));
      m_button->update();
      isOpen = false;
    }
    else
    {
      proxy_widget->show();
      m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
      m_button->update();
      isOpen = true;
    }

    this->update();
}


/*
********************************************************************************
*                                                                              *
*    Class PageSongInfo                                                        *
*                                                                              *
********************************************************************************
*/
PageSongInfo::PageSongInfo(QWidget* parentView) : QGraphicsWidget(0)
{
    m_parent       = parentView;
    isOpen         = true;


    m_title   = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->m_name   = tr("Song info");


    m_button = new ButtonItem();
    m_button->setPos(0,0);
    m_button->setColor(QApplication::palette().color(QPalette::Base));
    m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
    m_button->setParentItem(this);

    connect(m_button, SIGNAL(clicked()), this, SLOT(slot_on_titlebutton_clicked()));


    // main widget
    QWidget* main_widget = new QWidget();
    main_widget->setAttribute(Qt::WA_OpaquePaintEvent, true);
    main_widget->setAutoFillBackground(true);


    QLabel *ui_label = new QLabel(tr("Choose the websites you want to use when searching for lyrics"), main_widget);

    ui_listwidget = new QListWidget();
    ui_listwidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    ui_move_up   = new QPushButton(tr("move up"), main_widget);
    ui_move_down = new QPushButton(tr("move down"), main_widget);

    ui_move_up->setEnabled(false);
    ui_move_down->setEnabled(false);

    QVBoxLayout* vl1 = new QVBoxLayout();
    vl1->addWidget(ui_move_up);
    vl1->addWidget(ui_move_down);
    vl1->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QHBoxLayout* hl0 = new QHBoxLayout();
    hl0->addWidget(ui_listwidget);
    hl0->addLayout(vl1);

    QVBoxLayout* vl0 = new QVBoxLayout(main_widget);
    vl0->addWidget(ui_label);
    vl0->addLayout(hl0);

    connect(ui_listwidget, SIGNAL(itemChanged(QListWidgetItem*)),SLOT(slot_item_changed(QListWidgetItem*)));

    connect(ui_listwidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
          SLOT(slot_current_item_changed(QListWidgetItem*)));


    connect(ui_move_up, SIGNAL(clicked()), this, SLOT(slot_move_up()));
    connect(ui_move_down, SIGNAL(clicked()), this, SLOT(slot_move_down()));

    // proxy widget
    proxy_widget = new QGraphicsProxyWidget( this );
    proxy_widget->setWidget( main_widget );
    proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    m_title->setParentItem(this);
    m_title->setPos(0,0);
}

void PageSongInfo::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
    //Debug::debug() << "## PageSongInfo::resizeEvent";
this->update();
}

void PageSongInfo::doLayout()
{
    m_button->setPos(qobject_cast<QGraphicsView*> (m_parent)->viewport()->width()-40,0);
    proxy_widget->setPos(50,30);
}

void PageSongInfo::update()
{
    prepareGeometryChange();

    doLayout();

    updateGeometry();

    QGraphicsWidget::update();

    emit layout_changed();
}

QSizeF PageSongInfo::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which);
Q_UNUSED(constraint);

  if(proxy_widget->isVisible())
    return QSize(
      proxy_widget->geometry().size().width(),
      proxy_widget->geometry().size().height() + 30
      );
  else
    return QSize( proxy_widget->geometry().size().width(), 30);
}

//! ----------- saveSettings ---------------------------------------------------
void PageSongInfo::saveSettings()
{
    Debug::debug() << "PageSongInfo::saveSettings";
    QStringList search_order;
    for (int i=0 ; i < ui_listwidget->count() ; ++i)
    {
        const QListWidgetItem* item = ui_listwidget->item(i);
        if (item->checkState() == Qt::Checked)
            search_order << item->text();
    }

    SETTINGS()->_lyrics_providers = search_order;
}

//! ----------- restoreSettings ------------------------------------------------
void PageSongInfo::restoreSettings()
{
    /* loop over user activated providers */
    foreach(const QString& provider_name, SETTINGS()->_lyrics_providers)
    {
      foreach(const QString& name, ServiceLyrics::fullProvidersList())
      {
         if (provider_name == name)
         {
          QListWidgetItem* item = new QListWidgetItem(ui_listwidget);
          item->setText(provider_name);
          item->setCheckState(Qt::Checked);
          item->setForeground(palette().color(QPalette::Active, QPalette::Text));
         }
      }
    }

    foreach(const QString& name, ServiceLyrics::fullProvidersList())
    {
        if( !SETTINGS()->_lyrics_providers.contains( name ) )
        {
          QListWidgetItem* item = new QListWidgetItem(ui_listwidget);
          item->setText(name);
          item->setCheckState(Qt::Unchecked);
          item->setForeground(palette().color(QPalette::Disabled, QPalette::Text));
        }
    }
}

void PageSongInfo::setContentVisible(bool b)
{
    if(isOpen != b)
      slot_on_titlebutton_clicked();
}

void PageSongInfo::slot_on_titlebutton_clicked()
{
    if(isOpen) {
      proxy_widget->hide();
      m_button->setPixmap(QPixmap(":/images/add_32x32.png"));
      m_button->update();
      isOpen = false;
    }
    else
    {
      proxy_widget->show();
      m_button->setPixmap(QPixmap(":/images/remove_32x32.png"));
      m_button->update();
      isOpen = true;
    }

    this->update();
}

void PageSongInfo::slot_item_changed(QListWidgetItem* item)
{
  const bool checked = item->checkState() == Qt::Checked;
  item->setForeground(checked ? palette().color(QPalette::Active, QPalette::Text)
                              : palette().color(QPalette::Disabled, QPalette::Text));
}

void PageSongInfo::slot_current_item_changed(QListWidgetItem* item)
{
  if (!item) {
    ui_move_up->setEnabled(false);
    ui_move_down->setEnabled(false);
  } else {
    const int row = ui_listwidget->row(item);
    ui_move_up->setEnabled(row != 0);
    ui_move_down->setEnabled(row != ui_listwidget->count() - 1);
  }
}

void PageSongInfo::slot_move_up()
{
    Move(-1);
}

void PageSongInfo::slot_move_down()
{
    Move(+1);
}

void PageSongInfo::Move(int d)
{
  const int row = ui_listwidget->currentRow();
  QListWidgetItem* item = ui_listwidget->takeItem(row);
  ui_listwidget->insertItem(row + d, item);
  ui_listwidget->setCurrentRow(row + d);
}

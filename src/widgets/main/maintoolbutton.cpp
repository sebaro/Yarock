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

#include "maintoolbutton.h"
#include "threadmanager.h"
#include "core/database/database.h"
#include "playqueue/playqueue_model.h"

#include "settings.h"
#include "global_actions.h"
#include "debug.h"

#include <QApplication>
/*
********************************************************************************
*                                                                              *
*    Class MainToolButton                                                      *
*                                                                              *
********************************************************************************
*/
MainToolButton::MainToolButton( QWidget* parent ) : QToolButton(parent)
{
    this->setPalette(QApplication::palette());
    this->setAutoRaise(true);
    this->setIcon(QIcon(":/images/menu_48x48.png"));
    this->setToolTip(tr("Tools"));
    this->setPopupMode(QToolButton::InstantPopup);

    m_menuChooseDbAction = 0;
    m_menuChooseDb = new QMenu(tr("Choose database"));

    QMap<ENUM_ACTION, QAction*> *actions = ACTIONS();

    m_menu = new QMenu(this);

    QMenu *m0 = m_menu->addMenu(tr("&Show/Hide panel"));
    m0->addAction(actions->value(APP_SHOW_PLAYQUEUE));
    m_menu->addSeparator();

    QMenu *m1 = m_menu->addMenu(tr("&Add to playqueue"));
    m1->addAction(actions->value(PLAYQUEUE_ADD_FILE));
    m1->addAction(actions->value(PLAYQUEUE_ADD_DIR));
    m1->addAction(actions->value(PLAYQUEUE_ADD_URL));

    m_menu->addAction(actions->value(PLAYQUEUE_CLEAR));
    m_menu->addAction(actions->value(PLAYQUEUE_EXPORT));
    m_menu->addAction(actions->value(PLAYQUEUE_AUTOSAVE));

    m_menu->addSeparator();
    m_menu->addAction(actions->value(DATABASE_OPERATION));
    m_menu->addSeparator();
    m_menu->addAction(actions->value(APP_SHOW_SETTINGS));
    m_menu->addAction(actions->value(APP_SHOW_YAROCK_ABOUT));
    m_menu->addAction(actions->value(APP_MODE_COMPACT));

    m_menu->addSeparator();
    m_menu->addAction(actions->value(APP_QUIT));

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL( clicked ( bool ) ), this, SLOT( slot_showMenu() ) );
}


void MainToolButton::slot_showMenu()
{
    //Debug::debug() << "      [MainToolButton] slot_showMenu";
    updatePlayqueueActions();

    updateMultiDbMenu();

    QPoint pos( 0, height() );
    m_menu->exec( mapToGlobal( pos ) );
}

void MainToolButton::updatePlayqueueActions()
{
    const bool isPlaylistEmpty = Playqueue::instance()->rowCount(QModelIndex()) < 1;
    ACTIONS()->value(PLAYQUEUE_CLEAR)->setEnabled(!isPlaylistEmpty);
    ACTIONS()->value(PLAYQUEUE_EXPORT)->setEnabled(!isPlaylistEmpty);
    ACTIONS()->value(PLAYQUEUE_AUTOSAVE)->setEnabled(!isPlaylistEmpty);
}



void MainToolButton::updateMultiDbMenu()
{
    //Debug::debug() << "      [MainToolButton] updateMultiDbMenu";
    if( !Database::instance()->isMultiDb() )
    {
        if(m_menuChooseDbAction != 0)
          m_menu->removeAction ( m_menuChooseDbAction );

        m_menuChooseDbAction = 0;
        return;
    }

    //! Multi Db activated --> show choose database menu
    QActionGroup* group = new QActionGroup(this);

    foreach ( const QString& name, Database::instance()->param_names() )
    {
        QAction *a = new QAction(QIcon(), name, this);
        a->setCheckable(true);

        group->addAction(a);

        if(name == Database::instance()->param()._name)
          a->setChecked(true);

        connect(a, SIGNAL(triggered()), this, SLOT(slot_dbNameClicked()));
    }

    m_menuChooseDb->clear();
    m_menuChooseDb->addActions(group->actions());

    if(!m_menuChooseDbAction)
       m_menuChooseDbAction = m_menu->insertMenu ( ACTIONS()->value(DATABASE_OPERATION), m_menuChooseDb );

    m_menuChooseDb->setEnabled(!ThreadManager::instance()->isDbRunning());
}


void MainToolButton::slot_dbNameClicked()
{
    QAction *action = qobject_cast<QAction *>(sender());

    Database::instance()->change_database( action->text() );
    
    //Debug::debug() << "      [MainToolButton] dbNameClicked = " << action->text();
    
    emit dbNameChanged();
}

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

#include "menumodel.h"
#include "core/database/database.h"
#include "views.h"

#include "global_actions.h"
#include "debug.h"
#include "iconmanager.h"

#include <QDir>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QAction>
#include <QActionGroup>
#include <QStandardItem>



/*
********************************************************************************
*                                                                              *
*    Class MenuItem                                                            *
*                                                                              *
********************************************************************************
*/
class MenuItem: public QStandardItem
{
  public:
     MenuItem(const QString & text, const QIcon & icon);
     MenuItem(QAction* action, MenuItemRole role);
     MenuItem(const QString & text, const QIcon & icon , VIEW::Id id, QVariant data=QVariant());
};


MenuItem::MenuItem(const QString & text, const QIcon & icon ) : QStandardItem(icon,text)
{
    setEditable(false);
    setSelectable(true);
}

MenuItem::MenuItem(QAction *action, MenuItemRole role) : QStandardItem(action->icon(),action->text())
{
    setData(QVariant::fromValue(action), role);

    setEditable(false);
    setSelectable(true);
}

MenuItem::MenuItem(const QString & text, const QIcon & icon , VIEW::Id id, QVariant data) : QStandardItem(icon,text)
{
    setEditable(false);
    setSelectable(true);

    setData(QVariant::fromValue(new QAction(icon, text, MenuModel::instance())), MenuActionRole);
    setData(QVariant(int(id)), ViewModeRole);

    if(!data.isNull())
      setData(data, DataRole);
}



MenuModel* MenuModel::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class MenuModel                                                           *
*                                                                              *
********************************************************************************
*/
MenuModel::MenuModel(QObject *parent) : QStandardItemModel(parent)
{
    INSTANCE = this;

    populateMenu();

    configureMenuAction();

    connect(Database::instance(), SIGNAL(settingsChanged()), this, SLOT(slot_database_settingsChanged()));
}


MenuModel::~MenuModel()
{
}


/* ---------------------------------------------------------------------------*/
/* MenuModel::modelIndexAction                                                */
/* ---------------------------------------------------------------------------*/
QAction* MenuModel::modelIndexAction(QModelIndex idx)
{
    if( idx.data(GlobalActionRole).value<QAction*>() )
    {
        return idx.data(GlobalActionRole).value<QAction*>();
    }
    else if (idx.data(MenuActionRole).value<QAction*>() )
    {
        return idx.data(MenuActionRole).value<QAction*>();
    }
    else if (idx.data(DatabaseChooseRole).value<QAction*>() )
    {
        return idx.data(DatabaseChooseRole).value<QAction*>();
    }

    return 0;
}

/* ---------------------------------------------------------------------------*/
/* MenuModel::populateMenu                                                    */
/* ---------------------------------------------------------------------------*/
void MenuModel::populateMenu()
{
   QStandardItem *rootItem = this->invisibleRootItem();

   /* -----------------------*/
   /* populate Root Item     */
   /* -----------------------*/
   MenuItem* item1 = new MenuItem(tr("home"), IconManager::instance()->icon( "home"));
   rootItem->appendRow(item1);

   MenuItem* item2 = new MenuItem(tr("music browser"), IconManager::instance()->icon( "music"));
   rootItem->appendRow(item2);

   MenuItem* item3 = new MenuItem(tr("playlist browser"), IconManager::instance()->icon( "playlist1"));
   rootItem->appendRow(item3);

   MenuItem* item4 = new MenuItem(tr("radio browser"), IconManager::instance()->icon( "globe"));
   rootItem->appendRow(item4);

   MenuItem* item5 = new MenuItem(tr("computer"), IconManager::instance()->icon( "desktop"));
   rootItem->appendRow(item5);

   MenuItem* item6 = new MenuItem(tr("settings"), IconManager::instance()->icon( "setting"));
   rootItem->appendRow(item6);

   m_settings_item = item6;

   /* -----------------------*/
   /* populate Home Item     */
   /* -----------------------*/
   MenuItem* item11 = new MenuItem(tr("context"), QIcon(":/images/info-48x48.png"), VIEW::ViewContext);
   item1->appendRow(item11);

   MenuItem* item12 = new MenuItem(tr("dashboard"), QIcon(":/images/chart-48x48.png"), VIEW::ViewDashBoard);
   item1->appendRow(item12);

   MenuItem* item13 = new MenuItem(tr("history"), QIcon(":/images/history-48x48.png"), VIEW::ViewHistory);
   item1->appendRow(item13);


   /* -------------------------*/
   /* populate Collection Item */
   /* -------------------------*/

   MenuItem* item21 = new MenuItem(tr("artists"), IconManager::instance()->icon( "artist"), VIEW::ViewArtist);
   item2->appendRow(item21);

   MenuItem* item22 = new MenuItem(tr("albums"), QIcon(":/images/album.png"), VIEW::ViewAlbum);
   item2->appendRow(item22);

   MenuItem* item23 = new MenuItem(tr("tracks"), IconManager::instance()->icon( "track"), VIEW::ViewTrack);
   item2->appendRow(item23);

   MenuItem* item24 = new MenuItem(tr("genre"), QIcon(":/images/genre.png"), VIEW::ViewGenre);
   item2->appendRow(item24);

   MenuItem* item25 = new MenuItem(tr("years"), QIcon(":/images/date-48x48.png"), VIEW::ViewYear);
   item2->appendRow(item25);

   MenuItem* item26 = new MenuItem(tr("favorites"), QIcon(":/images/favorites-48x48.png"), VIEW::ViewFavorite);
   item2->appendRow(item26);

   /* -------------------------*/
   /* populate Paylist Browser */
   /* -------------------------*/
   MenuItem* item31 = new MenuItem(tr("playlists"), IconManager::instance()->icon( "playlist1"), VIEW::ViewPlaylist);
   item3->appendRow(item31);

   MenuItem* item32 = new MenuItem(tr("smart playlists"), QIcon(":/images/smart-playlist-48x48.png"), VIEW::ViewSmartPlaylist);
   item3->appendRow(item32);

   MenuItem* item33 = new MenuItem( ACTIONS()->value(NEW_PLAYLIST), GlobalActionRole);
   item3->appendRow(item33);

   MenuItem* item34 = new MenuItem( ACTIONS()->value(NEW_SMART_PLAYLIST), GlobalActionRole);
   item3->appendRow(item34);

   /* -----------------------*/
   /* populate Radio Item    */
   /* -----------------------*/
   /* add TuneIn */
   MenuItem *item41 = new MenuItem("tunein", QIcon(":/images/tunein_48x48.png"), VIEW::ViewTuneIn);
   item4->appendRow(item41);

   /* add Radionomy */
   MenuItem *item42 = new MenuItem("radionomy", QIcon(":/images/radionomy.png"), VIEW::ViewRadionomy);
   item4->appendRow(item42);

   /* add Dirble */
   MenuItem *item43 = new MenuItem("dirble", QIcon(":/images/dirble.png"), VIEW::ViewDirble);
   item4->appendRow(item43);

   /* add Favorite*/
   MenuItem *item44 = new MenuItem(tr("favorite stream"), QIcon(":/images/favorites-48x48.png"), VIEW::ViewFavoriteRadio);
   item4->appendRow(item44);


   /* -----------------------*/
   /* populate Computer Item */
   /* -----------------------*/
   MenuItem* item51 = new MenuItem(tr("home directory"), QIcon(":/images/folder-48x48.png"), VIEW::ViewFileSystem, QVariant(QDir::homePath()));
   item5->appendRow(item51);

   MenuItem* item52 = new MenuItem(tr("root directory"), QIcon(":/images/folder-48x48.png"), VIEW::ViewFileSystem, QVariant(QDir::rootPath()));
   item5->appendRow(item52);


   /* -----------------------*/
   /* populate Settings Item */
   /* -----------------------*/
   MenuItem* item60 = new MenuItem(tr("settings"), IconManager::instance()->icon("setting"), VIEW::ViewSettings);
   item6->appendRow(item60);

   MenuItem* item61 = new MenuItem(tr("about"), QIcon(":/images/about-48x48.png"), VIEW::ViewAbout);
   item6->appendRow(item61);


   MenuItem* item63 = new MenuItem( ACTIONS()->value(DATABASE_OPERATION), GlobalActionRole);
   item6->appendRow( item63 );

   MenuItem* item64 = new MenuItem( ACTIONS()->value(DATABASE_ADD), GlobalActionRole);
   item6->appendRow( item64 );

   m_db_item = new MenuItem(tr("Choose database"), QIcon() );
   item6->appendRow( m_db_item );

   MenuItem* item65 = new MenuItem( ACTIONS()->value(APP_SHOW_PLAYQUEUE), GlobalActionRole);
   item6->appendRow( item65 );

   MenuItem* item66 = new MenuItem( ACTIONS()->value(APP_MODE_COMPACT), GlobalActionRole);
   item6->appendRow( item66 );

   populateChooseDatabase();
}

/* ---------------------------------------------------------------------------*/
/* MenuModel::populateChooseDatabase                                          */
/* ---------------------------------------------------------------------------*/
void MenuModel::populateChooseDatabase()
{
    m_db_item->removeRows(0, m_db_item->rowCount() );

    QActionGroup* group = new QActionGroup(this);

    foreach ( const QString& name, Database::instance()->param_names() )
    {
        QAction *a = new QAction(QIcon(), name, this);
        a->setCheckable(true);

        if(name == Database::instance()->param()._name)
          a->setChecked(true);

         group->addAction(a);

        MenuItem* mi = new MenuItem( a, DatabaseChooseRole);
        m_db_item->appendRow( mi );

        connect(a, SIGNAL(triggered()), this, SLOT(slot_dbNameClicked()));
    }
}

QModelIndex MenuModel::settingsModelIdx()
{
    return indexFromItem( m_settings_item );
}

/* ---------------------------------------------------------------------------*/
/* MenuModel::slot_database_settingsChanged                                   */
/* ---------------------------------------------------------------------------*/
void MenuModel::slot_database_settingsChanged()
{
    //Debug::debug() << "MenuModel::slot_database_settingsChanged()";

    populateChooseDatabase();

    emit databaseMenuChanged();
}


/* ---------------------------------------------------------------------------*/
/* MenuModel::configureMenuAction                                             */
/* ---------------------------------------------------------------------------*/
void MenuModel::configureMenuAction()
{
   for (int i=0; i < this->rowCount(QModelIndex()); i++)
   {
    const QModelIndex childIdx = this->index(i, 0, QModelIndex());

    QStandardItem *childItem = itemFromIndex( childIdx );

    for (int j=0; j < childItem->rowCount(); j++)
    {
         const QModelIndex child2Idx = this->index(j, 0, childIdx);

         if( QAction *a = child2Idx.data(MenuActionRole).value<QAction*>() )
         {
           a->setData(QVariant::fromValue(child2Idx));
           connect(a, SIGNAL(triggered()),this, SLOT(slot_on_menu_triggered()));
         }
      }
  }
}

/* ---------------------------------------------------------------------------*/
/* MenuModel::slot_dbNameClicked                                              */
/* ---------------------------------------------------------------------------*/
void MenuModel::slot_dbNameClicked()
{
     QAction *action = qobject_cast<QAction *>(sender());

     Database::instance()->change_database( action->text() );

     //Debug::debug() << "      [MenuModel] dbNameClicked = " << action->text();

     emit dbNameChanged();
}

/* ---------------------------------------------------------------------------*/
/* MenuModel::slot_on_menu_triggered                                          */
/*   -> Quand une action provenant du menu (action associ� au item et visible */
/*   par les menu de la menubar --> on retrouve l'index dans le modele        */
/* ---------------------------------------------------------------------------*/
void MenuModel::slot_on_menu_triggered()
{
   QAction *action = qobject_cast<QAction *>(sender());
   if(!action) return;

   QModelIndex idx = action->data().value<QModelIndex>();

   emit menu_browser_triggered(VIEW::Id (idx.data(ViewModeRole).toInt()), idx.data(DataRole));
}


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

#include "dialogs/database_add.h"

#include "addfolder_widget.h"
#include "exlineedit.h"
#include "database.h"
#include "debug.h"

#include <QDesktopServices>
#include <QDir>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QApplication>

/*
********************************************************************************
*                                                                              *
*    Class DatabaseAddDialog                                                   *
*                                                                              *
********************************************************************************
*/
DatabaseAddDialog::DatabaseAddDialog(QWidget *parent) : DialogBase(parent, tr("Add music collection"))
{
    /* -- content ui -- */
    this->resize(445, 150);

    buttonBox()->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    ui_line_edit = new ExLineEdit(this);
    ui_line_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui_line_edit->clearFocus();
    ui_line_edit->setInactiveText(tr("Choose collection name"));
    
    ui_add_folder = new AddFolderWidget();
    ui_add_folder->hideRemoveButton();
    ui_add_folder->setMinimumWidth(350);
    
    ui_info_label = new QLabel();
    ui_info_label->setStyleSheet("QLabel { color : red; }");
    
    /* --- layout --- */
    QGridLayout *grid_layout = new QGridLayout();
    grid_layout->setContentsMargins(0, 0, 0, 0);

    grid_layout->addWidget(new QLabel( tr("Name") ), 0, 0, 1, 1);
    grid_layout->addWidget(ui_line_edit, 0, 1, 1, 1);
    
    grid_layout->addWidget(new QLabel( tr("Path") ), 1, 0, 1, 1);
    grid_layout->addWidget(ui_add_folder, 1, 1, 1, 1);

    grid_layout->addWidget(ui_info_label, 2, 1, 1, 2);
    
    setContentLayout( grid_layout );
    
    /* -- initialization -- */
#if QT_VERSION < 0x050000
    QDir userMusicDir = QDir( QDesktopServices::storageLocation( QDesktopServices::MusicLocation ) );
#else
    QDir userMusicDir = QDir( QStandardPaths::writableLocation( QStandardPaths::MusicLocation ) );
#endif

    if(userMusicDir.exists())
      ui_add_folder->setText( userMusicDir.path() );
    else
      ui_add_folder->setText( tr(" Choose music folder") );

    isFirstStart = false;

    /* -- connection -- */
    QObject::connect(buttonBox(), SIGNAL(accepted()), this, SLOT(on_buttonBox_accepted()));
    QObject::connect(buttonBox(), SIGNAL(rejected()), this, SLOT(on_buttonBox_rejected()));    
}

/* ---------------------------------------------------------------------------*/
/* DatabaseAddDialog::showEvent                                               */
/* ---------------------------------------------------------------------------*/
QString DatabaseAddDialog::newDatabaseName()
{
    return ui_line_edit->text();
}


/* ---------------------------------------------------------------------------*/
/* DatabaseAddDialog::showEvent                                               */
/* ---------------------------------------------------------------------------*/
void DatabaseAddDialog::showEvent ( QShowEvent * event )
{
   QWidget::showEvent(event);

   ui_line_edit->clearFocus();
   ui_info_label->setFocus();
}

/* ---------------------------------------------------------------------------*/
/* DatabaseAddDialog::on_buttonBox_rejected                                   */
/* ---------------------------------------------------------------------------*/
void DatabaseAddDialog::on_buttonBox_rejected()
{
    this->setResult(QDialog::Rejected);
    QDialog::reject();
    this->close();
}

/* ---------------------------------------------------------------------------*/
/* DatabaseAddDialog::on_buttonBox_accepted                                   */
/* ---------------------------------------------------------------------------*/
void DatabaseAddDialog::on_buttonBox_accepted()
{
    Debug::debug() << "DatabaseAddDialog::on_buttonBox_accepted";
    
    /* -- checks name -- */
    QString new_db_name = ui_line_edit->text();
    if( new_db_name.isEmpty() )
    {
        ui_info_label->setText( tr (" Collection name cannot be empty") );
        return;
    }
    
    foreach (const QString& name, Database::instance()->param_names()) 
    {
        if( name == new_db_name )
        {
          ui_info_label->setText( tr (" Collection name already exists ") );
          return;
        }
    }
    
    /* -- set database parameters -- */       
    Database::Param param;
    param._name                     = new_db_name;
    param._option_auto_rebuild      = false;
    param._option_check_cover       = true;
    param._option_download_cover    = true;
    param._option_group_albums      = false;
    param._option_artist_image      = true;
    param._option_wr_rating_to_file = false;
    
    if( QFileInfo( ui_add_folder->path() ).isDir() )
      param._paths.append( ui_add_folder->path() );

    if( isFirstStart )
      Database::instance()->param_clear();

    Database::instance()->param_add( param );
    Database::instance()->change_database( new_db_name );
    Database::instance()->settings_save();

    /* -- accept and close dialog -- */
    this->setResult(QDialog::Accepted);
    QDialog::accept();
    this->close();
}


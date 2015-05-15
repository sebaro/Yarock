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

#include "first_time_dialog.h"
#include "addfolder_widget.h"
#include "database.h"

#include <QDesktopServices>
#include <QDir>
#include <QDialogButtonBox>
#include <QVBoxLayout>

/*
********************************************************************************
*                                                                              *
*    Class FirstTimeDialog                                                     *
*                                                                              *
********************************************************************************
*/
FirstTimeDialog::FirstTimeDialog(QWidget *parent) : DialogBase(parent, tr("Setup your music collection directory"))
{
    // create Gui
    this->resize(445, 150);

    buttonBox()->setStandardButtons(QDialogButtonBox::Ok);
    
    
    const QString str = tr("<p>Welcome to yarock</p>"
                           "<p>Select the location of your music collection:</p>");

    QLabel* label = new QLabel();
    label->setText( str );
    
    ui_add_folder = new AddFolderWidget();
    ui_add_folder->hideRemoveButton();
    ui_add_folder->setMinimumWidth(350);

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget( label );
    layout->addWidget( ui_add_folder );

    setContentLayout(layout);
    
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
    
    
    /* -- connection -- */
    QObject::connect(buttonBox(), SIGNAL(accepted()), this, SLOT(on_buttonBox_accepted()));
}


/*******************************************************************************
    on_buttonBox_accepted
*******************************************************************************/
void FirstTimeDialog::on_buttonBox_accepted()
{
    //Debug::debug() << "FirstTimeDialog::on_buttonBox_accepted";
    Database *database = Database::instance();
    Database::Param param = Database::instance()->param();
    
    if( QFileInfo( ui_add_folder->path() ).isDir() )
      param._paths.append( ui_add_folder->path() );

    database->param_clear();
    database->param_add( param );
      
    /* accept and close dialog */
    this->setResult(QDialog::Accepted);
    QDialog::accept();
    this->close();
}


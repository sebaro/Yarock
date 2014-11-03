/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2014 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

#include "addfolder_widget.h"
#include "filedialog.h"

#include "debug.h"

#include <QApplication>
#include <QLayout>
/*
********************************************************************************
*                                                                              *
*    Class AddFolderWidget                                                     *
*                                                                              *
********************************************************************************
*/
AddFolderWidget::AddFolderWidget(QWidget *parent) : QWidget(parent)
{
    this->setMaximumHeight(30);
    this->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

    ui_file_path = new QLineEdit();
    
    ui_file_path->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
   
    ui_file_path->setStyleSheet(QString("QLineEdit:read-only { background: %1;}")
                     .arg(QApplication::palette().color( QPalette::Window ).name()));

    ui_file_path->setReadOnly( true );
    ui_file_path->setFocusPolicy( Qt::NoFocus );
    ui_file_path->setFrame( false );
    
    ui_open_button = new QPushButton();
    ui_open_button->setIcon(QIcon(":/images/folder-48x48.png"));
    ui_open_button->setToolTip(tr("Open"));
    
    ui_remove_button = new QPushButton();
    ui_remove_button->setIcon(QIcon::fromTheme("edit-delete"));
    ui_remove_button->setToolTip(tr("Remove"));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget( ui_file_path );
    layout->addWidget( ui_open_button );
    layout->addWidget( ui_remove_button );
    
    connect(ui_remove_button, SIGNAL(clicked ()), this, SIGNAL(removedClicked()));
    connect(ui_open_button, SIGNAL(clicked()), this, SLOT(slot_open_folder()));
}

void AddFolderWidget::slot_open_folder()
{
    FileDialog fd(0, FileDialog::AddDir, tr("Open folder"));
  
    if(fd.exec() == QDialog::Accepted)
    {
      ui_file_path->setText( fd.addDirectory() );
      ui_file_path->home(false);
      emit pathChanged();
    }
}

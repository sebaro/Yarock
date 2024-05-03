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

#include "database_operation.h"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>

#include <QButtonGroup>
#include <QPushButton>
/*
********************************************************************************
*                                                                              *
*    Class DatabaseOperationDialog                                             *
*                                                                              *
********************************************************************************
*/
DatabaseOperationDialog::DatabaseOperationDialog(QWidget *parent) : DialogBase(parent, tr("Database operation"))
{
    // create Gui
    this->resize(445, 150);

    buttonBox()->setStandardButtons(QDialogButtonBox::Cancel);

    QButtonGroup* group = new QButtonGroup();
    group->addButton( new QPushButton(QIcon(":/images/rebuild.png"), tr("Update")) , (int)OPE_RESCAN);
    group->addButton( new QPushButton(QIcon(":/images/rebuild.png"), tr("Rebuild")) , (int)OPE_REBUILD);
    group->addButton( new QPushButton(QIcon(":/images/download-48x48.png"), tr("Cover")) , (int)OPE_COVER);
    group->addButton( new QPushButton(QIcon(":/images/download-48x48.png"), tr("Artist")) , (int)OPE_ARTIST_IMAGE);
//     group->addButton( new QPushButton(QIcon(":/images/download-48x48.png"), tr("Genre")) , (int)OPE_GENRE_TAG);

    foreach(QAbstractButton* button, group->buttons() )
    {
       button->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
       static_cast<QPushButton*>(button)->setFlat(false);
    }

    QGridLayout *ui_grid_layout = new QGridLayout();
    ui_grid_layout->setContentsMargins(0, 0, 0, 0);

    ui_grid_layout->addWidget(group->button( (int)OPE_RESCAN), 0, 0, 1, 1);
    ui_grid_layout->addWidget(new QLabel(tr("Rescan medias files and update database")), 0, 1, 1, 1);
    ui_grid_layout->addWidget(group->button( (int)OPE_REBUILD), 1, 0, 1, 1);
    ui_grid_layout->addWidget(new QLabel(tr("Delete and rebuild database (*)")), 1, 1, 1, 1);
    ui_grid_layout->addWidget(new QLabel(tr("* all changes into collection database will be discarded !!")), 2, 0, 1, 2);
    ui_grid_layout->addWidget(group->button( (int)OPE_COVER), 3, 0, 1, 1);
    ui_grid_layout->addWidget(new QLabel(tr("Download missing album cover")), 3, 1, 1, 1);

    ui_grid_layout->addWidget(group->button( (int)OPE_ARTIST_IMAGE), 4, 0, 1, 1);
    ui_grid_layout->addWidget(new QLabel(tr("Download missing artist image")), 4, 1, 1, 1);

//     ui_grid_layout->addWidget(group->button( (int)OPE_GENRE_TAG), 5, 0, 1, 1);
//     ui_grid_layout->addWidget(new QLabel(tr("Download genre tag")), 5, 1, 1, 1);


    this->setContentLayout(ui_grid_layout);

    QObject::connect(buttonBox(), SIGNAL(rejected()), this, SLOT(on_buttonBox_rejected()));
    QObject::connect(group, SIGNAL(idClicked ( int )), this, SLOT(on_button_clicked( int )));

    /* initialize */
    m_operation = OPE_NONE;
}


/* ---------------------------------------------------------------------------*/
/* DatabaseOperationDialog::on_buttonBox_rejected                             */
/* ---------------------------------------------------------------------------*/
void DatabaseOperationDialog::on_buttonBox_rejected()
{
    this->setResult(QDialog::Rejected);
    QDialog::reject();
    this->close();
}


/* ---------------------------------------------------------------------------*/
/* DatabaseOperationDialog::on_button_clicked                                 */
/* ---------------------------------------------------------------------------*/
void DatabaseOperationDialog::on_button_clicked( int id)
{
    m_operation = (E_OPERATION)id;

    this->setResult(QDialog::Accepted);
    QDialog::accept();
    this->close();
}

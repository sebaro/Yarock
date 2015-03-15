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

#include "addstreamdialog.h"
#include "debug.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>


/*
********************************************************************************
*                                                                              *
*    Class AddStreamDialog                                                     *
*                                                                              *
********************************************************************************
*/
AddStreamDialog::AddStreamDialog(QWidget *parent,bool active_category) : DialogBase(parent, tr("Add Stream"))
{
    //create ui
    this->setFixedSize(480,160);

    ui_edit_url   = new QLineEdit();
    ui_edit_url->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui_edit_url->setText("http://");

    ui_edit_name  = new QLineEdit();
    ui_edit_name->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QGridLayout *ui_grid_layout = new QGridLayout();
    ui_grid_layout->setContentsMargins(0, 0, 0, 0);

    ui_grid_layout->addWidget(new QLabel(tr("url")), 0, 0, 1, 1);
    ui_grid_layout->addWidget(ui_edit_url, 0, 1, 1, 1);
    ui_grid_layout->addWidget(new QLabel(tr("name")), 1, 0, 1, 1);
    ui_grid_layout->addWidget(ui_edit_name, 1, 1, 1, 1);

    ui_edit_category = 0;
    
    if(active_category) 
    {
      ui_edit_category  = new QLineEdit();
      ui_grid_layout->addWidget(new QLabel(tr("category")), 2, 0, 1, 1);
      ui_grid_layout->addWidget(ui_edit_category, 2, 1, 1, 1);
    }

     /* layout content */
     setContentLayout(ui_grid_layout);

     QObject::connect(buttonBox(), SIGNAL(accepted()), this, SLOT(on_buttonBox_accepted()));
     QObject::connect(buttonBox(), SIGNAL(rejected()), this, SLOT(on_buttonBox_rejected()));
}

/*******************************************************************************
    setter/getter
*******************************************************************************/
const QString AddStreamDialog::url() const
{
    return ui_edit_url->text();
}


const QString AddStreamDialog::name() const
{
    return ui_edit_name->text();
}

const QString AddStreamDialog::category() const
{
    if(ui_edit_category)
      return ui_edit_category->text();
    else
      return QString();
}

void AddStreamDialog::setUrl(const QString& url)
{
    ui_edit_url->setText(url);
}


void AddStreamDialog::setName(const QString& name)
{
    return ui_edit_name->setText(name);
}

void AddStreamDialog::setCategory(const QString& categorie)
{
    if(ui_edit_category)
      ui_edit_category->setText(categorie);
}
/*******************************************************************************
    on_buttonBox_rejected
*******************************************************************************/
void AddStreamDialog::on_buttonBox_rejected()
{
    this->setResult(QDialog::Rejected);
    QDialog::reject();
    this->close();
}
/*******************************************************************************
    on_buttonBox_accepted
*******************************************************************************/
void AddStreamDialog::on_buttonBox_accepted()
{
    //Debug::debug() << "AddStreamDialog::on_buttonBox_accepted";

    this->setResult(QDialog::Accepted);
    QDialog::accept();
    this->close();
}

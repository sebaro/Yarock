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

#include "addstreamdialog.h"
#include "debug.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>

#include "widgets/editors/editor_stream.h"

/*
********************************************************************************
*                                                                              *
*    Class AddStreamDialog                                                     *
*                                                                              *
********************************************************************************
*/
AddStreamDialog::AddStreamDialog(MEDIA::TrackPtr stream,QWidget *parent) : DialogBase(parent, tr("Add Stream"))
{
    // init 
    m_stream = stream;
    
    //create ui
    this->setMinimumWidth(480);

    ui_edit_name       = new EdLineEdit();
    ui_edit_url        = new EdLineEdit();
    ui_edit_genre      = new EdLineEdit();
    ui_edit_website    = new EdLineEdit();
    ui_edit_bitrate    = new EdLineEdit();
    ui_edit_samplerate = new EdLineEdit();
    ui_edit_format     = new EdLineEdit();

    ui_edit_url->setText("http://");
          
 
    QGridLayout *ui_grid_layout = new QGridLayout();
    ui_grid_layout->setColumnStretch(0, 0);
    ui_grid_layout->setColumnStretch(1, 2);
    ui_grid_layout->setContentsMargins(0, 0, 0, 0);
    ui_grid_layout->addWidget(new QLabel(tr("name")), 0, 0, 1, 1);
    ui_grid_layout->addWidget(ui_edit_name, 0, 1, 1, 1);
    ui_grid_layout->addWidget(new QLabel(tr("url")), 1, 0, 1, 1);
    ui_grid_layout->addWidget(ui_edit_url, 1, 1, 1, 1);
    ui_grid_layout->addWidget(new QLabel(tr("genre")), 2, 0, 1, 1);
    ui_grid_layout->addWidget(ui_edit_genre, 2, 1, 1, 1);
    ui_grid_layout->addWidget(new QLabel(tr("website")), 3, 0, 1, 1);
    ui_grid_layout->addWidget(ui_edit_website, 3, 1, 1, 1);
    ui_grid_layout->addWidget(new QLabel(tr("bitrate")), 4, 0, 1, 1);
    ui_grid_layout->addWidget(ui_edit_bitrate, 4, 1, 1, 1);
    ui_grid_layout->addWidget(new QLabel(tr("samplerate")), 5, 0, 1, 1);
    ui_grid_layout->addWidget(ui_edit_samplerate, 5, 1, 1, 1);
    ui_grid_layout->addWidget(new QLabel(tr("format")), 6, 0, 1, 1);
    ui_grid_layout->addWidget(ui_edit_format, 6, 1, 1, 1);

    /* layout content */
    setContentLayout(ui_grid_layout);

    QObject::connect(buttonBox(), SIGNAL(accepted()), this, SLOT(on_buttonBox_accepted()));
    QObject::connect(buttonBox(), SIGNAL(rejected()), this, SLOT(on_buttonBox_rejected()));
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
              
    m_stream->id                   = -1;
    m_stream->url                  = ui_edit_url->text();
    m_stream->extra["station"]     = ui_edit_name->text();
    m_stream->genre                = ui_edit_genre->text();
    m_stream->extra["website"]     = ui_edit_website->text();
    m_stream->extra["bitrate"]     = ui_edit_bitrate->text();
    m_stream->extra["samplerate"]  = ui_edit_samplerate->text();
    m_stream->extra["format"]      = ui_edit_format->text();

    if(ui_edit_genre->text().isEmpty())
        m_stream->genre            = "Unknown";
    
    m_stream->isFavorite       = false;
    m_stream->isPlaying        = false;
    m_stream->isBroken         = false;
    m_stream->isPlayed         = false;
    m_stream->isStopAfter      = false;


    this->setResult(QDialog::Accepted);
    QDialog::accept();
    this->close();
}

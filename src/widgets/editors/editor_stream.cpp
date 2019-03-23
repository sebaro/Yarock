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

#include "editor_stream.h"
#include "playlistwidget.h"


#include "models/stream/service_favorite.h"

#include "covers/covercache.h"
#include "filedialog.h"
#include "utilities.h"
#include "global_actions.h"
#include "settings.h"
#include "debug.h"

#include <QtCore>
#include <QSqlQuery>
#include <QLayout>
#include <QScrollArea>
#include <QGraphicsScene>


/*
********************************************************************************
*                                                                              *
*    Class EditorStream                                                        *
*                                                                              *
********************************************************************************
*/
EditorStream::~EditorStream()
{
    Debug::debug() << "EditorStream delete";
}


EditorStream::EditorStream(MEDIA::TrackPtr stream, FavoriteStreams* fv,QGraphicsScene* scene) : QWidget()
{
    Debug::debug() << "EditorStream";
    m_new_image      = QImage(); 
    m_isChanged      = false;
    m_isImageChange  = false;
    m_stream         = MEDIA::TrackPtr( stream );
    m_fvs_model      = fv;
    m_scene          = scene;

    /* ------------ create gui ------------ */
    create_ui();
    
    /* ------------ initialization -------- */
    ui_edit_name->setText( stream->extra["station"].toString() );
    ui_edit_url->setText( stream->url );
    ui_edit_genre->setText( stream->genre );
    ui_edit_website->setText( stream->extra["website"].toString() );
    ui_edit_bitrate->setText( stream->extra["bitrate"].toString() );
    ui_edit_samplerate->setText( stream->extra["samplerate"].toString() );
    ui_edit_format->setText( stream->extra["format"].toString() );
    
    ui_image->setPixmap( CoverCache::instance()->cover(stream) );
    
    /* ------------ connections ------------ */
    connect(ui_buttonBox, SIGNAL(clicked ( QAbstractButton *)), this, SLOT(slot_on_buttonbox_clicked(QAbstractButton *)));
}


/* ---------------------------------------------------------------------------*/
/* EditorStream::slot_on_buttonbox_clicked                                    */
/* ---------------------------------------------------------------------------*/
void EditorStream::slot_on_buttonbox_clicked(QAbstractButton * button)
{
    Debug::debug() << "   [EditorStream] slot_on_buttonbox_clicked";
    
    QDialogButtonBox::ButtonRole role = ui_buttonBox->buttonRole(button);
    
    if(role == QDialogButtonBox::RejectRole) 
    {
      emit close();
    }
    else if (role == QDialogButtonBox::ApplyRole)
    {
      do_changes(); 

      if( m_isImageChange )
        save_new_image();
    }
}


/* ---------------------------------------------------------------------------*/
/* EditorStream::create_ui                                                    */
/* ---------------------------------------------------------------------------*/
void EditorStream::create_ui()
{
    ui_buttonBox = new QDialogButtonBox();
    ui_buttonBox->setOrientation(Qt::Horizontal);
    ui_buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Close);
    ui_buttonBox->setCenterButtons(true);
    ui_buttonBox->setContentsMargins(0,5,0,5);

    
    ui_headertitle = new QLabel();
    ui_headertitle->setFont(QFont("Arial",12,QFont::Bold));
    ui_headertitle->setStyleSheet(QString("QLabel { color : %1; }").arg(SETTINGS()->_baseColor.name()));
    ui_headertitle->setText(tr("Edit stream"));

    QVBoxLayout *vb1 = new QVBoxLayout();
    vb1->setSpacing(2);
    vb1->setContentsMargins(0, 6, 0, 6);
    vb1->addWidget( ui_headertitle );
    
    
    ui_image           = new QLabel();
    ui_edit_name       = new EdLineEdit();
    ui_edit_url        = new EdLineEdit();
    ui_edit_genre      = new EdLineEdit();
    ui_edit_website    = new EdLineEdit();
    ui_edit_bitrate    = new EdLineEdit();
    ui_edit_samplerate = new EdLineEdit();
    ui_edit_format     = new EdLineEdit();
    
    EdPushButton *cover_pb1 = new EdPushButton(QIcon(":/images/folder-48x48.png"), QString(), 0);
    EdPushButton *cover_pb3 = new EdPushButton(QIcon::fromTheme("edit-delete"), QString(), 0);    

    cover_pb1->setToolTip(tr("Load from file"));
    cover_pb3->setToolTip(tr("Remove"));

    connect(cover_pb1, SIGNAL(clicked()), this, SLOT(slot_load_image_from_file()));
    connect(cover_pb3, SIGNAL(clicked()), this, SLOT(slot_image_remove()));
    

    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing( 4 );
    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 2);
    grid->setColumnStretch(2, 0);
    
    grid->addWidget(new QLabel(tr("Station")), 0, 0, 1, 1);
    grid->addWidget(ui_edit_name, 0, 1, 1, 1);
    grid->addWidget(new QLabel(tr("Url")), 1, 0, 1, 1);
    grid->addWidget(ui_edit_url, 1, 1, 1, 1);
    grid->addWidget(new QLabel(tr("Genre")), 2, 0, 1, 1);
    grid->addWidget(ui_edit_genre, 2, 1, 1, 1);
    grid->addWidget(new QLabel(tr("Website")), 3, 0, 1, 1);
    grid->addWidget(ui_edit_website, 3, 1, 1, 1);
    grid->addWidget(new QLabel(tr("Bitrate")), 4, 0, 1, 1);
    grid->addWidget(ui_edit_bitrate, 4, 1, 1, 1);
    grid->addWidget(new QLabel(tr("Samplerate")), 5, 0, 1, 1);
    grid->addWidget(ui_edit_samplerate, 5, 1, 1, 1);
    grid->addWidget(new QLabel(tr("Format")), 6, 0, 1, 1);
    grid->addWidget(ui_edit_format, 6, 1, 1, 1);
    
    grid->addItem(new QSpacerItem(20, 20), 7,0,1,3);

    grid->addWidget(new QLabel(tr("Image")), 8, 0, 3, 1);
    grid->addWidget(ui_image, 8, 1, 3, 1, Qt::AlignHCenter);
    grid->addWidget(cover_pb1, 9, 2, 1, 1);
    grid->addWidget(cover_pb3, 10, 2, 1, 1);
   
    
    /* --- scrollarea ---*/
    QWidget *w  = new QWidget();
    w->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
     
    QVBoxLayout *vb3 = new QVBoxLayout( w );
    vb3->setSpacing(2);
    vb3->setContentsMargins(4, 2, 4, 4);
    vb3->addLayout( vb1 );
    vb3->addLayout( grid );
    vb3->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
  
    QScrollArea* ui_scrollarea = new QScrollArea();
    ui_scrollarea->setWidget(w);
    ui_scrollarea->setWidgetResizable(true);
    ui_scrollarea->setFrameShape(QFrame::NoFrame);
    ui_scrollarea->viewport()->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    
    /* --- layout ---*/
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget( ui_scrollarea );
    layout->addWidget(ui_buttonBox);
    
}

/* ---------------------------------------------------------------------------*/
/* EditorStream::do_changes                                                   */
/* ---------------------------------------------------------------------------*/
void EditorStream::do_changes()
{
    MEDIA::TrackPtr stream = m_stream;

    /* evaluate change */
    if( m_stream->url == ui_edit_url->text() &&
        m_stream->genre == ui_edit_genre->text() &&
        m_stream->extra["station"].toString() == ui_edit_name->text() &&
        m_stream->extra["website"].toString() == ui_edit_website->text() &&
        m_stream->extra["bitrate"].toString() == ui_edit_bitrate->text() &&
        m_stream->extra["samplerate"].toString() == ui_edit_samplerate->text() &&
        m_stream->extra["format"].toString() == ui_edit_format->text()
    )
    {
      Debug::debug() << "no change done";
      m_isChanged = false;
      return;
    }
       
    m_isChanged = true;


    /* apply new metada values */
    m_stream->url                 = ui_edit_url->text();
    m_stream->genre               = ui_edit_genre->text();
    m_stream->extra["station"]    = ui_edit_name->text();
    m_stream->extra["website"]    = ui_edit_website->text();
    m_stream->extra["bitrate"]    = ui_edit_bitrate->text();
    m_stream->extra["samplerate"] = ui_edit_samplerate->text();
    m_stream->extra["format"]     = ui_edit_format->text();
        
    /* save into model */
    m_fvs_model->updateStreamFavorite(stream);
    m_fvs_model->reload();
}


/* ---------------------------------------------------------------------------*/
/* EditorStream::slot_load_image_from_file                                    */
/* ---------------------------------------------------------------------------*/
void EditorStream::slot_load_image_from_file()
{
    //Debug::debug() << "   [EditorStream] slot_load_image_from_file";
    FileDialog fd(0, FileDialog::AddFile, tr("Select image file..."));
    QStringList filters = QStringList() << tr("Files (*.png *.jpg)");
    fd.setFilters(filters);

    if(fd.exec() != QDialog::Accepted || fd.addFile().isEmpty()) 
      return;

    /* get new cover file */
    m_new_image = QImage(fd.addFile());
 
    QByteArray ba;
    QBuffer buffer(&ba);

    buffer.open(QIODevice::WriteOnly);
    
    m_new_image.save(&buffer, "PNG");

    /*  save image according to configuration size */
    int SIZE = SETTINGS()->_coverSize;
    m_new_image = m_new_image.scaled(QSize(SIZE, SIZE), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    ui_image->setPixmap( QPixmap::fromImage(m_new_image).scaled( QSize(200,200), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
    ui_image->update();
    m_isImageChange = true;
}



/* ---------------------------------------------------------------------------*/
/* EditorStream::slot_load_image_from_file                                    */
/* ---------------------------------------------------------------------------*/
void EditorStream::slot_image_remove()
{
    //Debug::debug() << "   [EditorStream] slot_image_remove";
    ui_image->setPixmap( QPixmap(":/images/default-cover-200x200.png") );
    ui_image->update();
    
    m_new_image = QImage();
    m_isImageChange = true;
}


/* ---------------------------------------------------------------------------*/
/* EditorStream::slot_load_image_from_file                                    */
/* ---------------------------------------------------------------------------*/
void EditorStream::save_new_image()
{
    /* get image location */
    const QString path = UTIL::CONFIGDIR + "/radio/" + MEDIA::urlHash( m_stream->extra["station"].toString() );
    if( QFile(path).exists() ) 
    {
        QFile::remove(path);
    }
        
    if( ! m_new_image.isNull() ) 
    {
       /* save new image to location*/
       m_new_image.save(path, "png", -1);
    }
   
    /* update browser view */
    CoverCache::instance()->invalidate( m_stream );

    if(m_scene)
      m_scene->update();
}


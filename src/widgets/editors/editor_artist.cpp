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

#include "editor_artist.h"
#include "core/database/database.h"
#include "core/database/database_cmd.h"
#include "core/mediaitem/mediaitem.h"

#include "covers/covercache.h"
#include "filedialog.h"
#include "threadmanager.h"

#include "utilities.h"
#include "settings.h"
#include "debug.h"

#include "models/local/local_track_model.h"
#include "widgets/ratingwidget.h"
#include "widgets/editors/editor_common.h"

#include <QtCore>
#include <QSqlQuery>
#include <QLayout>
#include <QGraphicsScene>
#include <QScrollArea>


/*
********************************************************************************
*                                                                              *
*    Class EditorArtist                                                        *
*                                                                              *
********************************************************************************
*/
EditorArtist::~EditorArtist()
{
    Debug::debug() << "EditorArtist delete";
}


EditorArtist::EditorArtist(MEDIA::ArtistPtr artist, QGraphicsScene* scene) : QWidget()
{
    Debug::debug() << "EditorArtist";
    m_new_image      = QImage(); 
    m_isChanged      = false;
    m_isImageChange  = false;
    m_artist         = MEDIA::ArtistPtr( artist );
    m_scene          = scene;

    /* ------------ create gui ------------ */
    create_ui();
    
    /* ------------ initialization -------- */
    ui_artist_name->setText( artist->name );    
    ui_playcount->setValue( artist->playcount );
    ui_image->setPixmap( CoverCache::instance()->image(artist) );

    slot_update_rating_widget( artist->isUserRating );

    
    /* ------------ connections ------------ */
    connect(ui_buttonBox, SIGNAL(clicked ( QAbstractButton *)), this, SLOT(slot_on_buttonbox_clicked(QAbstractButton *)));
    connect(ui_clear_rating, SIGNAL(clicked()), this, SLOT(slot_update_rating_widget()));
    connect(ui_rating, SIGNAL(RatingChanged(float)), this, SLOT(slot_rating_changed(float)));
    
}


/* ---------------------------------------------------------------------------*/
/* EditorArtist::slot_on_buttonbox_clicked                                    */
/* ---------------------------------------------------------------------------*/
void EditorArtist::slot_on_buttonbox_clicked(QAbstractButton * button)
{
    QDialogButtonBox::ButtonRole role = ui_buttonBox->buttonRole(button);

    if (role == QDialogButtonBox::ApplyRole)
    {
      do_changes_artist(); 

      if( m_isImageChange )
        save_new_image();

      if( m_isChanged )
        ThreadManager::instance()->populateLocalTrackModel();
    }
    else
    {
      emit close();
    }
}


/* ---------------------------------------------------------------------------*/
/* EditorArtist::create_ui                                                    */
/* ---------------------------------------------------------------------------*/
void EditorArtist::create_ui()
{
    ui_buttonBox = new QDialogButtonBox();
    ui_buttonBox->setOrientation(Qt::Horizontal);
    ui_buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Close);

    
    ui_headertitle = new QLabel();
    ui_headertitle->setFont(QFont("Arial",12,QFont::Bold));
    ui_headertitle->setStyleSheet(QString("QLabel { color : %1; }").arg(SETTINGS()->_baseColor.name()));
    ui_headertitle->setText(tr("Edit artist"));

    QVBoxLayout *vb1 = new QVBoxLayout();
    vb1->setSpacing(2);
    vb1->setContentsMargins(0, 6, 0, 6);
    vb1->addWidget( ui_headertitle );
    
    
    ui_artist_name     = new EdLineEdit();
    ui_playcount       = new QSpinBox();
    ui_rating          = new RatingWidget();
    ui_clear_rating    = new EdPushButton(QIcon::fromTheme("edit-delete"), QString());
    ui_image           = new QLabel();
    
    EdPushButton *cover_pb1 = new EdPushButton(QIcon(":/images/folder-48x48.png"), QString(), 0);
    EdPushButton *cover_pb2 = new EdPushButton(QIcon(":/images/download-48x48.png"), QString(), 0);
    EdPushButton *cover_pb3 = new EdPushButton(QIcon::fromTheme("edit-delete"), QString(), 0);    

    cover_pb1->setToolTip(tr("Load from file"));
    cover_pb2->setToolTip(tr("Download"));
    cover_pb3->setToolTip(tr("Remove"));

    connect(cover_pb1, SIGNAL(clicked()), this, SLOT(slot_load_image_from_file()));
    connect(cover_pb2, SIGNAL(clicked()), this, SLOT(slot_download_image()));
    connect(cover_pb3, SIGNAL(clicked()), this, SLOT(slot_image_remove()));
    
    ui_artist_name->setMinimumHeight(25);
    ui_playcount->setMinimumHeight(25);
    ui_rating->setMinimumHeight(25);

    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing( 4 );
    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 2);
    grid->setColumnStretch(2, 0);
    
    grid->addWidget(new QLabel(tr("Artist")), 0, 0, 1, 1);
    grid->addWidget(ui_artist_name, 0, 1, 1, 2);
    grid->addWidget(new QLabel(tr("Playcount")), 1, 0, 1, 1);
    grid->addWidget(ui_playcount, 1, 1, 1, 2);
    grid->addWidget(new QLabel(tr("Rating")), 2, 0, 1, 1);
    grid->addWidget(ui_rating, 2, 1, 1, 1);
    grid->addWidget(ui_clear_rating, 2, 2, 1, 1);
    
    grid->addItem(new QSpacerItem(20, 20), 3,0,1,3);

    grid->addWidget(new QLabel(tr("Image")), 4, 0, 3, 1);
    grid->addWidget(ui_image, 4, 1, 3, 1, Qt::AlignHCenter);
    grid->addWidget(cover_pb1, 4, 2, 1, 1);
    grid->addWidget(cover_pb2, 5, 2, 1, 1);
    grid->addWidget(cover_pb3, 6, 2, 1, 1);
    
    
    /* --- scrollarea ---*/
    QWidget *w  = new QWidget();
    w->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
     
    QVBoxLayout *vb3 = new QVBoxLayout( w );
    vb3->setSpacing(2);
    vb3->setContentsMargins(4, 2, 4, 4);
    vb3->addLayout( vb1 );
    vb3->addLayout( grid );
    vb3->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
    vb3->addWidget(ui_buttonBox);
  
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
}


/* ---------------------------------------------------------------------------*/
/* EditorArtist::slot_rating_changed                                          */
/* ---------------------------------------------------------------------------*/
void EditorArtist::slot_rating_changed(float rating)
{
Q_UNUSED(rating)
    //Debug::debug() << " EditorArtist::slot_rating_changed";
    m_artist->isUserRating = true;

    if( ui_clear_rating )
    {
        ui_clear_rating->setToolTip(tr("clear user rating") );
        ui_clear_rating->setEnabled (true);
    }

    ui_rating->set_user_rating(true);
}

/* ---------------------------------------------------------------------------*/
/* EditorArtist::slot_update_rating_widget                                    */
/* ---------------------------------------------------------------------------*/
void EditorArtist::slot_update_rating_widget(bool isUser)
{
    //Debug::debug() << "slot_update_rating_widget USER : " << isUser;
    m_artist->isUserRating = isUser;

    float value = isUser ? m_artist->rating : LocalTrackModel::instance()->getItemAutoRating( m_artist );
    ui_rating->set_rating( value  );
    ui_rating->set_user_rating( isUser );

    QString toolTip =  isUser ? tr("clear user rating") : tr("auto rating");
    ui_clear_rating->setToolTip( toolTip );
    ui_clear_rating->setEnabled ( isUser );
}

/* ---------------------------------------------------------------------------*/
/* EditorArtist::do_changes_artist                                            */
/* ---------------------------------------------------------------------------*/
void EditorArtist::do_changes_artist()
{
    MEDIA::ArtistPtr artist = m_artist;

    /* evaluate change */
    if( artist->name        == ui_artist_name->text() &&
        artist->playcount   == ui_playcount->value()  &&
       (((artist->rating == ui_rating->rating()) && artist->isUserRating) || !artist->isUserRating)
    )
    {
      Debug::debug() << "no change done";
      m_isChanged = false;
      return;
    }
       
    m_isChanged = true;


    /* apply new metada values */
    QString  old_artist_name         = artist->name;
    artist->name                     = ui_artist_name->text();
    artist->playcount                = ui_playcount->value();
    artist->rating                   = artist->isUserRating ? ui_rating->rating() : -1.0;

    /* database update */
    if (!Database::instance()->open()) return;

    QSqlQuery("BEGIN TRANSACTION;",*Database::instance()->db());

    /* ------ artist ------ */
    int new_artist_id = DatabaseCmd::updateArtist(artist->name, artist->isFavorite, artist->playcount, artist->rating);

    /* ------ album ------ */
    foreach(MEDIA::MediaPtr media, artist->children())
    {
      MEDIA::AlbumPtr album   = MEDIA::AlbumPtr::staticCast(media);

      /* handle multiset album */
      QSet<int> disc_numbers;
      if(album->isMultiset()) 
      {
        foreach(MEDIA::MediaPtr media, album->children())
          disc_numbers.insert( MEDIA::TrackPtr::staticCast(media)->disc_number );
      }
      else
      {
        disc_numbers.insert( album->disc_number );
      }
      
      foreach (const int &disc_number, disc_numbers)
      {
        //Debug::debug() << "QSet value " << value;

        const QString  old_cover_name = MEDIA::coverName(old_artist_name,album->name);
        const QString  new_cover_name = MEDIA::coverName(artist->name,album->name);
        if(old_cover_name != new_cover_name)
          recoverCoverImage(new_cover_name,old_cover_name);

        int new_album_id = DatabaseCmd::updateAlbum(album->name, new_artist_id,album->year, disc_number, album->isFavorite, album->playcount, album->rating);

        /* ------ tracks ------ */
        foreach(MEDIA::MediaPtr media, album->children())
        {
          MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(media);

          /* skip track that not belong to disc number/album */
          if(album->isMultiset() && ( track->disc_number != disc_number ))
              continue;

          int track_id = track->id;

          QSqlQuery q("", *Database::instance()->db());
          q.prepare("UPDATE `tracks` SET `album_id`=?, `artist_id`=? WHERE `id`=?;");
          q.addBindValue(new_album_id);
          q.addBindValue(new_artist_id);
          q.addBindValue(track_id);
          q.exec();
        } // END foreach tracks
      } // END foreach albums set
    } // END foreach albums


    //! Database Clean
    QSqlQuery query("DELETE FROM artists WHERE id NOT IN (SELECT artist_id FROM tracks GROUP BY artist_id);", *Database::instance()->db());

    QSqlQuery("COMMIT TRANSACTION;",*Database::instance()->db());

}


/* ---------------------------------------------------------------------------*/
/* EditorArtist::slot_load_image_from_file                                    */
/* ---------------------------------------------------------------------------*/
void EditorArtist::slot_load_image_from_file()
{
    //Debug::debug() << "   [EditorArtist] slot_load_image_from_file";
    FileDialog fd(0, FileDialog::AddFile, tr("Select image file..."));
    QStringList filters = QStringList() << tr("Files (*.png *.jpg)");
    fd.setFilters(filters);

    if(fd.exec() != QDialog::Accepted || fd.addFile().isEmpty()) 
      return;

    /* get new cover file */
    m_new_image = QImage(fd.addFile());
 
    int ITEM_HEIGHT = 120;
    int ITEM_WIDTH  = 120;
    int MAX_SIZE    = 250;

    const int width = m_new_image.size().width();
    const int height = m_new_image.size().height();
    if (width > MAX_SIZE || height > MAX_SIZE)
      m_new_image = m_new_image.scaled(MAX_SIZE, MAX_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    int xOffset = 0;
    int wDiff = m_new_image.width() - ITEM_WIDTH;
    if (wDiff > 0) xOffset = wDiff / 2;
    int yOffset = 0;
    int hDiff = m_new_image.height() - ITEM_HEIGHT;
    if (hDiff > 0) yOffset = hDiff / 4;
    m_new_image = m_new_image.copy(xOffset, yOffset, ITEM_WIDTH, ITEM_HEIGHT);

    ui_image->setPixmap( QPixmap::fromImage(m_new_image) );
    ui_image->update();
    m_isImageChange = true;
}


/* ---------------------------------------------------------------------------*/
/* EditorArtist::slot_load_image_from_file                                    */
/* ---------------------------------------------------------------------------*/
void EditorArtist::slot_download_image()
{
    Debug::debug() << "   [EditorArtist] slot_download_image";
    INFO::InfoRequestData request;
    INFO::InfoStringHash hash;
    
    hash["artist"]     = m_artist->name;
    request = INFO::InfoRequestData(INFO::InfoArtistImages, hash);

    m_requests_ids << request.requestId;  
    
    connect( InfoSystem::instance(),
                SIGNAL( info( INFO::InfoRequestData, QVariant ) ),
                SLOT( slot_on_image_received( INFO::InfoRequestData, QVariant ) ), Qt::UniqueConnection );   
    
    /* start system info */
    InfoSystem::instance()->getInfo( request );
}


/* ---------------------------------------------------------------------------*/
/* EditorArtist::slot_load_image_from_file                                    */
/* ---------------------------------------------------------------------------*/
void EditorArtist::slot_on_image_received(INFO::InfoRequestData request, QVariant output)
{
    Debug::debug() << "   [EditorArtist] slot_on_image_received";
    /* check request */
    if(!m_requests_ids.contains(request.requestId))
      return;

    m_requests_ids.removeOne(request.requestId);
    
    /* get data */  
    m_new_image = QImage::fromData(output.toByteArray());

    int ITEM_HEIGHT = 120;
    int ITEM_WIDTH  = 120;
    int MAX_SIZE    = 250;

    const int width = m_new_image.size().width();
    const int height = m_new_image.size().height();
    if (width > MAX_SIZE || height > MAX_SIZE)
      m_new_image = m_new_image.scaled(MAX_SIZE, MAX_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    int xOffset = 0;
    int wDiff = m_new_image.width() - ITEM_WIDTH;
    if (wDiff > 0) xOffset = wDiff / 2;
    int yOffset = 0;
    int hDiff = m_new_image.height() - ITEM_HEIGHT;
    if (hDiff > 0) yOffset = hDiff / 4;
    m_new_image = m_new_image.copy(xOffset, yOffset, ITEM_WIDTH, ITEM_HEIGHT);

    ui_image->setPixmap( QPixmap::fromImage(m_new_image) );
    ui_image->update();
    m_isImageChange = true;
}

/* ---------------------------------------------------------------------------*/
/* EditorArtist::slot_load_image_from_file                                    */
/* ---------------------------------------------------------------------------*/
void EditorArtist::slot_image_remove()
{
    Debug::debug() << "   [EditorArtist] slot_image_remove";
    ui_image->setPixmap( QPixmap(":/images/default-cover-120x120.png") );
    ui_image->update();
    
    m_new_image = QImage();
    m_isImageChange = true;
}

/* ---------------------------------------------------------------------------*/
/* EditorArtist::slot_load_image_from_file                                    */
/* ---------------------------------------------------------------------------*/
void EditorArtist::save_new_image()
{
    /* get image location */
    QString path = m_artist->coverpath();
      
    /* removing existing image file */
    if(QFile::exists(path)) {
      QFile::remove(path);
    }

    if( ! m_new_image.isNull() ) 
    {
       /* save new image to location*/
       m_new_image.save(path, "png", -1);
    }
   
    /* update browser view */
    CoverCache::instance()->invalidate( m_artist );

    if(m_scene)
      m_scene->update();
}
 

/* ---------------------------------------------------------------------------*/
/* EditorArtist::recoverCoverImage                                            */
/* ---------------------------------------------------------------------------*/
/* copy old cover if exist into new cover art (according to new artist or album name) */
void EditorArtist::recoverCoverImage(const QString &newCoverName, const QString &oldCoverName)
{
    //Debug::debug() << " ----recoverCoverImage newCoverName + oldCoverName---- " << newCoverName + " - " + oldCoverName;
    QString storageLocation = UTIL::CONFIGDIR + "/albums/";

    //! quit if same filename
    if(newCoverName == oldCoverName) return;

    //! copy old into new
    QFile oldCoverFile(storageLocation + oldCoverName);
    if(oldCoverFile.exists()) 
    {
      //! remove new cover if already exist
      QFile newCoverFile(storageLocation + newCoverName);
      if(newCoverFile.exists()) 
        newCoverFile.remove();

      //Debug::debug() << " ----recoverCoverImage oldCoverName exists COPY ---- ";
      oldCoverFile.rename(storageLocation + newCoverName);
    }
}
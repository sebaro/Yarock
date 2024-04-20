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

#include "editor_album.h"
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
*    Class EditorAlbum                                                         *
*                                                                              *
********************************************************************************
*/
EditorAlbum::~EditorAlbum()
{
    Debug::debug() << "EditorAlbum delete";
}


EditorAlbum::EditorAlbum(MEDIA::AlbumPtr album, QGraphicsScene* scene) : QWidget()
{
    Debug::debug() << "EditorAlbum";
    m_new_image      = QImage(); 
    m_isChanged      = false;
    m_isImageChange  = false;
    m_album          = MEDIA::AlbumPtr( album );
    m_scene          = scene;

    /* ------------ create gui ------------ */
    create_ui();
    
    /* ------------ initialization -------- */    
    ui_album_name->setText( album->name );
    ui_artist_name->setText( MEDIA::ArtistPtr::staticCast(m_album->parent())->name );
    ui_album_year->setDate( QDate(album->year, 1, 1));
    ui_album_genre->setText( album->genres().join(",") );
    ui_playcount->setValue( album->playcount );
    ui_image->setPixmap( CoverCache::instance()->cover(album) );

    slot_update_rating_widget( m_album->isUserRating );

    
    /* ------------ connections ------------ */
    connect(ui_buttonBox, SIGNAL(clicked ( QAbstractButton *)), this, SLOT(slot_on_buttonbox_clicked(QAbstractButton *)));
    connect(ui_clear_rating, SIGNAL(clicked()), this, SLOT(slot_update_rating_widget()));
    connect(ui_rating, SIGNAL(RatingChanged(float)), this, SLOT(slot_rating_changed(float)));
}

/* ---------------------------------------------------------------------------*/
/* EditorAlbum::slot_on_buttonbox_clicked                                     */
/* ---------------------------------------------------------------------------*/
void EditorAlbum::slot_on_buttonbox_clicked(QAbstractButton * button)
{
    QDialogButtonBox::ButtonRole role = ui_buttonBox->buttonRole(button);

    if (role == QDialogButtonBox::ApplyRole)
    {
      do_changes_album(); 

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
/* EditorAlbum::create_ui                                                     */
/* ---------------------------------------------------------------------------*/
void EditorAlbum::create_ui()
{
    ui_buttonBox = new QDialogButtonBox();
    ui_buttonBox->setOrientation(Qt::Horizontal);
    ui_buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Close);
    ui_buttonBox->setCenterButtons(true);
    ui_buttonBox->setContentsMargins(0,5,0,5);
    
    ui_headertitle = new QLabel();
    ui_headertitle->setFont(QFont("Arial",12,QFont::Bold));
    ui_headertitle->setStyleSheet(QString("QLabel { color : %1; }").arg(SETTINGS()->_baseColor.name()));
    ui_headertitle->setText(tr("Edit album"));

    QVBoxLayout *vb1 = new QVBoxLayout();
    vb1->setSpacing(2);
    vb1->setContentsMargins(0, 6, 0, 6);
    vb1->addWidget( ui_headertitle );


    
    ui_album_name    = new EdLineEdit();
    ui_artist_name   = new EdLineEdit();
    ui_album_year    = new QDateEdit();
    ui_album_genre   = new EdLineEdit();
    ui_album_genre->setReadOnly( true );
    ui_album_genre->setEnabled( false );
    
    ui_playcount     = new QSpinBox();
    ui_rating        = new RatingWidget();
    ui_clear_rating  = new EdPushButton(QIcon::fromTheme("edit-delete"), QString());
    ui_image         = new QLabel();

    ui_album_year->setDisplayFormat("yyyy");

    EdPushButton *cover_pb1 = new EdPushButton(QIcon(":/images/folder-48x48.png"), QString(), 0);
    EdPushButton *cover_pb2 = new EdPushButton(QIcon(":/images/download-48x48.png"), QString(), 0);
    EdPushButton *cover_pb3 = new EdPushButton(QIcon::fromTheme("edit-delete"), QString(), 0);    

    cover_pb1->setToolTip(tr("Load from file"));
    cover_pb2->setToolTip(tr("Download"));
    cover_pb3->setToolTip(tr("Remove"));

    connect(cover_pb1, SIGNAL(clicked()), this, SLOT(slot_load_image_from_file()));
    connect(cover_pb2, SIGNAL(clicked()), this, SLOT(slot_download_image()));
    connect(cover_pb3, SIGNAL(clicked()), this, SLOT(slot_image_remove()));

    QGridLayout *gridl = new QGridLayout();

    gridl->setContentsMargins(0, 0, 0, 0);
    gridl->setHorizontalSpacing( 4 );
    gridl->setColumnStretch(0, 0);
    gridl->setColumnStretch(1, 2);
    gridl->setColumnStretch(2, 0);
    
    gridl->addWidget(new QLabel(tr("Album")), 0, 0, 1, 1);
    gridl->addWidget(ui_album_name, 0, 1, 1, 2);
    gridl->addWidget(new QLabel(tr("Artist")), 1, 0, 1, 1);
    gridl->addWidget(ui_artist_name, 1, 1, 1, 2);
    gridl->addWidget(new QLabel(tr("Year")), 2, 0, 1, 1);
    gridl->addWidget(ui_album_year, 2, 1, 1, 2);
    
    gridl->addWidget(new QLabel(tr("Genre")), 3, 0, 1, 1);
    gridl->addWidget(ui_album_genre, 3, 1, 1, 2);
    
    gridl->addWidget(new QLabel(tr("Playcount")), 4, 0, 1, 1);
    gridl->addWidget(ui_playcount, 4, 1, 1, 2);
    gridl->addWidget(new QLabel(tr("Rating")), 5, 0, 1, 1);
    gridl->addWidget(ui_rating, 5, 1, 1, 1);
    gridl->addWidget(ui_clear_rating, 5,2,1,1);

    gridl->addItem(new QSpacerItem(20, 20), 6,0,1,3);

    gridl->addWidget(new QLabel(tr("Cover")), 7, 0, 3, 1);
    gridl->addWidget(ui_image, 7, 1, 3, 1, Qt::AlignHCenter);
    gridl->addWidget(cover_pb1, 7, 2, 1, 1);
    gridl->addWidget(cover_pb2, 8, 2, 1, 1);
    gridl->addWidget(cover_pb3, 9, 2, 1, 1);
 
    gridl->addWidget(new QLabel(tr("Tracks")), 10, 0, 1, 1);
     
  

    QVBoxLayout* vb2 = new QVBoxLayout();
    
    foreach(MEDIA::MediaPtr media, m_album->children()) 
    {
      MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast( media );
      EdTrackEdit* track_editor = new EdTrackEdit( track );

      m_track_editors.insert( track->id, track_editor );
      vb2->addWidget( track_editor );
    }  
    
    
    /* --- scrollarea ---*/
    QWidget *w  = new QWidget();
    w->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
     
    QVBoxLayout *vb3 = new QVBoxLayout( w );
    vb3->setSpacing(2);
    vb3->setContentsMargins(4, 2, 4, 4);
    vb3->addLayout( vb1 );
    vb3->addLayout( gridl );
    vb3->addLayout( vb2 );
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
    layout->addWidget( ui_buttonBox );
}


/* ---------------------------------------------------------------------------*/
/* EditorAlbum::slot_rating_changed                                           */
/* ---------------------------------------------------------------------------*/
void EditorAlbum::slot_rating_changed(float rating)
{
Q_UNUSED(rating)
    //Debug::debug() << " EditorAlbum::slot_rating_changed";
    m_album->isUserRating = true;

    if( ui_clear_rating )
    {
        ui_clear_rating->setToolTip(tr("clear user rating") );
        ui_clear_rating->setEnabled (true);
    }

    ui_rating->set_user_rating(true);
}

/* ---------------------------------------------------------------------------*/
/* EditorAlbum::slot_update_rating_widget                                     */
/* ---------------------------------------------------------------------------*/
void EditorAlbum::slot_update_rating_widget(bool isUser)
{
    //Debug::debug() << "slot_update_rating_widget USER : " << isUser;
    m_album->isUserRating = isUser;

    float value = isUser ? m_album->rating : LocalTrackModel::instance()->getItemAutoRating( m_album );
    ui_rating->set_rating( value  );
    ui_rating->set_user_rating( isUser );

    QString toolTip =  isUser ? tr("clear user rating") : tr("auto rating");
    ui_clear_rating->setToolTip( toolTip );
    ui_clear_rating->setEnabled ( isUser );
}



/* ---------------------------------------------------------------------------*/
/* EditorAlbum::eval_change                                                   */
/* ---------------------------------------------------------------------------*/
bool EditorAlbum::eval_change()
{
    bool change =false;
    MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(m_album->parent());
    
    /* check album  */
    if( m_album->name            != ui_album_name->text()   ||
        artist->name           != ui_artist_name->text()  ||
        m_album->year            != ui_album_year->date().year() ||
        m_album->playcount       != ui_playcount->value()   ||
       ( (m_album->rating != ui_rating->rating() ) && m_album->isUserRating)
    )
    {
      change = true;
    }

    /* check tracks */
    foreach( EdTrackEdit* track_editor, m_track_editors.values() )
    {
      if( track_editor->isModified() )
        change = true;
    }
    
    return change;
}


/* ---------------------------------------------------------------------------*/
/* EditorAlbum::do_changes_album                                              */
/* ---------------------------------------------------------------------------*/
void EditorAlbum::do_changes_album()
{
    MEDIA::AlbumPtr  album  = m_album;
    MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(album->parent());

    m_isChanged = eval_change();
    
    /* evaluate change */
    if( !m_isChanged )
    {
      Debug::debug() << "no change done";
      return;
    }

    Debug::debug() << "change done --> apply";

    /* recover album cover */
    const QString  old_cover_name  = album->coverHash();
    const QString  new_cover_name  = MEDIA::coverHash(ui_artist_name->text(),ui_album_name->text());
    recoverCoverImage(new_cover_name,old_cover_name);
    
    /* apply new metada values */
    album->name            = ui_album_name->text();
    album->year            = ui_album_year->date().year();
    artist->name           = ui_artist_name->text();
    album->playcount       = ui_playcount->value();
    album->rating          = album->isUserRating ? ui_rating->rating() : -1.0;

    foreach ( MEDIA::MediaPtr media, m_album->children())
    {
      MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(media);
      track->album   = ui_album_name->text();
      track->artist  = ui_artist_name->text();
      track->year    = ui_album_year->date().year();
      
      track->num     = m_track_editors[track->id]->trackNum();
      track->title   = m_track_editors[track->id]->trackTitle();
      track->genre   = m_track_editors[track->id]->trackGenre();
    }


    /* database update */
    if (!Database::instance()->open()) return;
    QSqlQuery("BEGIN TRANSACTION;",*Database::instance()->db());

    /* ------ artist ------ */
    int new_artist_id = DatabaseCmd::updateArtist(artist->name, artist->isFavorite, artist->playcount, artist->rating);

    /* ------ album ------- */
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
        int new_album_id = DatabaseCmd::updateAlbum(album->name, new_artist_id,album->year, disc_number, album->isFavorite, album->playcount, album->rating);

        /* ------ tracks ------- */
        foreach ( MEDIA::MediaPtr media, album->children())
        {
            MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(media);
            int trackId = track->id;

            /* skip track that not belong to disc number/album */
            if(album->isMultiset() && ( track->disc_number != disc_number ))
              continue;

            /* ------ genre ------ */
            DatabaseCmd::insertGenre( track->genre );

            /* ------ year ------ */
            DatabaseCmd::insertYear( track->year );


            QSqlQuery q("",*Database::instance()->db());
            q.prepare("UPDATE tracks SET `trackname`=?,`number`=?,`artist_id`=?,`album_id`=?,`year_id`=(SELECT `id` FROM `years` WHERE `year`=?),`genre_id`=(SELECT `id` FROM `genres` WHERE `genre`=?), `rating`=? WHERE `id`=?;");
            q.addBindValue( track->title );
            q.addBindValue( track->num );
            q.addBindValue( new_artist_id );
            q.addBindValue( new_album_id );
            q.addBindValue( track->year );
            q.addBindValue( track->genre );
            q.addBindValue( track->rating );
            q.addBindValue( trackId );
            q.exec();
        }
    } // end foreach multiset album loop


    /* clean database */
    DatabaseCmd::clean();

    QSqlQuery("COMMIT TRANSACTION;",*Database::instance()->db());
}

/* ---------------------------------------------------------------------------*/
/* EditorAlbum::slot_load_image_from_file                                     */
/* ---------------------------------------------------------------------------*/
void EditorAlbum::slot_load_image_from_file()
{
    //Debug::debug() << "   [EditorAlbum] slot_load_image_from_file";
    FileDialog fd(0, FileDialog::AddFile, tr("Select image file..."));
    QStringList filters = QStringList() << tr("Files (*.png *.jpg)");
    fd.setFilters(filters);

    if(fd.exec() != QDialog::Accepted || fd.addFile().isEmpty()) 
      return;

    /* get new cover file */
    m_new_image = QImage(fd.addFile());
 
    /*  save image according to configuration size */
    int SIZE = SETTINGS()->_coverSize;
    m_new_image = m_new_image.scaled(QSize(SIZE, SIZE), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui_image->setPixmap( QPixmap::fromImage(m_new_image).scaled( QSize(200,200), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
    ui_image->update();
    m_isImageChange = true;
}

/* ---------------------------------------------------------------------------*/
/* EditorAlbum::slot_download_image                                           */
/* ---------------------------------------------------------------------------*/
void EditorAlbum::slot_download_image()
{
    Debug::debug() << "   [EditorAlbum] slot_download_image";
    INFO::InfoRequestData request;
    INFO::InfoStringHash hash;
    
    hash["artist"]     = MEDIA::ArtistPtr::staticCast( m_album->parent() )->name;
    hash["album"]      = m_album->name;
    hash["covername"]  = MEDIA::coverHash( hash["artist"], hash["album"] );

    request = INFO::InfoRequestData(INFO::InfoAlbumCoverArt, hash);

    m_requests_ids << request.requestId;  

    connect( InfoSystem::instance(),
                SIGNAL( info( INFO::InfoRequestData, QVariant ) ),
                SLOT( slot_on_image_received( INFO::InfoRequestData, QVariant ) ), Qt::UniqueConnection );   
    
    /* start system info */
    InfoSystem::instance()->getInfo( request );
}

/* ---------------------------------------------------------------------------*/
/* EditorAlbum::slot_on_image_received                                        */
/* ---------------------------------------------------------------------------*/
void EditorAlbum::slot_on_image_received(INFO::InfoRequestData request, QVariant output)
{
    Debug::debug() << "   [EditorAlbum] slot_on_image_received";
    
    /* check request */
    if(!m_requests_ids.contains(request.requestId))
      return;

    m_requests_ids.removeOne(request.requestId);
    
    /* get data */
    m_new_image = QImage::fromData(output.toByteArray());

    m_new_image = m_new_image.scaled(QSize(200, 200), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui_image->setPixmap( QPixmap::fromImage(m_new_image) );
    ui_image->update();
    m_isImageChange = true;
}


/* ---------------------------------------------------------------------------*/
/* EditorAlbum::slot_image_remove                                             */
/* ---------------------------------------------------------------------------*/
void EditorAlbum::slot_image_remove()
{
    Debug::debug() << "   [EditorAlbum] slot_image_remove";
    ui_image->setPixmap( QPixmap(":/images/default-cover-200x200.png") );
    ui_image->update();
    
    m_new_image = QImage();
    m_isImageChange = true;
}

/* ---------------------------------------------------------------------------*/
/* EditorAlbum::save_new_image                                                */
/* ---------------------------------------------------------------------------*/
void EditorAlbum::save_new_image()
{
    /* get image location */
    const QString path = UTIL::CONFIGDIR + "/albums/" + m_album->coverHash();
      
    /* removing existing image file */
    if(QFile::exists(path)) {
      QFile::remove(path);
    }

    if( !m_new_image.isNull() ) 
    {
       /* save new image to location*/
       m_new_image.save(path, "png", -1);
    }
   
    /* update browser view */
    CoverCache::instance()->invalidate( m_album );

    if(m_scene)
      m_scene->update();
}
 
/* ---------------------------------------------------------------------------*/
/* EditorAlbum::recoverCoverImage                                             */
/* ---------------------------------------------------------------------------*/
/* copy old cover if exist into new cover art (according to new artist or album name) */
void EditorAlbum::recoverCoverImage(const QString &newCoverName, const QString &oldCoverName)
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


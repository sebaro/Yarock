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

#include "editor_track.h"
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
*    Class EditorTrack                                                         *
*                                                                              *
********************************************************************************
*/
EditorTrack::~EditorTrack()
{
    Debug::debug() << "EditorTrack delete";
}


EditorTrack::EditorTrack(MEDIA::TrackPtr track, QGraphicsScene* scene) : QWidget()
{
    Debug::debug() << "EditorTrack";
    m_isChanged      = false;
    m_scene          = scene;
    m_tracks         << track;

    create_ui();
    init(); 
    
    /* ------------ connections ------------ */
    connect(ui_buttonBox, SIGNAL(clicked ( QAbstractButton *)), this, SLOT(slot_on_buttonbox_clicked(QAbstractButton *)));
}

EditorTrack::EditorTrack(QList<MEDIA::TrackPtr> tracks, QGraphicsScene* scene) : QWidget()
{
    Debug::debug() << "EditorTrack";
    m_isChanged      = false;
    m_scene          = scene;
    m_tracks         << tracks;

    create_ui();
    init(); 
    
    /* ------------ connections ------------ */
    connect(ui_buttonBox, SIGNAL(clicked ( QAbstractButton *)), this, SLOT(slot_on_buttonbox_clicked(QAbstractButton *)));
}

void EditorTrack::init()
{
    /* ------------ initialization -------- */
    QVariantList lti, lal, lar, lge, lye, lnu, lpl, lra; 
    
    foreach(MEDIA::TrackPtr track, m_tracks) {
      lti << QVariant( track->title );
      lal << QVariant( track->album );
      lar << QVariant( track->artist );
      lye << QVariant( track->year );
      lge << QVariant( track->genre );
      lnu << QVariant( QString::number(track->num) );
      lra << QVariant( track->rating );
      lpl << QVariant( track->playcount );
    } 
  
    ui_multifields["title"]->setValues( lti );
    ui_multifields["album"]->setValues( lal );
    ui_multifields["artist"]->setValues( lar );
    ui_multifields["year"]->setValues( lye );
    ui_multifields["genre"]->setValues( lge );
    ui_multifields["number"]->setValues( lnu );
    ui_multifields["rating"]->setValues( lra );
    ui_multifields["playcount"]->setValues( lpl );  
  
    if( m_tracks.size() == 1 ) {
      ui_filename->setText( m_tracks.first()->url );
      ui_filename->home(false);
      
      foreach(EdMultiFieldEdit* w, ui_multifields.values())
        w->checkWidget()->setVisible(false);
    }
}

/* ---------------------------------------------------------------------------*/
/* EditorTrack::slot_on_buttonbox_clicked                                     */
/* ---------------------------------------------------------------------------*/
void EditorTrack::slot_on_buttonbox_clicked(QAbstractButton * button)
{
    QDialogButtonBox::ButtonRole role = ui_buttonBox->buttonRole(button);

    if (role == QDialogButtonBox::ApplyRole)
    {
      do_changes_track(); 

      if( m_isChanged )
        ThreadManager::instance()->populateLocalTrackModel();
    }
    else
    {
      emit close();
    }
}

/* ---------------------------------------------------------------------------*/
/* EditorTrack::create_ui                                                     */
/* ---------------------------------------------------------------------------*/
void EditorTrack::create_ui()
{
    ui_buttonBox = new QDialogButtonBox();
    ui_buttonBox->setOrientation(Qt::Horizontal);
    ui_buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Close);

    ui_headertitle = new QLabel();
    ui_headertitle->setFont(QFont("Arial",12,QFont::Bold));
    ui_headertitle->setStyleSheet(QString("QLabel { color : %1; }").arg(SETTINGS()->_baseColor.name()));
    ui_headertitle->setText(tr("Edit track"));

    QVBoxLayout *vb1 = new QVBoxLayout();
    vb1->setSpacing(2);
    vb1->setContentsMargins(0, 6, 0, 6);
    vb1->addWidget( ui_headertitle );
    
    
    ui_filename     = new EdLineEdit();
    ui_track_title  = new EdLineEdit();
    ui_album_name   = new EdLineEdit();
    ui_artist_name  = new EdLineEdit();
    ui_track_year   = new QDateEdit();
    ui_track_genre  = new EdLineEdit();
    ui_track_number = new EdLineEdit();
    ui_playcount    = new QSpinBox();
    ui_rating       = new RatingWidget();
    ui_rating->set_user_rating( true );

    ui_filename->setStyleSheet(QString::fromUtf8("QLineEdit {background: transparent;}"));
    ui_filename->setFrame(false);
    ui_filename->setReadOnly(true);

    ui_track_year->setDisplayFormat("yyyy");

    ui_track_title->setMinimumHeight(25);
    ui_album_name->setMinimumHeight(25);
    ui_artist_name->setMinimumHeight(25);
    ui_track_year->setMinimumHeight(25);
    ui_track_genre->setMinimumHeight(25);
    ui_track_number->setMinimumHeight(25);
    ui_playcount->setMinimumHeight(25);
    ui_rating->setMinimumHeight(25);

    
    ui_multifields.insert("title",    new EdMultiFieldEdit(EdMultiFieldEdit::LINEEDIT, ui_track_title) );
    ui_multifields.insert("album",    new EdMultiFieldEdit(EdMultiFieldEdit::LINEEDIT, ui_album_name) );
    ui_multifields.insert("artist",   new EdMultiFieldEdit(EdMultiFieldEdit::LINEEDIT, ui_artist_name) );
    ui_multifields.insert("year",     new EdMultiFieldEdit(EdMultiFieldEdit::DATEEDIT, ui_track_year) );
    ui_multifields.insert("genre",    new EdMultiFieldEdit(EdMultiFieldEdit::LINEEDIT, ui_track_genre) );
    ui_multifields.insert("playcount",new EdMultiFieldEdit(EdMultiFieldEdit::SPINBOX,  ui_playcount) );
    ui_multifields.insert("rating",   new EdMultiFieldEdit(EdMultiFieldEdit::RATING,   ui_rating) );
    ui_multifields.insert("number",   new EdMultiFieldEdit(EdMultiFieldEdit::LINEEDIT, ui_track_number) );

    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing( 0 );
    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 0);
    grid->setColumnStretch(2, 2);
    grid->setColumnStretch(3, 0);

    grid->addWidget(new QLabel(tr("File")), 0, 1, 1, 1);
    grid->addWidget(ui_filename, 0, 2, 1, 1);
    
    grid->addWidget(ui_multifields.value("title")->checkWidget(), 1, 0, 1, 1);
    grid->addWidget(new QLabel(tr("Title")), 1, 1, 1, 1);
    grid->addWidget(ui_track_title, 1, 2, 1, 1);
    
    grid->addWidget(ui_multifields.value("album")->checkWidget(), 2, 0, 1, 1);
    grid->addWidget(new QLabel(tr("Album")), 2, 1, 1, 1);
    grid->addWidget(ui_album_name, 2, 2, 1, 1);

    grid->addWidget(ui_multifields.value("artist")->checkWidget(), 3, 0, 1, 1);
    grid->addWidget(new QLabel(tr("Artist")), 3, 1, 1, 1);
    grid->addWidget(ui_artist_name, 3, 2, 1, 1);
    
    grid->addWidget(ui_multifields.value("year")->checkWidget(), 4, 0, 1, 1);
    grid->addWidget(new QLabel(tr("Year")), 4, 1, 1, 1);
    grid->addWidget(ui_track_year, 4, 2, 1, 1);
    
    grid->addWidget(ui_multifields.value("genre")->checkWidget(), 5, 0, 1, 1);
    grid->addWidget(new QLabel(tr("Genre")), 5, 1, 1, 1);
    grid->addWidget(ui_track_genre, 5, 2, 1, 1);
    
    grid->addWidget(ui_multifields.value("number")->checkWidget(), 6, 0, 1, 1);
    grid->addWidget(new QLabel(tr("Number")), 6, 1, 1, 1);
    grid->addWidget(ui_track_number, 6, 2, 1, 1);

    grid->addWidget(ui_multifields.value("playcount")->checkWidget(), 7, 0, 1, 1);
    grid->addWidget(new QLabel(tr("Playcount")), 7, 1, 1, 1);
    grid->addWidget(ui_playcount, 7, 2, 1, 1);

    grid->addWidget(ui_multifields.value("rating")->checkWidget(), 8, 0, 1, 1);
    grid->addWidget(new QLabel(tr("Rating")), 8, 1, 1, 1);
    grid->addWidget(ui_rating, 8, 2, 1, 1);
  
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
/* EditorTrack::do_changes_track                                              */
/* ---------------------------------------------------------------------------*/
void EditorTrack::do_changes_track()
{
    /* evaluate change */
    if ( !ui_multifields["title"]->isValueChanged()  &&
         !ui_multifields["album"]->isValueChanged()  && 
         !ui_multifields["artist"]->isValueChanged() && 
         !ui_multifields["genre"]->isValueChanged()  && 
         !ui_multifields["year"]->isValueChanged()   && 
         !ui_multifields["playcount"]->isValueChanged()  && 
         !ui_multifields["rating"]->isValueChanged()  &&
         !ui_multifields["number"]->isValueChanged() 
       )
    {
      Debug::debug() << "no change done";
      m_isChanged = false;
      return;
    }

    Debug::debug() << "change done --> apply";
    m_isChanged = true;
    
    
    /* database update */
    if (!Database::instance()->open()) return;
    QSqlQuery("BEGIN TRANSACTION;",*Database::instance()->db());    

    /* loop over tracks */
    foreach(MEDIA::TrackPtr track, m_tracks) 
    {
      /* apply new metada values */
      if( ui_multifields["title"]->isValueChanged() )
          track->title      =  ui_track_title->text();
      if( ui_multifields["album"]->isValueChanged() )
          track->album      =  ui_album_name->text();
      if( ui_multifields["artist"]->isValueChanged() )
          track->artist     =  ui_artist_name->text();
      if( ui_multifields["genre"]->isValueChanged() )
          track->genre      =  ui_track_genre->text();
      if( ui_multifields["year"]->isValueChanged() )
          track->year       =  ui_track_year->date().year();
      if( ui_multifields["playcount"]->isValueChanged() )
          track->playcount  =  ui_playcount->value();
      if( ui_multifields["number"]->isValueChanged() )
          track->num        =  ui_track_number->text().toInt();
      if( ui_multifields["rating"]->isValueChanged() )
          track->rating     =  ui_rating->rating();

      MEDIA::AlbumPtr  album_media   = MEDIA::AlbumPtr::staticCast( track->parent());
      MEDIA::ArtistPtr artist_media  = MEDIA::ArtistPtr::staticCast(album_media->parent());
    
      /* ------ genre ------ */
      DatabaseCmd::insertGenre( track->genre );

      /* ------ year ------ */
      DatabaseCmd::insertYear( track->year );

      /* ------ artist ------ */
      int new_artist_id = DatabaseCmd::updateArtist(track->artist, artist_media->isFavorite, artist_media->playcount, artist_media->rating);

      /* ------ album ------- */
      int new_album_id = DatabaseCmd::updateAlbum(track->album, new_artist_id,track->year, track->disc_number, album_media->isFavorite, album_media->playcount, album_media->rating);

      /* ------ tracks ------ */
      int trackId = track->id;

      QSqlQuery q("",*Database::instance()->db());
      q.prepare("UPDATE tracks SET `trackname`=?,`artist_id`=?,`album_id`=?,`year_id`=(SELECT `id` FROM `years` WHERE `year`=?),`genre_id`=(SELECT `id` FROM `genres` WHERE `genre`=?), `rating`=? WHERE `id`=?;");
      q.addBindValue( track->title );
      q.addBindValue( new_artist_id );
      q.addBindValue( new_album_id );
      q.addBindValue( track->year );
      q.addBindValue( track->genre );
      q.addBindValue( track->rating );
      q.addBindValue( trackId );
      q.exec();
    } /* end loop over tracks */
    
    /* clean database */
    DatabaseCmd::clean();

    QSqlQuery("COMMIT TRANSACTION;",*Database::instance()->db());
}


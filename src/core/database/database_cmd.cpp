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

#include "database_cmd.h"
#include "database.h"
#include "core/mediaitem/tag.h"
#include "models/local/local_track_model.h"
#include "debug.h"

#include <QSqlQuery>

/*
********************************************************************************
*                                                                              *
*    Class DatabaseCmd                                                         *
*                                                                              *
********************************************************************************
*/
DatabaseCmd::DatabaseCmd()  {}

/* ---------------------------------------------------------------------------*/
/* DatabaseCmd::clean                                                         */
/* ---------------------------------------------------------------------------*/  
void DatabaseCmd::clean()
{
    QSqlQuery q("",*Database::instance()->db());
    q.prepare("DELETE FROM `albums` WHERE `id` NOT IN (SELECT `album_id` FROM `tracks` GROUP BY `album_id`);");
    q.exec();
    
    q.prepare("DELETE FROM `genres` WHERE `id` NOT IN (SELECT `genre_id` FROM `tracks` GROUP BY `genre_id`);");
    q.exec();

    q.prepare("DELETE FROM `artists` WHERE `id` NOT IN (SELECT `artist_id` FROM `tracks` GROUP BY `artist_id`);");
    q.exec();    

    q.prepare("DELETE FROM `years` WHERE `id` NOT IN (SELECT `year_id` FROM `tracks` GROUP BY `year_id`);");
    q.exec();  

    q.prepare("DELETE FROM `playlist_items` WHERE `playlist_id` NOT IN (SELECT `id` FROM `playlists`);");
    q.exec(); 
}

/* ---------------------------------------------------------------------------*/
/* DatabaseCmd::insertGenre                                                   */
/* ---------------------------------------------------------------------------*/  
int DatabaseCmd::insertGenre(const QString & genre)
{
    //Debug::debug() << "- DatabaseCmd -> insert genre";
  
    QSqlQuery q("",*Database::instance()->db());
    q.prepare("SELECT `id` FROM `genres` WHERE `genre`=?;");
    q.addBindValue( genre );
    q.exec();

    if ( !q.next() ) {
      q.prepare("INSERT INTO `genres`(`genre`) VALUES (?);");
      q.addBindValue( genre );
      q.exec();

      if(q.numRowsAffected() < 1) return -1;
      q.prepare("SELECT `id` FROM `genres` WHERE `genre`=?;");
      q.addBindValue( genre );
      q.exec();
      q.next();
    }
    return q.value(0).toInt();
}

/* ---------------------------------------------------------------------------*/
/* DatabaseCmd::insertYear                                                    */
/* ---------------------------------------------------------------------------*/  
int DatabaseCmd::insertYear(int year)
{
    //Debug::debug() << "- DatabaseCmd -> insert year";
  
    QSqlQuery q("",*Database::instance()->db());
    q.prepare("SELECT `id` FROM `years` WHERE `year`=?;");
    q.addBindValue( year );
    q.exec();

    if ( !q.next() ) {
      q.prepare("INSERT INTO `years`(`year`) VALUES (?);");
      q.addBindValue( year );
      q.exec();

      if(q.numRowsAffected() < 1) return -1;
      q.prepare("SELECT `id` FROM `years` WHERE `year`=?;");
      q.addBindValue( year );
      q.exec();
      q.next();
    }
    return q.value(0).toInt();
}

/* ---------------------------------------------------------------------------*/
/* DatabaseCmd::isArtistExists                                                */
/* ---------------------------------------------------------------------------*/
bool DatabaseCmd::isArtistExists(const QString & artist)
{
    //Debug::debug() << "- DatabaseCmd -> is artist exists";
    QSqlQuery q("",*Database::instance()->db());
    q.prepare("SELECT `id` FROM `artists` WHERE `name`=?;");
    q.addBindValue( artist );
    q.exec();

    if ( !q.next() )
      return false;
    else
      return  true;
} 

/* ---------------------------------------------------------------------------*/
/* DatabaseCmd::insertArtist                                                  */
/* ---------------------------------------------------------------------------*/
int DatabaseCmd::insertArtist(const QString & artist)
{
    //Debug::debug() << "- DatabaseCmd -> insert artist";
  
    QSqlQuery q("",*Database::instance()->db());
    q.prepare("SELECT `id` FROM `artists` WHERE `name`=?;");
    q.addBindValue( artist );
    q.exec();

    if ( !q.next() )
    {
        q.prepare("INSERT INTO `artists`(`name`,`favorite`,`playcount`,`rating`) VALUES (?,?,?,?);");
        q.addBindValue( artist );
        q.addBindValue( 0 );
        q.addBindValue( 0 );
        q.addBindValue( -1 );
        q.exec();

        if(q.numRowsAffected() < 1) return -1;
        q.prepare("SELECT `id` FROM `artists` WHERE `name`=?;");
        q.addBindValue( artist );
        q.exec();
        q.next();
    }

    return  q.value(0).toInt();
}  

/* ---------------------------------------------------------------------------*/
/* DatabaseCmd::updateArtist                                                  */
/* ---------------------------------------------------------------------------*/
int DatabaseCmd::updateArtist(const QString & artist, bool favorite, int playcount, float rating)
{
    //Debug::debug() << "- DatabaseCmd -> update artist";
  
    int id = DatabaseCmd::insertArtist(artist);
    
    QSqlQuery q("",*Database::instance()->db());
    q.prepare("UPDATE `artists` SET `name`=?,`favorite`=?,`playcount`=?,`rating`=? WHERE `id`=?;");
    q.addBindValue( artist );
    q.addBindValue( favorite );
    q.addBindValue( playcount );
    q.addBindValue( rating );    
    q.addBindValue( id );
    q.exec();
    
    return id;
}      

/* ---------------------------------------------------------------------------*/
/* DatabaseCmd::insertAlbum                                                   */
/* ---------------------------------------------------------------------------*/  
int DatabaseCmd::insertAlbum(const QString & album, int artist_id,int year,int disc)
{
    //Debug::debug() << "- DatabaseCmd -> insert album";
  
    QSqlQuery q("",*Database::instance()->db());
    q.prepare("SELECT `id` FROM `albums` WHERE `name`=? AND `artist_id`=? AND `disc`=?;");
    q.addBindValue( album );
    q.addBindValue( artist_id );
    q.addBindValue( disc );
    q.exec();

    if ( !q.next() ) {
      q.prepare("INSERT INTO `albums`(`name`,`artist_id`,`year`,`favorite`,`playcount`,`rating`,`disc`) VALUES (?,?,?,?,?,?,?);");
      q.addBindValue( album );
      q.addBindValue( artist_id );
      q.addBindValue( year );
      q.addBindValue( 0 );
      q.addBindValue( 0 );
      q.addBindValue( -1 );      
      q.addBindValue( disc );
      q.exec();

      if(q.numRowsAffected() < 1) return -1;
      q.prepare("SELECT `id` FROM `albums` WHERE `name`=? AND `artist_id`=? AND `disc`=?;");
      q.addBindValue( album );
      q.addBindValue( artist_id );
      q.addBindValue( disc );
      q.exec();
      q.next();
    }
    return q.value(0).toInt();
}

/* ---------------------------------------------------------------------------*/
/* DatabaseCmd::updateAlbum                                                   */
/* ---------------------------------------------------------------------------*/  
int DatabaseCmd::updateAlbum(const QString & album, int artist_id, int year, int disc, bool favorite, int playcount, float rating)
{
    //Debug::debug() << "- DatabaseCmd -> update album";
  
    int id = DatabaseCmd::insertAlbum(album, artist_id, year, disc);
    
    QSqlQuery q("",*Database::instance()->db());
    q.prepare("UPDATE `albums` SET `name`=?,`artist_id`=?,`year`=?,`favorite`=?,`playcount`=?,`rating`=? WHERE `id`=?;");
    q.addBindValue( album );
    q.addBindValue( artist_id );
    q.addBindValue( year );
    q.addBindValue( favorite );
    q.addBindValue( playcount );
    q.addBindValue( rating );
    q.addBindValue( id );
    q.exec();
    
    return id;
}
          
/* ---------------------------------------------------------------------------*/
/* DatabaseCmd::updateFavorite                                                */
/* ---------------------------------------------------------------------------*/
void DatabaseCmd::updateFavorite(MEDIA::MediaPtr media, bool isFavorite)
{
    if(media->type() == TYPE_ALBUM)
    {
       MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast( media );
      
       foreach (const int &id, album->dbIds()) 
       {
         QSqlQuery q("",*Database::instance()->db());
         q.prepare("UPDATE `albums` SET `favorite`=? WHERE `id`=?;");
         q.addBindValue( int(isFavorite) );
         q.addBindValue( id );
         q.exec();
       }
    }      
    else if (media->type() == TYPE_ARTIST)
    {
        MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast( media );
      
        QSqlQuery q("",*Database::instance()->db());
        q.prepare("UPDATE `artists` SET `favorite`=? WHERE `id`=?;");
        q.addBindValue( int(isFavorite) );
        q.addBindValue( artist->id );
        q.exec();      
    }
    else if (media->type() == TYPE_PLAYLIST)
    {
        MEDIA::PlaylistPtr playlist = MEDIA::PlaylistPtr::staticCast( media );

        QSqlQuery q("",*Database::instance()->db());
        if(playlist->p_type == T_DATABASE || playlist->p_type == T_FILE)
          q.prepare("UPDATE `playlists` SET `favorite`=? WHERE `id`=?;");
        else
          q.prepare("UPDATE `smart_playlists` SET `favorite`=? WHERE `id`=?;");

        q.addBindValue( int(isFavorite) );
        q.addBindValue( playlist->id );
        q.exec();
    }
}


/* ---------------------------------------------------------------------------*/
/* DatabaseCmd::rateMediaItems                                                */
/* ---------------------------------------------------------------------------*/
void DatabaseCmd::rateMediaItems(QList<MEDIA::MediaPtr> list)
{
    //Debug::debug() << "- DatabaseCmd -> rate media item";
  
    if( !Database::instance()->open() )
      return;
  
    foreach(MEDIA::MediaPtr media, list)
    {
        switch(media->type())
        {
          case TYPE_TRACK  : rateTrack( MEDIA::TrackPtr::staticCast( media ) ); break;
          case TYPE_ALBUM  : rateAlbum( MEDIA::AlbumPtr::staticCast( media ) ); break;
          case TYPE_ARTIST : rateArtist( MEDIA::ArtistPtr::staticCast( media ) ); break;
          default : break;
        }
    }
}


/* ---------------------------------------------------------------------------*/
/* DatabaseCmd::rateTrack                                                     */
/* ---------------------------------------------------------------------------*/
void DatabaseCmd::rateTrack(MEDIA::TrackPtr track)
{
    if(track->id != -1) 
    {
      QSqlQuery q("", *Database::instance()->db());
      q.prepare("UPDATE `tracks` SET `rating`=? WHERE `id`=?;");
      q.addBindValue( track->rating );
      q.addBindValue( track->id );
      q.exec();

      MEDIA::AlbumPtr album = MEDIA::AlbumPtr::staticCast(track->parent());
      if(!album->isUserRating)
         album->rating = LocalTrackModel::instance()->getItemAutoRating(album);

      MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(album->parent());
      if(!artist->isUserRating)
         artist->rating = LocalTrackModel::instance()->getItemAutoRating(artist);
      
      if( Database::instance()->param()._option_wr_rating_to_file )
        Tag::writeTrackRatingToFile( track->url, track->rating );
    }
}

/* ---------------------------------------------------------------------------*/
/* DatabaseCmd::rateArtist                                                    */
/* ---------------------------------------------------------------------------*/
void DatabaseCmd::rateArtist(MEDIA::ArtistPtr artist)
{
    if(artist->id != -1)
    {
      QSqlQuery q("", *Database::instance()->db());
      q.prepare("UPDATE `artists` SET `rating`=? WHERE `id`=?;");
      q.addBindValue( artist->rating );
      q.addBindValue( artist->id );
      q.exec();
    }
}

/* ---------------------------------------------------------------------------*/
/* DatabaseCmd::rateAlbum                                                     */
/* ---------------------------------------------------------------------------*/
void DatabaseCmd::rateAlbum(MEDIA::AlbumPtr album)
{
    if(album->id != -1) 
    {
      QList<int> db_ids;
      if(album->isMultiset())
        db_ids << album->ids;
      else
        db_ids << album->id;

      foreach (const int &id, db_ids) 
      {      
        QSqlQuery q("", *Database::instance()->db());
        q.prepare("UPDATE `albums` SET `rating`=? WHERE `id`=?;");
        q.addBindValue( album->rating );
        q.addBindValue( id );
        q.exec();

        MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(album->parent());
        if(!artist->isUserRating)
           artist->rating = LocalTrackModel::instance()->getItemAutoRating(artist);
      }
   }
}



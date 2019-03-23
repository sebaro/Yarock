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

#ifndef _DATABASE_CMD_H_
#define _DATABASE_CMD_H_

#include "mediaitem.h"

/*
********************************************************************************
*                                                                              *
*    Class DatabaseCmd                                                         *
*                                                                              *
********************************************************************************
*/
class DatabaseCmd 
{
public:
  DatabaseCmd();
  
  static int  insertGenre(const QString & genre);
  static int  insertYear(int year);
  
  static bool isArtistExists(const QString & artist);
  static int  insertArtist(const QString & artist);
  static int  updateArtist(const QString & artist, bool favorite, int playcount, float rating);
  
  static bool isAlbumExists(const QString & album,int artist_id);
  static int  insertAlbum(const QString & album, int artist_id,int year,int disc);
  static int  updateAlbum(const QString & album, int artist_id,int year, int disc, bool favorite, int playcount, float rating);
  static void clean();
  
  static void updateFavorite(MEDIA::MediaPtr media, bool isFavorite);
  static void addStreamToFavorite(MEDIA::TrackPtr stream);
  static void removeStreamToFavorite(MEDIA::TrackPtr stream);
  static void updateStreamFavorite(MEDIA::TrackPtr stream);
  
  static void rateMediaItems(QList<MEDIA::MediaPtr> list);
  
private:
  static void rateTrack(MEDIA::TrackPtr track);
  static void rateArtist(MEDIA::ArtistPtr artist);
  static void rateAlbum(MEDIA::AlbumPtr album);
};

#endif //_DATABASE_CMD_H_

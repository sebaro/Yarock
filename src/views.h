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
#ifndef _VIEWS_H_
#define _VIEWS_H_


namespace VIEW {
  enum Type {    
      ABOUT = 0,
      CONTEXT,
      SETTING, 
      FILESYSTEM,
      LOCAL,   
      RADIO
  };

  enum Id {    
      ViewAbout             = 1,
      ViewSettings          = 2,
      ViewDashBoard         = 3,
      ViewContext           = 4,
      ViewHistory           = 5,
      ViewArtist            = 6,
      ViewAlbum             = 7,
      ViewTrack             = 8,
      ViewGenre             = 9,
      ViewYear              = 10,
      ViewFavorite          = 11,
      ViewPlaylist          = 12,
      ViewSmartPlaylist     = 13,
      ViewDirble            = 14,
      ViewShoutCast         = 15,
      ViewTuneIn            = 16,
      ViewFavoriteRadio     = 17,
      ViewFileSystem        = 18   
  };
  
  inline static bool isIdValid(VIEW::Id id)
  {
    return (id >= VIEW::ViewAbout && id <= ViewFileSystem);
  };
  
  inline static VIEW::Type typeForView(VIEW::Id id)
  {
    switch(id)
    {
      case ViewAbout             : return VIEW::ABOUT;  break;
      case ViewSettings          : return VIEW::SETTING;break;
      case ViewDashBoard         : return VIEW::LOCAL;  break;
      case ViewContext           : return VIEW::CONTEXT;break;
      case ViewHistory           : return VIEW::LOCAL;  break;
      case ViewArtist            : return VIEW::LOCAL;  break;
      case ViewAlbum             : return VIEW::LOCAL;  break;
      case ViewTrack             : return VIEW::LOCAL;  break;
      case ViewGenre             : return VIEW::LOCAL;  break;
      case ViewYear              : return VIEW::LOCAL;  break;
      case ViewFavorite          : return VIEW::LOCAL;  break;
      case ViewPlaylist          : return VIEW::LOCAL;  break;
      case ViewSmartPlaylist     : return VIEW::LOCAL;  break;
      case ViewDirble            : return VIEW::RADIO;  break;
      case ViewShoutCast         : return VIEW::RADIO;  break;
      case ViewTuneIn            : return VIEW::RADIO;  break;
      case ViewFavoriteRadio     : return VIEW::RADIO;  break;
      case ViewFileSystem        : return VIEW::FILESYSTEM;break;
      default:break;
    }    
    return VIEW::LOCAL;
  };
  
  enum ViewAlbum_Type {
   album_grid     = 0,
   album_extended
  };

  enum ViewPlaylist_Type {
   playlist_overview = 0,
   playlist_by_tracks
  };
  
} // end namespace VIEW
 

#endif // _VIEWS_H_

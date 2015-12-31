/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2016 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

#ifndef _SEARCH_ENGINE_H_
#define _SEARCH_ENGINE_H_


#include "core/mediasearch/media_search.h"
#include "core/mediaitem/mediaitem.h"

#include <QList>
#include <QVariant>
#include <QDate>
#include <QTime>

/*
********************************************************************************
*                                                                              *
*    Class SearchEngine                                                        *
*                                                                              *
********************************************************************************
*/

class SearchEngine
{
  public:
    SearchEngine();

    void init_search_engine(const MediaSearch& search);

    void doSearch(bool for_playqueue = false);

    QList<MEDIA::TrackPtr> result() {return list_result_media_;}

    bool mediaMatch(const MediaSearch& search,const MEDIA::TrackPtr track);
    
  private:
    MediaSearch             search_;
    QList<MEDIA::TrackPtr>  list_result_media_;

  private:
    typedef QList<MEDIA::TrackPtr>::iterator media_iterator;

    QVariant fieldData(SearchQuery::Search_Field field, MEDIA::TrackPtr track);

    bool match(SearchQuery::Search_Operator op, SearchQuery::Field_Type type, QVariant data_field, QVariant pattern);

    bool match_string(SearchQuery::Search_Operator op, QString field_string, QString pattern);
    bool match_number(SearchQuery::Search_Operator op, int field_number, int pattern);
    bool match_time(SearchQuery::Search_Operator op, int field_second, int pattern);
    bool match_date(SearchQuery::Search_Operator op, QDate field_date, QDate pattern);
    bool match_rating(SearchQuery::Search_Operator op, float field_rating, float pattern);

    bool sortMedia(const MEDIA::TrackPtr track1,const MEDIA::TrackPtr track2);
    void sortMedias(media_iterator start,media_iterator end);
};


#endif //_SEARCH_ENGINE_H_

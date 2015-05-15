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

#include "core/mediasearch/media_search_engine.h"

#include "models/local/local_track_model.h"
#include "playqueue/playqueue_model.h"

#include "debug.h"

#include <QtAlgorithms>    // qSort/qSwap
#include <QtGlobal>        // qrand
#include <algorithm>       // for std::random_shuffle
/*
********************************************************************************
*                                                                              *
*    Class SearchEngine                                                        *
*                                                                              *
********************************************************************************
*/
SearchEngine::SearchEngine()
{
    search_.search_type_      = MediaSearch::Type_And;
    search_.query_list_.clear();
    search_.sort_type_        = MediaSearch::Sort_No;
    search_.sort_field_       = SearchQuery::field_track_trackname;
    search_.limit_            = -1;
}


void SearchEngine::init_search_engine(const MediaSearch& search)
{
    search_ = MediaSearch(search);
#ifdef TEST_FLAG
    Debug::debug() << "      [SearchEngine] init_search_engine search_type_ :" << (int)search_.search_type_;

    foreach(SearchQuery query, search_.query_list_) {
      Debug::debug() << "      [SearchEngine] init_search_engine query_list_  query.field_ :" << SearchQuery::FieldName(query.field_);
      Debug::debug() << "      [SearchEngine] init_search_engine query_list_  query.operator_ :" << query.operator_;
      Debug::debug() << "      [SearchEngine] init_search_engine query_list_  query.value_ :" << query.value_;
    }

    Debug::debug() << "      [SearchEngine] init_search_engine sort_type_   :" << (int)search_.sort_type_;
    Debug::debug() << "      [SearchEngine] init_search_engine sort_field_  :" << SearchQuery::FieldName(search_.sort_field_);
    Debug::debug() << "      [SearchEngine] init_search_engine limit_       :" << (int)search_.limit_;
#endif
}



bool SearchEngine::mediaMatch(const MediaSearch& search, MEDIA::TrackPtr track)
{
    bool result;
    for(int i =0; i< search.query_list_.size();i++)
    {
        SearchQuery::Search_Field     field     = search.query_list_.at(i).field_;
        SearchQuery::Search_Operator  op        = search.query_list_.at(i).operator_;
        QVariant                      pattern   = search.query_list_.at(i).value_;

        QVariant value = QVariant::fromValue(fieldData(field,track));

        result =  match(op, SearchQuery::TypeOf(field), value, pattern);

        if(result == true && search.search_type_ == MediaSearch::Type_Or)
        {
          return true;
        }
        else if(result == false && search.search_type_ == MediaSearch::Type_And)
        {
          return false;
        }
    }
    
    /* return last result */
    return result;
}


void SearchEngine::doSearch(bool for_playqueue /*= false*/)
{
    Debug::debug() << "      [SearchEngine] doSearch";

    list_result_media_.clear();
    
    // un-ordered media track
    QList<MEDIA::TrackPtr> list_media;
    if(!for_playqueue)
      list_media = LocalTrackModel::instance()->trackItemHash.values();
    else
      list_media = Playqueue::instance()->tracks();
    
    QList<MEDIA::TrackPtr> list_media2;

    //! sort random
    if(search_.sort_type_ == MediaSearch::Sort_Random) 
    {
      std::random_shuffle( list_media.begin(), list_media.end() );
    }
    //! sort by field
    else if(search_.sort_type_ == MediaSearch::Sort_FieldAsc || search_.sort_type_ == MediaSearch::Sort_FieldDesc )
    {
        if(search_.sort_field_ == SearchQuery::field_track_lastPlayed)
        {
          for (int index = 0; index < list_media.size(); index++) {
            if( list_media.at(index)->lastPlayed != -1)
              list_media2 << list_media.at(index);
          }
          list_media = list_media2;
        }

        this->sortMedias(list_media.begin(), list_media.end());
    }

    foreach (MEDIA::TrackPtr media, list_media)
    {
        if(search_.search_type_ == MediaSearch::Type_All)
        {
            list_result_media_ << media;
        }
        else
        {
            bool skip_track = false;

            for(int i =0; i< search_.query_list_.size();i++)
            {
              SearchQuery::Search_Field     field     = search_.query_list_.at(i).field_;
              SearchQuery::Search_Operator  op        = search_.query_list_.at(i).operator_;
              QVariant                      pattern   = search_.query_list_.at(i).value_;

              QVariant value = QVariant::fromValue(fieldData(field,media));

              bool result =  match(op, SearchQuery::TypeOf(field), value, pattern);

              if(result == true && search_.search_type_ == MediaSearch::Type_Or)
              {
                list_result_media_ << media;
                skip_track = true;
              }
              else if(result == false && search_.search_type_ == MediaSearch::Type_And) {
                skip_track = true;
              }
              else if(result == true && search_.search_type_ == MediaSearch::Type_And &&  (i == search_.query_list_.size()-1))
              {
                list_result_media_ << media;
              }

              if(skip_track) break;
            } // end for

            if(skip_track) continue;
      }

      //! limit result
      if(search_.limit_ != -1)
      {
         if(list_result_media_.size() >= search_.limit_)
            break;
      }
    } // end foreach

}


QVariant SearchEngine::fieldData(SearchQuery::Search_Field field, MEDIA::TrackPtr track)
{
    //Debug::debug() << "      [SearchEngine] fieldData ";
    if(track->type() == TYPE_TRACK)
    {    
      switch(field)
      {
        case SearchQuery::field_track_filename  : return QVariant(track->url);       break;
        case SearchQuery::field_track_trackname : return QVariant(track->title);     break;
        case SearchQuery::field_track_number    : return QVariant(track->num);       break;

        case SearchQuery::field_track_length    : return QVariant(track->duration);  break;
        case SearchQuery::field_artist_name     : return QVariant(track->artist);    break;
        case SearchQuery::field_album_name      : return QVariant(track->album);     break;
        case SearchQuery::field_genre_name      : return QVariant(track->genre);     break;

        case SearchQuery::field_track_year      : return QVariant( QDate(track->year, 1, 1) ); break;
        case SearchQuery::field_track_playcount : return QVariant(track->playcount); break;
        case SearchQuery::field_track_rating    : return QVariant(track->rating);    break;

        case SearchQuery::field_track_lastPlayed: return QVariant(QDateTime::fromTime_t(track->lastPlayed).date()); break;
        default : break;
      }
      
      if(track->id != -1)
      {
        MEDIA::AlbumPtr album   = MEDIA::AlbumPtr::staticCast(track->parent());
        MEDIA::ArtistPtr artist = MEDIA::ArtistPtr::staticCast(album->parent());

        switch(field)
        {
          case SearchQuery::field_album_year      : return QVariant( QDate(album->year, 1, 1) ); break;
          case SearchQuery::field_artist_playcount: return QVariant(artist->playcount);break;
          case SearchQuery::field_album_playcount : return QVariant(album->playcount);break;
          case SearchQuery::field_artist_rating   : return QVariant(artist->rating);break;
          case SearchQuery::field_album_rating    : return QVariant(album->rating);break;
          default :break;
        }
      }
    } /* end TYPE_TRACK */
    else if(track->type() == TYPE_STREAM)
    {
      switch(field)
      {      
        case SearchQuery::field_track_filename  : return QVariant(track->url);       break;
        case SearchQuery::field_track_trackname : return QVariant(track->name);      break;
        default:break;
      }
    }

    return QVariant();
}


bool SearchEngine::match(SearchQuery::Search_Operator op, SearchQuery::Field_Type type, QVariant data_field, QVariant pattern)
{
    switch (type)
    {
      case SearchQuery::type_Text   : return match_string(op, data_field.toString(), pattern.toString());break;
      case SearchQuery::type_Number : return match_number(op, data_field.toInt(), pattern.toInt()); break;

      /* for type_Time entry is integer = seconds  */
      case SearchQuery::type_Time   : return match_time(op, data_field.toInt(), QTime().secsTo(pattern.toTime()));break;
      case SearchQuery::type_Date   : return match_date(op, data_field.toDate(), pattern.toDate());      break;
      case SearchQuery::type_Rating : return match_rating(op, data_field.toFloat(), pattern.toFloat());  break;
      default : return false;
    }
}


bool SearchEngine::match_string (SearchQuery::Search_Operator op, QString field_string, QString pattern)
{
    //Debug::debug() << "      [SearchEngine] match_string " << field_string << " pattern " << pattern;

    switch (op) {
      case SearchQuery::op_StartsWith:
        return field_string.startsWith ( pattern, Qt::CaseInsensitive );
      case SearchQuery::op_EndsWith:
        return field_string.endsWith ( pattern, Qt::CaseInsensitive );
      case SearchQuery::op_Contains:
        return field_string.contains(pattern, Qt::CaseInsensitive);
      case SearchQuery::op_NotContains:
        return !field_string.contains(pattern, Qt::CaseInsensitive);
      case SearchQuery::op_Equals:
        return field_string == pattern;
      //! only use for sorting not matching
      case SearchQuery::op_GreaterThan:
        return field_string > pattern;
      case SearchQuery::op_LessThan:
        return field_string < pattern;

      default : return false;
    }

    return false;
}

bool SearchEngine::match_number(SearchQuery::Search_Operator op, int field_number, int pattern)
{
    switch (op) {
      case SearchQuery::op_GreaterThan  : return field_number > pattern;
      case SearchQuery::op_LessThan     : return field_number < pattern;
      case SearchQuery::op_Equals       : return field_number == pattern;
      case SearchQuery::op_NotEquals    : return field_number != pattern;
      default : return false;
    }
    return false;
}

bool SearchEngine::match_time(SearchQuery::Search_Operator op, int field_second /* second !*/, int pattern /* second !*/)
{
    switch (op) {
      case SearchQuery::op_GreaterThan:
        return field_second > pattern;
      case SearchQuery::op_LessThan:
        return field_second < pattern;
      case SearchQuery::op_Equals:
        return field_second == pattern;
      case SearchQuery::op_NotEquals:
        return field_second != pattern;
      default : return false;
    }
    return false;
}



bool SearchEngine::match_date(SearchQuery::Search_Operator op, QDate field_date, QDate pattern)
{
    switch (op) {
      case SearchQuery::op_GreaterThan : return field_date > pattern;  // after
      case SearchQuery::op_LessThan    : return field_date < pattern;  // before
      case SearchQuery::op_Equals      : return field_date == pattern; // date is on
      case SearchQuery::op_NotEquals   : return field_date != pattern; // date is not on
      default : return false;
    }
    return false;
}

bool SearchEngine::match_rating(SearchQuery::Search_Operator op, float field_rating, float pattern)
{
    //Debug::debug() << "      [SearchEngine] match_rating field_rating " << field_rating << " pattern " << pattern;

    switch (op) {
      case SearchQuery::op_GreaterThan : return field_rating > pattern;
      case SearchQuery::op_LessThan    : return field_rating < pattern;
      case SearchQuery::op_Equals      : return field_rating == pattern;
      case SearchQuery::op_NotEquals   : return field_rating != pattern;
      default : return false;
    }
    return false;
}



bool SearchEngine::sortMedia(const MEDIA::TrackPtr track1,const MEDIA::TrackPtr track2)
{
    //Debug::debug() << "      [SearchEngine] sortMedia";

    SearchQuery::Search_Field field = search_.sort_field_;

    SearchQuery::Search_Operator ope;

    if(search_.sort_type_ == MediaSearch::Sort_FieldAsc)
      ope = SearchQuery::op_LessThan;
    else
      ope = SearchQuery::op_GreaterThan;

    return match(ope, SearchQuery::TypeOf(field), fieldData(field, track1), fieldData(field, track2));
}

// code adapted from Qt qSort in qalgorithms.h
// no need to use boost::bind to use class member sortMedia function
void SearchEngine::sortMedias(media_iterator start,media_iterator end)
{
    //Debug::debug() << "      [SearchEngine] sortMedias";

top:
    int span = int(end - start);
    if (span < 2)
        return;

    --end;
    media_iterator low = start, high = end - 1;
    media_iterator pivot = start + span / 2;

    if (sortMedia(*end, *start))
        qSwap(*end, *start);
    if (span == 2)
        return;

    if (sortMedia(*pivot, *start))
        qSwap(*pivot, *start);
    if (sortMedia(*end, *pivot))
        qSwap(*end, *pivot);
    if (span == 3)
        return;

    qSwap(*pivot, *end);

    while (low < high) {
        while (low < high && sortMedia(*low, *end))
            ++low;

        while (high > low && sortMedia(*end, *high))
            --high;

        if (low < high) {
            qSwap(*low, *high);
            ++low;
            --high;
        } else {
            break;
        }
    }

    if (sortMedia(*low, *end))
        ++low;

    qSwap(*end, *low);
    // warning recursive !!
    sortMedias(start, low);

    start = low + 1;
    ++end;
    goto top;
}


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

#include "core/mediasearch/media_search.h"
//#include "debug.h"
/*
********************************************************************************
*                                                                              *
*    Class MediaSearch                                                         *
*                                                                              *
********************************************************************************
*/
MediaSearch::MediaSearch()
{
    init();
}


MediaSearch::MediaSearch(
              SearchType                 type,
              SearchQueryList            terms,
              SortType                   sort_type,
              SearchQuery::Search_Field  sort_field,
              int limit)
{
    search_type_ = type;
    query_list_  = terms;
    sort_type_   = sort_type;
    sort_field_  = sort_field;
    limit_       = limit;
}

/* ---------------------------------------------------------------------------*/
/* MediaSearch::init                                                          */
/* ---------------------------------------------------------------------------*/ 
void MediaSearch::init()
{
    search_type_ = Type_And;
    query_list_.clear();
    sort_type_  = Sort_No;
    sort_field_ = SearchQuery::field_track_trackname;
    limit_      = -1;
}

/* ---------------------------------------------------------------------------*/
/* MediaSearch::is_valid                                                      */
/* ---------------------------------------------------------------------------*/ 
bool MediaSearch::is_valid() const
{
    if (search_type_ == Type_All)
      return true;
    return !query_list_.isEmpty();
}

/* ---------------------------------------------------------------------------*/
/* MediaSearch::database stream                                               */
/* ---------------------------------------------------------------------------*/ 
MediaSearch MediaSearch::fromDatabase(QVariant variant)
{
    const QByteArray byte_array = variant.toByteArray();
    QDataStream s(byte_array);

    MediaSearch search;
    s >> search;

    return search;
}


QVariant MediaSearch::toDatabase(MediaSearch search)
{
    QByteArray byte_array;
    QDataStream s(&byte_array, QIODevice::ReadWrite);

    s << search;
    
    return QVariant(byte_array);
}


bool MediaSearch::operator ==(const MediaSearch& other) const
{
  return search_type_ == other.search_type_   &&
         query_list_  == other.query_list_    &&
         sort_type_   == other.sort_type_     &&
         sort_field_  == other.sort_field_    &&
         limit_       == other.limit_;
}

QDataStream& operator <<(QDataStream& s, const MediaSearch& media_search)
{
    //Debug::debug() << "      [MediaSearch] operator <<";
    s << media_search.query_list_;
    s << quint8(media_search.sort_type_);
    s << quint8(media_search.sort_field_);
    s << qint32(media_search.limit_);
    s << quint8(media_search.search_type_);
    return s;
}

QDataStream& operator >>(QDataStream& s, MediaSearch& media_search)
{
    //Debug::debug() << "      [MediaSearch] operator >>";
    quint8 sort_type, sort_field, search_type;
    qint32 limit;

    s >> media_search.query_list_ >> sort_type >> sort_field >> limit >> search_type;

    media_search.sort_type_   = MediaSearch::SortType(sort_type);
    media_search.sort_field_  = SearchQuery::Search_Field(sort_field);
    media_search.limit_       = limit;
    media_search.search_type_ = MediaSearch::SearchType(search_type);

    return s;
}

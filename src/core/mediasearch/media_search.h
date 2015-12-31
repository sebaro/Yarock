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

#ifndef _MEDIA_SEARCH_H_
#define _MEDIA_SEARCH_H_

#include "core/mediasearch/search_query.h"

#include <QDataStream>

/*
********************************************************************************
*                                                                              *
*    Class MediaSearch                                                         *
*                                                                              *
********************************************************************************
*/

class MediaSearch
{
public:
  enum SearchType {
    Type_And = 0,
    Type_Or,
    Type_All
  };

  enum SortType {
    Sort_No = 0,
    Sort_Random,
    Sort_FieldAsc,
    Sort_FieldDesc,
  };

  MediaSearch();
  MediaSearch(SearchType                 type,
              SearchQueryList            terms,
              SortType                   sort_type,
              SearchQuery::Search_Field  sort_field,
              int limit = 50);

  void init();

  bool is_valid() const;
  bool operator ==(const MediaSearch& other) const;
  bool operator !=(const MediaSearch& other) const { return !(*this == other); }

  static MediaSearch fromDatabase(QVariant v);
  static QVariant toDatabase(MediaSearch search);
  
  SearchType                   search_type_;
  SearchQueryList              query_list_;
  SortType                     sort_type_;
  SearchQuery::Search_Field    sort_field_;
  int                          limit_;
};

Q_DECLARE_METATYPE(MediaSearch);

QDataStream& operator <<(QDataStream& s, const MediaSearch& media_search);
QDataStream& operator >>(QDataStream& s, MediaSearch& media_search);

#endif // _MEDIA_SEARCH_H_


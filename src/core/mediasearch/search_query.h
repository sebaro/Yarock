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

#ifndef _SEARCH_QUERY_H_
#define _SEARCH_QUERY_H_


#include <QList>
#include <QVariant>

/*******************************************************************************
  Enum and types
*******************************************************************************/
#define CST_SEARCH_FIELD_COUNT 16


/*
********************************************************************************
*                                                                              *
*    Class SearchQuery                                                         *
*                                                                              *
********************************************************************************
*/
class SearchQuery
{
public:
//! ---------------- Enum ------------------------------------------------------
  enum Search_Field
  {
      field_track_filename = 0,   // type_Text
      field_track_trackname,      // type_Text
      field_track_number,         // type_Number
      field_track_length,         // type_Time
      field_track_playcount,      // type_Number
      field_track_year,           // type_Date
      field_artist_name,          // type_Number
      field_artist_playcount,     // type_Number
      field_album_name,           // type_Text
      field_album_year,           // type_Date
      field_album_playcount,      // type_Number
      field_genre_name,           // type_Text
      field_track_lastPlayed,     // type_Date
      field_track_rating,         // type_Rating (Float number)
      field_album_rating,         // type_Rating (Float number)
      field_artist_rating         // type_Rating (Float number)
  };

  enum Search_Operator
  {
      // For text
      op_Contains     = 0,
      op_NotContains,
      op_StartsWith,
      op_EndsWith,

      // For numbers & date
      op_GreaterThan,
      op_LessThan,

      // For everything
      op_Equals,

      // For number & date
      op_NotEquals
  };

  enum Field_Type {
      type_Text,
      type_Date,
      type_Time,
      type_Number,
      type_Rating
  };

  typedef QList<Search_Operator> OperatorList;

//! ---------------- Class -----------------------------------------------------
  SearchQuery();
  SearchQuery(Search_Field field, Search_Operator op, const QVariant& value);

  Search_Field            field_;
  Search_Operator         operator_;
  QVariant                value_;

  bool is_valid() const;
  bool operator ==(const SearchQuery& other) const;
  bool operator !=(const SearchQuery& other) const { return !(*this == other); }


  static SearchQuery::Field_Type TypeOf(SearchQuery::Search_Field field);

  static SearchQuery::OperatorList OperatorsForType(SearchQuery::Field_Type type);

  static QString OperatorText(SearchQuery::Field_Type type, SearchQuery::Search_Operator op);

  static QString FieldName(SearchQuery::Search_Field field);

  static QString FieldSortOrderText(SearchQuery::Field_Type type, bool ascending);

};

typedef QList<SearchQuery> SearchQueryList;


QDataStream& operator <<(QDataStream& s, const SearchQuery& search_query);
QDataStream& operator >>(QDataStream& s, SearchQuery& search_query);

#endif // _SEARCH_QUERY_H_

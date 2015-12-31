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
#include "core/mediasearch/search_query.h"

#include <QDate>


/*
********************************************************************************
*                                                                              *
*    Class SearchQuery                                                         *
*                                                                              *
********************************************************************************
*/

SearchQuery::SearchQuery()
{
    field_     = field_track_trackname;
    operator_  = op_Contains;
}

SearchQuery::SearchQuery (Search_Field field, Search_Operator op, const QVariant& value)
{
    field_     = field;
    operator_  = op;
    value_     = value;
}

SearchQuery::Field_Type SearchQuery::TypeOf(SearchQuery::Search_Field field)
{
    switch (field) {
      case field_track_filename :
      case field_track_trackname:
      case field_artist_name:
      case field_album_name:
      case field_genre_name:
        return type_Text;

      case field_track_year:
      case field_album_year:
      case field_track_lastPlayed:
        return type_Date;

      case field_track_length:
        return type_Time;

      case field_track_number:
      case field_track_playcount:
      case field_artist_playcount:
      case field_album_playcount:
        return type_Number;

      case field_track_rating:
      case field_album_rating:
      case field_artist_rating:
        return type_Rating;

      default:
        return type_Text;
    }
}


bool SearchQuery::is_valid() const
{
    switch (TypeOf(field_)) {
      case type_Text:   return !value_.toString().isEmpty();
      case type_Date:   return value_.toDate().year() != 0;
      case type_Number: return value_.toInt() >= 0;
      case type_Rating: return value_.toFloat() >= 0.0;
      case type_Time:   return true;
    }
    return false;
}


bool SearchQuery::operator ==(const SearchQuery& other) const
{
    return (field_    == other.field_     &&
            operator_ == other.operator_  &&
            value_    == other.value_);
}


SearchQuery::OperatorList SearchQuery::OperatorsForType(SearchQuery::Field_Type type)
{
    switch (type) {
      case type_Text:
        return OperatorList() << op_Contains << op_NotContains << op_Equals
                            << op_StartsWith << op_EndsWith;
      case type_Date:
        return OperatorList() << op_Equals << op_NotEquals << op_GreaterThan << op_LessThan;

      default:
        return OperatorList() << op_Equals << op_NotEquals << op_GreaterThan << op_LessThan;
    }
}



QString SearchQuery::FieldName(SearchQuery::Search_Field field)
{
    switch (field) {
      case field_track_filename :  return QObject::tr("Filename");
      case field_track_trackname:  return QObject::tr("Title");
      case field_album_name:       return QObject::tr("Album");
      case field_genre_name:       return QObject::tr("Genre");
      case field_track_length:     return QObject::tr("Duration");
      case field_track_year:       return QObject::tr("Track Year");
      case field_album_year:       return QObject::tr("Album Year");
      case field_track_number:     return QObject::tr("Number");
      case field_track_playcount:  return QObject::tr("Track Playcount");
      case field_artist_playcount: return QObject::tr("Album Playcount");
      case field_album_playcount:  return QObject::tr("Artist Playcount");
      case field_artist_name:      return QObject::tr("Artist");
      case field_track_lastPlayed: return QObject::tr("Last Played date");
      case field_track_rating:     return QObject::tr("Track rating");
      case field_album_rating:     return QObject::tr("Album rating");
      case field_artist_rating:    return QObject::tr("Artist rating");
      default: return QString();
    }
}




QString SearchQuery::OperatorText(SearchQuery::Field_Type type, Search_Operator op)
{
    if (type == type_Date) {
      switch (op) {
        case op_GreaterThan:    return QObject::tr("after");
        case op_LessThan:       return QObject::tr("before");
        case op_Equals:         return QObject::tr("on");
        case op_NotEquals:      return QObject::tr("not on");
        default:                return QString();
      }
    }

    switch (op) {
      case op_Contains:    return QObject::tr("contains");
      case op_NotContains: return QObject::tr("does not contain");
      case op_StartsWith:  return QObject::tr("starts with");
      case op_EndsWith:    return QObject::tr("ends with");
      case op_GreaterThan: return QObject::tr("greater than");
      case op_LessThan:    return QObject::tr("less than");
      case op_Equals:      return QObject::tr("equals");
      case op_NotEquals:   return QObject::tr("not equals");
      default:             return QString();
    }

    return QString();
}

QString SearchQuery::FieldSortOrderText(SearchQuery::Field_Type type, bool ascending)
{
    switch (type) {
      case type_Text:   return ascending ? QObject::tr("A-Z")            : QObject::tr("Z-A");
      case type_Date:   return ascending ? QObject::tr("oldest first")   : QObject::tr("newest first");
      case type_Time:   return ascending ? QObject::tr("shortest first") : QObject::tr("longest first");
      case type_Number:
      case type_Rating: return ascending ? QObject::tr("smallest first") : QObject::tr("biggest first");
    }
    return QString();
}


QDataStream& operator <<(QDataStream& s, const SearchQuery& query)
{
    s << quint8(query.field_);
    s << quint8(query.operator_);
    s << query.value_;

    return s;
}

QDataStream& operator >>(QDataStream& s, SearchQuery& query)
{
    quint8 field, op;
    s >> field >> op >> query.value_;

    query.field_      = SearchQuery::Search_Field(field);
    query.operator_   = SearchQuery::Search_Operator(op);

    return s;
}



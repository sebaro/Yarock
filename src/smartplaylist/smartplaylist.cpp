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

#include "smartplaylist.h"

#include "core/database/database.h"
#include "core/mediasearch/media_search.h"
#include "core/mediasearch/media_search_engine.h"
#include "dialog_base.h"

#include "debug.h"

// Qt
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>
#include <QtSql/QSqlResult>

#include <QString>
#include <QDataStream>


/*
********************************************************************************
*                                                                              *
*    Class SmartPlaylist                                                       *
*                                                                              *
********************************************************************************
*/
SmartPlaylist::SmartPlaylist(QObject *parent)  : QObject( parent )
{

}

/*******************************************************************************
 createDatabase
*******************************************************************************/
void SmartPlaylist::createDatabase()
{

const QStringList p_names = QStringList()
            << tr("50 Random tracks")
            << tr("50 last played")
            << tr("50 most played")
            << tr("ever played")
            << tr("never played")
            << tr("top rated tracks");

const QStringList p_icon = QStringList()
            << ":/images/media-smartplaylist1-110x110.png"
            << ":/images/media-smartplaylist2-110x110.png"
            << ":/images/media-smartplaylist4-110x110.png"
            << ":/images/media-smartplaylist4-110x110.png"
            << ":/images/media-smartplaylist4-110x110.png"
            << ":/images/media-smartplaylist3-110x110.png";


  /*
   *  MediaSearch(SearchType                 type,
   *              SearchQueryList            terms,
   *              SortType                   sort_type,
   *              SearchQuery::Search_Field  sort_field,
   *              int limit = 50);
   */

QList<MediaSearch> list_search = QList<MediaSearch>()
  << MediaSearch(MediaSearch::Type_All, SearchQueryList(), MediaSearch::Sort_Random, SearchQuery::field_track_trackname, 50)

  << MediaSearch(MediaSearch::Type_All, SearchQueryList(), MediaSearch::Sort_FieldDesc, SearchQuery::field_track_lastPlayed,  50)

  << MediaSearch(MediaSearch::Type_All, SearchQueryList(), MediaSearch::Sort_FieldDesc, SearchQuery::field_track_playcount,  50)

  << MediaSearch( MediaSearch::Type_And,
                  SearchQueryList() << SearchQuery(SearchQuery::field_track_playcount, SearchQuery::op_GreaterThan, 0),
                  MediaSearch::Sort_Random, SearchQuery::field_track_trackname,  -1)

  << MediaSearch( MediaSearch::Type_And,
                  SearchQueryList() << SearchQuery(SearchQuery::field_track_playcount, SearchQuery::op_Equals, 0),
                  MediaSearch::Sort_Random, SearchQuery::field_track_trackname,  500)

  << MediaSearch( MediaSearch::Type_And,
                  SearchQueryList() << SearchQuery(SearchQuery::field_track_rating, SearchQuery::op_GreaterThan, 0.5),
                  MediaSearch::Sort_FieldDesc, SearchQuery::field_artist_name,  -1);




    QSqlQuery query_1("DELETE FROM `smart_playlists`;", *Database::instance()->db() );

    for (int i=0; i < p_names.size(); i++)
    {
      Debug::debug() << "--- SMART_PLAYLIST::createDatabase -> insert smart playlist :" << p_names.at(i);

      QVariant variant = MediaSearch::toDatabase( list_search.at(i) );

      QSqlQuery query( *Database::instance()->db() );
      query.prepare("INSERT INTO `smart_playlists`(`name`,`icon`,`rules`,`type`,`favorite`)" \
                      "VALUES(?,?,?,?,?);");

      query.addBindValue( p_names.at(i) );
      query.addBindValue( p_icon.at(i) );
      query.addBindValue( variant );
      query.addBindValue( (int) T_SMART );
      query.addBindValue( 0 );
      Debug::debug() << "exec " << query.exec();
    }
}



/*******************************************************************************
 SmartPlaylist::mediaItem
*******************************************************************************/
QList<MEDIA::TrackPtr> SmartPlaylist::mediaItem(QVariant variant)
{
    //Debug::debug() << "SmartPlaylist--> get mediaitem start : " << QTime::currentTime().toString();

    MediaSearch search = qvariant_cast<MediaSearch>(variant);

    SearchEngine* search_engine = new SearchEngine();
    search_engine->init_search_engine(search);
    search_engine->doSearch();

    //Debug::debug() << "SmartPlaylist--> get mediaitem end : " << QTime::currentTime().toString();
    return  search_engine->result();
}

/*******************************************************************************
 SmartPlaylist::applyChange
*******************************************************************************/
void SmartPlaylist::updatePlaylist(MEDIA::PlaylistPtr playlist)
{
    Debug::debug() << "SmartPlaylist::updatePlaylist";
    
    if (!Database::instance()->open()) return;

    QSqlQuery q("",*Database::instance()->db());
    q.prepare("SELECT `id` FROM `smart_playlists` WHERE `id`=?;");
    q.addBindValue( playlist->id );
    q.exec();

    MediaSearch search = qvariant_cast<MediaSearch>(playlist->rules);
    
    if ( !q.next() ) 
    {
      q.prepare("INSERT INTO `smart_playlists`(`name`,`icon`,`rules`,`type`,`favorite`) VALUES(?,?,?,?,?);");
     
      q.addBindValue(playlist->name);
      q.addBindValue(":/images/media-smartplaylist4-110x110.png");
      q.addBindValue(MediaSearch::toDatabase( search ));
      q.addBindValue((int) T_SMART);
      q.addBindValue((int)playlist->isFavorite);
      Debug::debug() << "query exec " << q.exec();
    }
    else
    {
      q.prepare("UPDATE `smart_playlists` SET `name`=?,`rules`=? WHERE `id`=?;");
      q.addBindValue(playlist->name);
      q.addBindValue(MediaSearch::toDatabase( search ));
      q.addBindValue( playlist->id );
      Debug::debug() << "query exec " << q.exec();
    }
}


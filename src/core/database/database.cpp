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

#include "database.h"
#include "statusmanager.h"

#include "smartplaylist.h"
#include "utilities.h"
#include "debug.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QFile>
#include <QThread>
#include <QMap>
#include <QSettings>

#define CST_DATABASE_REV  20



Database* Database::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class Database                                                            *
*                                                                              *
********************************************************************************
*/
Database::Database()
{
    INSTANCE = this;

    settings_restore();
    
    Debug::debug() << "[Database] load database " << param()._name;
}


Database::~Database() {}


/* ---------------------------------------------------------------------------*/
/* Database::create                                                           */
/* ---------------------------------------------------------------------------*/ 
void Database::create()
{
    Debug::debug() << "[Database] create database scheme";
    if(! this->open(true) ) {
      Debug::error() << "[Database] error creating database file";      
      return;
    }
      
    QSqlQuery query( *db() );
    Debug::debug() << query.exec("CREATE TABLE `directories` ("         \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                 "    `path` TEXT NOT NULL,"                            \
                 "    `mtime` INTEGER);");
    
    Debug::debug() << query.exec("CREATE TABLE `genres` ("              \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                 "    `genre` TEXT NOT NULL);");

    Debug::debug() << query.exec("CREATE TABLE `artists` ("             \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                 "    `name` TEXT NOT NULL,"                            \
                 "    `favorite` INTEGER DEFAULT 0, "                   \
                 "    `playcount` INTEGER DEFAULT 0, "                  \
                 "    `rating` INTEGER);");

   Debug::debug() << query.exec("CREATE TABLE `albums` ("               \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                 "    `name` TEXT NOT NULL,"                            \
                 "    `artist_id` INTEGER NOT NULL,"                    \
                 "    `year` INTEGER NOT NULL,"                         \
                 "    `disc` INTEGER DEFAULT 0, "                       \
                 "    `favorite` INTEGER DEFAULT 0, "                   \
                 "    `playcount` INTEGER DEFAULT 0, "                  \
                 "    `rating` INTEGER);");

   Debug::debug() << query.exec("CREATE TABLE `tracks` ("               \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                 "    `filename` TEXT NOT NULL,"                        \
                 "    `trackname` TEXT NOT NULL,"                       \
                 "    `number` INTEGER,"                                \
                 "    `length` INTEGER NULL,"                           \
                 "    `artist_id` INTEGER NOT NULL,"                    \
                 "    `album_id` INTEGER NOT NULL,"                     \
                 "    `genre_id` INTEGER NOT NULL,"                     \
                 "    `year_id` INTEGER NOT NULL,"                      \
                 "    `dir_id` INTEGER NOT NULL,"                       \
                 "    `mtime` INTEGER, "                                \
                 "    `playcount` INTEGER DEFAULT 0,"                   \
                 "    `rating`  INTEGER DEFAULT 0,"                     \
                 "    `bitrate` INTEGER DEFAULT 0,"                     \
                 "    `samplerate` INTEGER DEFAULT 0,"                  \
                 "    `bpm` REAL NULL,"                                 \
                 "    `albumgain` REAL NULL,"                           \
                 "    `albumpeakgain` REAL NULL,"                       \
                 "    `trackgain` REAL NULL,"                           \
                 "    `trackpeakgain` REAL NULL);");

    Debug::debug() << query.exec("CREATE TABLE `years` (" \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                 "    `year` INTEGER NOT NULL);");

    Debug::debug() << query.exec("CREATE TABLE `db_attribute` (" \
                 "    `name` VARCHAR(255)," \
                 "    `value` TEXT);");

    Debug::debug() << "[Database] insert database revision";

    Debug::debug() << query.exec("INSERT INTO db_attribute (name, value) values ('version', "+QString::number(CST_DATABASE_REV)+");");
    Debug::debug() << query.exec("INSERT INTO db_attribute (name, value) values ('lastUpdate', 0);");


    //! HISTO
    query.exec("CREATE TABLE `histo` ("                                 \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
                 "    `url` TEXT NOT NULL,"                             \
                 "    `name` TEXT NOT NULL,"                            \
                 "    `date` INTEGER);");


     Debug::debug() << query.exec("CREATE VIEW `view_histo` AS"              \
                 "    SELECT `histo`.`id`,"                                  \
                 "           `histo`.`url`,"                                 \
                 "           `histo`.`name`,"                                \
                 "           `histo`.`date`,"                                \
                 "           `tracks`.`id` AS `track_id`"                    \
                 "    FROM `histo`"                                          \
                 "    LEFT OUTER JOIN `tracks` ON `histo`.`url` = `tracks`.`filename`;");

    //! VIEW_TRACKS
    Debug::debug() << query.exec("CREATE VIEW `view_tracks` AS"                \
                 "    SELECT `tracks`.`id`,"                                   \
                 "           `tracks`.`filename`,"                             \
                 "           `tracks`.`trackname`,"                            \
                 "           `tracks`.`number`,"                               \
                 "           `tracks`.`length`,"                               \
                 "           `tracks`.`playcount`,"                            \
                 "           `tracks`.`rating`,"                               \
                 "           `tracks`.`artist_id`,"                            \
                 "           `tracks`.`album_id`,"                             \
                 "           `tracks`.`genre_id`,"                             \
                 "           `artists`.`name` AS `artist_name`,"               \
                 "           `artists`.`favorite` AS `artist_favorite`,"       \
                 "           `artists`.`playcount` AS `artist_playcount`,"     \
                 "           `artists`.`rating` AS `artist_rating`,"           \
                 "           `albums`.`name` AS `album_name`,"                 \
                 "           `albums`.`year` AS `album_year`,"                 \
                 "           `albums`.`favorite` AS `album_favorite`,"         \
                 "           `albums`.`playcount` AS `album_playcount`,"       \
                 "           `albums`.`rating` AS `album_rating`,"             \
                 "           `albums`.`disc` AS `album_disc`,"                 \
                 "           `genres`.`genre` AS `genre_name`,"                \
                 "           `years`.`year`,"                                  \
                 "           `histo`.`date` AS `last_played`"                  \
                 "    FROM `tracks`"                                           \
                 "    LEFT JOIN `artists` ON `tracks`.`artist_id` = `artists`.`id`" \
                 "    LEFT JOIN `albums` ON `tracks`.`album_id` = `albums`.`id`"    \
                 "    LEFT JOIN `genres` ON `tracks`.`genre_id` = `genres`.`id`"    \
                 "    LEFT JOIN `years` ON `tracks`.`year_id` = `years`.`id`"       \
                 "    LEFT JOIN `histo` ON `tracks`.`filename` = `histo`.`url`;");


     Debug::debug() << query.exec("CREATE VIEW `view_albums` AS"               \
                 "    SELECT `albums`.`id`,"                                   \
                 "           `albums`.`name`,"                                 \
                 "           `albums`.`year`,"                                 \
                 "           `albums`.`favorite`,"                             \
                 "           `albums`.`playcount`,"                            \
                 "           `artists`.`name` AS `artist_name`,"               \
                 "           `artists`.`favorite` AS `artist_favorite`,"       \
                 "           `artists`.`playcount` AS `artist_playcount`"      \
                 "    FROM `albums`"                                           \
                 "    LEFT JOIN `artists` ON `albums`.`artist_id` = `artists`.`id`");

    //! playlist part
    // type playlist (0 = T_DATABASE, 1 = T_FILE, 2 = T_SMART, 3 = T_PLAYQUEUE)
    query.exec("CREATE TABLE `playlists` ("                                  \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"      \
                 "    `filename` TEXT NOT NULL,"                             \
                 "    `name` TEXT NOT NULL,"                                 \
                 "    `type` INTEGER,"                                       \
                 "    `favorite` INTEGER,"                                   \
                 "    `dir_id` INTEGER NOT NULL,"                            \
                 "    `mtime` INTEGER);");

    query.exec("CREATE TABLE `playlist_items` ("                             \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"      \
                 "    `url` TEXT NOT NULL,"                                  \
                 "    `name` TEXT NOT NULL,"                                 \
                 "    `playlist_id` INTEGER NOT NULL);");

     Debug::debug() << query.exec("CREATE VIEW `view_playlists` AS"          \
                 "    SELECT `playlist_items`.`url`,"                        \
                 "           `playlist_items`.`name`,"                       \
                 "           `playlists`.`id` AS `playlist_id`,"             \
                 "           `playlists`.`filename` AS `playlist_filename`," \
                 "           `playlists`.`name` AS `playlist_name`,"         \
                 "           `playlists`.`type` AS `playlist_type`,"         \
                 "           `playlists`.`favorite` AS `playlist_favorite`," \
                 "           `playlists`.`mtime` AS `playlist_mtime`,"       \
                 "           `tracks`.`id` AS `track_id`"                    \
                 "    FROM `playlist_items`"                                 \
                 "    LEFT JOIN `playlists` ON `playlist_items`.`playlist_id` = `playlists`.`id`" \
                 "    LEFT OUTER JOIN `tracks` ON `playlist_items`.`url` = `tracks`.`filename`;");

    // type playlist (0 = T_DATABASE, 1 = T_FILE, 2 = T_SMART)
    query.exec("CREATE TABLE `smart_playlists` ("                           \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"     \
                 "    `name` TEXT NOT NULL,"                                \
                 "    `icon` TEXT NOT NULL,"                                \
                 "    `rules` TEXT NOT NULL,"                               \
                 "    `type` INTEGER,"                                      \
                 "    `favorite` INTEGER);");

    // favorite stream
    query.exec("CREATE TABLE `favorite_stream` ("                           \
                 "    `id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"     \
                 "    `url` TEXT NOT NULL,"                                 \
                 "    `name` TEXT NOT NULL,"                                \
                 "    `genre` TEXT,"                                        \
                 "    `website` TEXT,"                                      \
                 "    `provider` TEXT);");

    
    //! Smart Playlist
    SmartPlaylist::createDatabase( );
}


/* ---------------------------------------------------------------------------*/
/* Database::close                                                            */
/*   -> close all opened connections                                          */
/* ---------------------------------------------------------------------------*/
void Database::close()
{
    // USE Database::close static fonction before switching collection database
    Debug::debug() << "[Database] closing";
    foreach(QString connection, m_sqldbs.keys())
    {
        //Debug::debug() << "- Database -> closing connection :" << connection;
        QSqlDatabase::database( connection ).close();
        delete m_sqldbs.take(connection);
        QSqlDatabase::removeDatabase(connection);
    }
    Debug::debug() << "[Database] closing OK";
}

/* ---------------------------------------------------------------------------*/
/* Database::open                                                             */
/* ---------------------------------------------------------------------------*/
bool Database::open(bool create/*=false*/)
{
    QString connection =    QString("%1").arg(
      reinterpret_cast<quint64>(QThread::currentThread())
    );
    
    Debug::debug() << "[Database] open : " << connection;

    if (QSqlDatabase::contains(connection))
        return true;

    /* check if dabatase file already exists */
    if( !Database::exist() && create == false ) {
      Debug::debug() << "[Database] database not already created !";      
      return false;
    }
    
    /* create Sql Database */
    Debug::debug() << "[Database] open -> new database connection";
    if( !QSqlDatabase::isDriverAvailable("QSQLITE") ) {
        Debug::error() << "[Database] Sqlite driver not available (check your qt install) !";
        StatusManager::instance()->startMessage( "[Database] Sqlite driver not available", STATUS::ERROR_CLOSE );

        return false;
    }

    QSqlDatabase* db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE",connection));
    m_sqldbs[connection] = db;        

    const QString db_path = QString(UTIL::CONFIGDIR + "/" + m_current_id + ".db");
    db->setDatabaseName( db_path );

    if (!db->open()) {
        Debug::warning() << "[Database] Failed to establish connection" << db->connectionName() << "Reason: " << db->lastError().text();
        return false;
    }

    QSqlQuery query(*db);
    query.exec("PRAGMA synchronous = OFF");
    query.exec("PRAGMA journal_mode = MEMORY");
    query.exec("PRAGMA auto_vacuum = FULL");

    return true;
}

/* ---------------------------------------------------------------------------*/
/* Database::db                                                               */
/* ---------------------------------------------------------------------------*/
QSqlDatabase* Database::db()
{
    //Debug::debug() << "[Database] accessing db thread" << QThread::currentThread()->objectName();
    const QString connection =    QString("%1").arg(
      reinterpret_cast<quint64>(QThread::currentThread())
    );
    
    if(m_sqldbs.contains(connection))
    {
      return m_sqldbs.value(connection);
    }
    else
    {
      Debug::error() << "[Database] accessing sql violation";
      return NULL;
    }
}

/* ---------------------------------------------------------------------------*/
/* Database::exist                                                            */
/* ---------------------------------------------------------------------------*/
bool Database::exist()
{
    Debug::debug() << "[Database] check if database file exists !";
  
    const QString db_path = QString(UTIL::CONFIGDIR + "/" + m_current_id + ".db");
      
    return QFile::exists(db_path);
}
    
/* ---------------------------------------------------------------------------*/
/* Database::versionOK                                                        */
/* ---------------------------------------------------------------------------*/    
bool Database::versionOK()
{
    bool versionOK;

    if(! this->open() )
      return false;

    QSqlQuery query("SELECT value FROM db_attribute WHERE name='version' LIMIT 1;", *this->db());
    query.next();

    if (query.isValid())
    {
        int currentRev =  query.value(0).toInt();
        Debug::debug() << "[Database] revision found : " << currentRev;
        if (currentRev != CST_DATABASE_REV)
          versionOK = false;
        else
          versionOK = true;
    }
    else 
    {
        Debug::debug() << "[Database] database reading revision failed ";
        versionOK = false;
    }

    return versionOK;
}

/* ---------------------------------------------------------------------------*/
/* Database::remove                                                           */
/* ---------------------------------------------------------------------------*/ 
void Database::remove()
{
    QString db_path = QString(UTIL::CONFIGDIR + "/" + m_current_id + ".db");
      
    QFile file(db_path);
    if(file.exists()) 
      file.remove();
}

/* ---------------------------------------------------------------------------*/
/* Database::idForName                                                        */
/* ---------------------------------------------------------------------------*/ 
QString Database::idForName(const QString& name)
{
    if(name.isEmpty())
      return QString();

    return QString(QCryptographicHash::hash(name.toUtf8().constData(), QCryptographicHash::Sha1).toHex());
}

/* ---------------------------------------------------------------------------*/
/* Database::settings_restore                                                 */
/* ---------------------------------------------------------------------------*/ 
void Database::settings_restore()
{
    Debug::debug() << "[Database] restore settings";
    QSettings s(UTIL::CONFIGFILE, QSettings::IniFormat);
  
    m_params.clear();

    s.beginGroup("Databases");

    QString db_name = s.value("dbCurrent").toString();

    int count = s.beginReadArray("dbEntry");
    
    Debug::debug() << "[Database] restore settings db name" << db_name;
    Debug::debug() << "[Database] restore settings db count " << count;

    for (int i=0 ; i<count ; ++i) {
      s.setArrayIndex(i);

      QString db_name =  s.value("name").toString();
      QString db_id   = idForName( db_name );

      Database::Param param;
      param._name                 = db_name;
      param._paths                << s.value("sourcepath").toStringList(); 
      param._option_auto_rebuild  = s.value("autorebuild", false).toBool();
      param._option_check_cover   = s.value("checkcover", true).toBool();
      param._option_group_albums  = s.value("groupAlbums", false).toBool();
      param._option_artist_image  = s.value("imageArtists", true).toBool();
      param._option_wr_rating_to_file = s.value("ratingToFile", false).toBool();
      
      m_params.insert(db_id, param);
      
      Debug::debug() << "[Database] _name "                   << m_params[db_id]._name;
      Debug::debug() << "[Database] _paths "                  << m_params[db_id]._paths;
      Debug::debug() << "[Database] _option_auto_rebuild "    << m_params[db_id]._option_auto_rebuild;
      Debug::debug() << "[Database] _option_check_cover "     << m_params[db_id]._option_check_cover;
      Debug::debug() << "[Database] _option_group_albums "    << m_params[db_id]._option_group_albums;
      Debug::debug() << "[Database] _option_artist_image "    << m_params[db_id]._option_artist_image;
      Debug::debug() << "[Database] _option_wr_rating_to_file "    << m_params[db_id]._option_wr_rating_to_file;
    }
    s.endArray();
    s.endGroup();

    /* check current database id */
    m_current_id    = idForName( db_name );
    
    if( db_name.isEmpty() || !m_params.keys().contains(m_current_id) ) 
    {
      /* take default database name & parameter */
      Debug::warning() << "[Database] force default database settings";
    
      Database::Param param;
      param._name = QString("collection");
      
      m_current_id = idForName( param._name );
      m_params.insert(m_current_id, param);
    }
}


/* ---------------------------------------------------------------------------*/
/* Database::settings_save                                                    */
/* ---------------------------------------------------------------------------*/ 
void Database::settings_save()
{
    Debug::debug() << "[Database] settings_save";
    QSettings s(UTIL::CONFIGFILE, QSettings::IniFormat);

    s.beginGroup("Databases");
    s.setValue("multiDb", isMultiDb());
    s.setValue("dbCurrent", m_params[m_current_id]._name);
    s.beginWriteArray("dbEntry", m_params.keys().count());
    int i=0;
    foreach (const QString& db_id, m_params.keys()) 
    {
      s.setArrayIndex(i++);
      s.setValue("name",        m_params[db_id]._name);
      s.setValue("sourcepath",  m_params[db_id]._paths);
      s.setValue("autorebuild", m_params[db_id]._option_auto_rebuild);
      s.setValue("checkcover",  m_params[db_id]._option_check_cover);
      s.setValue("groupAlbums", m_params[db_id]._option_group_albums);
      s.setValue("imageArtists", m_params[db_id]._option_artist_image);
      s.setValue("ratingToFile", m_params[db_id]._option_wr_rating_to_file);
    }
    s.endArray();
    s.endGroup();
    s.sync();
    
    emit settingsChanged();
}

/* ---------------------------------------------------------------------------*/
/* Database::param                                                            */
/* ---------------------------------------------------------------------------*/ 
const Database::Param& Database::param(const QString& name /*=QString() */)
{
    if(name.isEmpty())
    {
      return m_params[ m_current_id ];
    }
    else
    {
      return m_params[ idForName(name) ];
    }
}

/* ---------------------------------------------------------------------------*/
/* Database::change_database                                                  */
/* ---------------------------------------------------------------------------*/ 
void Database::change_database(const QString& db_name)
{
    Debug::debug() << "[Database] change database to " << idForName( db_name );
    m_current_id = idForName( db_name );
}

/* ---------------------------------------------------------------------------*/
/* Database::param_add                                                        */
/* ---------------------------------------------------------------------------*/ 
void Database::param_add(const Database::Param& param)
{
    m_params.insert( idForName(param._name), param );
}

/* ---------------------------------------------------------------------------*/
/* Database::param_names                                                      */
/* ---------------------------------------------------------------------------*/ 
QStringList Database::param_names()
{
    QStringList names;

    foreach(QString id, m_params.keys())
    {
      names << m_params.value(id)._name;
    }
    return names;
}

